/** @file SessionManager.java */
/*
   Copyright 2015, Google Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

package orp.orp;


import android.util.Log;
import android.util.SparseArray;

import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.LinkedBlockingQueue;

/** @brief Top-level user class for the Faux Filesystem. */
public class SessionManager {
    public static final int PACKET_HEADER_LENGTH = 2 + 2; /**< Length of Faux Filesystem packet header in bytes (2 for session ID, 2 for packet length). */
    public static final int PACKET_LENGTH = 2048;         /**< Length of Faux Filesystem packet in bytes (including header). */
    public static final int STATUS_LENGTH = 16;           /**< Length of Faux Filesystem status block */

    public static String bufToString(byte[] in) {
        StringBuilder sb = new StringBuilder(in.length * 2);
        for (int i = 0; i < in.length; i++)
            sb.append(String.format("%02x", in[i]));
        return sb.toString();
    }

    /** @brief The writer-pump. */
    protected static class Writer implements Runnable {
        /** @brief Status of the write-channel. */
        private IWriteChannel channel;
        private BlockingQueue<Outbound> outbounds;

        protected Writer(IWriteChannel in_channel, BlockingQueue<Outbound> in_outbounds) {
            channel = in_channel;
            outbounds = in_outbounds;
        }

        @Override
        public void run() {
            byte nonce = (byte)0xc1;
            Outbound outbound = null;
            outbound_pump: while (true) {
                try {
                    nonce++;
                    if (0x00 == nonce)
                        nonce = 0x01;
                    if ((null == outbound) || !(Outbound.State.SENT_FAIL_RETRY == outbound.state))
                        outbound = outbounds.take();
                    if (Outbound.State.SENT_FAIL_RETRY == outbound.state)
                        Log.d("Session Manager write", "resending");
                    outbound.state = Outbound.State.IN_DELIVERY;
                    /* write data */
                    {
                        ByteArrayOutputStream formatted = new ByteArrayOutputStream();
                        TIDL.uint16_serialize(formatted, outbound.session_id);            /* session id */
                        TIDL.uint8_serialize(formatted, nonce);                           /* nonce */
                        TIDL.uint8_serialize(formatted, (byte) 0);                        /* reserved */
                        for (int i = 0; i < outbound.data.length; i++)
                            TIDL.uint8_serialize(formatted, outbound.data[i]);            /* pkt data */
                        Log.d("SessionManager write", bufToString(formatted.toByteArray()));
                        channel.write(formatted.toByteArray());
                    }
                    outbound.state = Outbound.State.SENT_WAITING;
                    /* wait for confirmation, notify when it arrives */
                    IWriteChannel.ChannelStatus status;
                    int waitTimes = 0;
                    wait_for_confirm:
                    while (true) {
                        status = channel.status(nonce);
                        switch (status) {
                            case CHANNEL_OK:
                                outbound.notifyResult(Outbound.State.SENT_ACK);
                                break wait_for_confirm;
                            case CHANNEL_ERROR:
                                outbound.notifyResult(Outbound.State.SENT_FAIL_ERR);
                                break wait_for_confirm;
                            case CHANNEL_RETRY:
                                outbound.state = Outbound.State.SENT_FAIL_RETRY;
                                Thread.sleep(125, 0);
                                continue outbound_pump;
                            case CHANNEL_WAIT:
                                Thread.sleep(25, 0);
                                waitTimes++;
                                if (waitTimes >= 8) {
                                    outbound.state = Outbound.State.SENT_FAIL_RETRY;
                                    continue outbound_pump;
                                }
                                continue wait_for_confirm;
                        }
                    }
                    assert(null != status);
                    Log.d("SessionManager write", "acknowledge status: " + status.toString());
                    /* done. */
                } catch (InterruptedException e) {
                    if (null != outbound) {
                        if (Outbound.State.IN_DELIVERY == outbound.state)
                            outbound.notifyResult(Outbound.State.LOCAL_FAIL);
                    }
                }
            }
        }
    }

    /** @brief The reader-pump. */
    protected static class Reader implements Runnable {
        private IReadChannel channel;
        private SparseArray<PrimSession> sessions;
        private ControlSession controlSession;

        protected Reader(IReadChannel in_channel, SparseArray<PrimSession> in_sessions, ControlSession in_controlSession) {
            channel = in_channel;
            sessions = in_sessions;
            controlSession = in_controlSession;
        }

        /** @brief Returns true if the packet header is 0 (i.e., there is no data or the data has not finished writing). */
        private static boolean isVoidPacket(byte[] inbound) {
            for (int i = 0; i < PACKET_HEADER_LENGTH; i++)
                if (0x00 != inbound[i])
                    return false;
            return true;
        }

        @Override
        public void run() {
            byte prev_nonce = 0x00;
            inbound_pump: while (true) {
                try {
                    IReadChannel.Status status = IReadChannel.Status.ERROR;
                    /* read packet */
                    byte[] inbound = channel.read(PACKET_LENGTH);
                    while (isVoidPacket(inbound)) {
                        Thread.sleep(25, 0);
                        inbound = channel.read(PACKET_LENGTH);
                    }
                    /* parse */
                    ByteBuffer buf = ByteBuffer.wrap(inbound);
                    short session_id = TIDL.uint16_deserialize(buf);
                    byte nonce = TIDL.uint8_deserialize(buf);
                    if (prev_nonce == nonce) {
                        status = IReadChannel.Status.ACKNOWLEDGE;
                        channel.acknowledge(status);
                        Thread.sleep(25);
                        continue inbound_pump;
                    }
                    Log.d("SessionManager read", bufToString(inbound));
                    prev_nonce = nonce;
                    TIDL.uint8_deserialize(buf);   /* skip unused portion */
                    byte[] data = new byte[inbound.length - PACKET_HEADER_LENGTH];
                    for (int i = 0; i < inbound.length - PACKET_HEADER_LENGTH; i++)
                        data[i] = TIDL.uint8_deserialize(buf);
                    /* route */
                    PrimSession destination = sessions.get(session_id);
                    if (null != destination) {
                        status = IReadChannel.Status.ACKNOWLEDGE;
                        destination.incomingData(data);
                    }
                    /* acknowledge */
                    channel.acknowledge(status);
                    Log.d("SessionManager read", "acknowledge status: " + status.toString());
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    /** @brief A pre-session object. Created when a request for a new session is made. */
    public static class PreSession {
        /** @brief State of the request-for-new-session. */
        public static enum State {
            IN_QUEUE,
            IN_NEGOTIATION,
            OK,
            FAIL,
            LOCAL_FAIL
        }

        protected final Endpoint endpoint;
        protected State state;
        private CountDownLatch latch;
        private Session session;

        protected PreSession(Endpoint in_endpoint) {
            endpoint = in_endpoint;
            state = State.IN_QUEUE;
            latch = new CountDownLatch(1);
            session = null;
        }

        public State getState() {
            return state;
        }

        /** @brief Indicate that the session request has finished, resulting in either a session object (on success) or null (on failure). */
        protected void setSession(Session in_session) {
            session = in_session;
            latch.countDown();
        }

        /** @brief Fetch the session, blocking until the request has been serviced. */
        public Session getSession() throws InterruptedException {
            latch.await();
            return session;
        }
    }

    private BlockingQueue<Outbound> outbounds;
    private SparseArray<PrimSession> sessions;
    private ControlSession controlSession;
    private Thread reader;
    private Thread writer;
    private Thread control;

    public SessionManager(IBus bus) {
        outbounds = new LinkedBlockingQueue();
        sessions = new SparseArray<>();
        controlSession = new ControlSession(this);
        sessions.put(0, controlSession);

        writer = new Thread(new Writer(bus.writeChannel(), outbounds));
        reader = new Thread(new Reader(bus.readChannel(), sessions, controlSession));
        control = new Thread(controlSession);

        writer.start();
        reader.start();
        control.start();
    }

    /** @brief Request a new session with the given endpoint. */
    public PreSession newSession(Endpoint ep) {
        return controlSession.requestSession(ep);
    }

    /** @brief Queue an outbound transmission. */
    protected void queueOutbound(Outbound outbound) {
        outbounds.add(outbound);
    }

    protected void addSession(short session_id, Session session) {
        sessions.put(session_id, session);
    }
}
