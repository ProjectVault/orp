/** @file ControlSession.java */
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

import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;

/** @brief The control session.
 *
 *
 * The control session (session ID 0) is used to start and stop other sessions. This object is a
 * driver for the control session; it is not to be used directly.
 *
 */
public class ControlSession extends PrimSession implements Runnable {
    private SessionManager mgr;
    private BlockingQueue<SessionManager.PreSession> session_requests;

    protected ControlSession(SessionManager in_mgr) {
        super();
        mgr = in_mgr;
        session_requests = new LinkedBlockingQueue<>();
    }

    /** @brief Request a new session with the given endpoint.
     *
     * @return A pre-session object.
     */
    protected SessionManager.PreSession requestSession(Endpoint ep) {
        SessionManager.PreSession preSession = new SessionManager.PreSession(ep);
        session_requests.add(preSession);
        return preSession;
    }

    /** @brief Write on the control channel.
     *
     * @return An outbound object.
     */
    private Outbound write(byte[] out) throws Outbound.ETooMuchData {
        Outbound outbound = new Outbound((short)0, out);
        mgr.queueOutbound(outbound);
        return outbound;
    }

    /** @brief Write on the control channel, blocking until done.
     *
     * @return The state of the write. Since the write is complete by the time this function returns, this value should be interpreted as an error/success indicator.
     */
    private Outbound.State writeBlocking(byte[] out) throws InterruptedException, Outbound.ETooMuchData {
        Outbound outbound = write(out);
        return outbound.blockUntil();
    }

    @Override
    public void run() {
        session_pump: while(true) {
            SessionManager.PreSession preSession = null;
            try {
                preSession = session_requests.take();
                preSession.state = SessionManager.PreSession.State.IN_NEGOTIATION;

                /* Send connection request */
                ByteArrayOutputStream packed_endpoint = new ByteArrayOutputStream();
                TIDL.uint8_serialize(packed_endpoint, (byte)0x01);
                Endpoint.serialize(packed_endpoint, preSession.endpoint);
                if (Outbound.State.SENT_FAIL_ERR == writeBlocking(packed_endpoint.toByteArray())) {
                    preSession.state = SessionManager.PreSession.State.FAIL;
                    preSession.setSession(null);
                    continue session_pump;
                }

                /* Read response */
                ByteBuffer response = ByteBuffer.wrap(read());
                short session_id = TIDL.uint16_deserialize(response);
                Session session = new Session(mgr, session_id);
                mgr.addSession(session_id, session);
                preSession.state = SessionManager.PreSession.State.OK;
                preSession.setSession(session);
            } catch (Exception e) {
                if (null != preSession) {
                    preSession.state = SessionManager.PreSession.State.LOCAL_FAIL;
                    preSession.setSession(null);
                }
            }
        }
    }
}
