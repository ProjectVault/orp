/** @file Outbound.java */
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


import java.util.concurrent.CountDownLatch;

/** @brief Data in-queued for transmission in a session. */
public class Outbound {
    /** @brief The state of a queued transmission. */
    public static enum State {
        IN_QUEUE,           /**< In-queue, waiting to be sent */
        IN_DELIVERY,        /**< In-delivery, will send soon */
        SENT_WAITING,       /**< Delivered, waiting for response */
        SENT_ACK,           /**< Sent and acknowledged (success) */
        SENT_FAIL_ERR,      /**< Sent, error response */
        SENT_FAIL_RETRY,    /**< Sent, request to retry sending */
        LOCAL_FAIL          /**< Failure for some local reason */
    }

    /** @brief Thrown when one attempts to transmit a data blob which is too-large for a Faux Filesystem packet. */
    public static class ETooMuchData extends Exception {
        public final byte[] data;

        public ETooMuchData(byte[] in_data) {
            data = in_data;
        }
    }

    protected byte[] data;
    protected short session_id;
    private CountDownLatch latch;
    protected State state;

    protected Outbound(short in_session_id, byte[] in_data) throws ETooMuchData {
        if (in_data.length > SessionManager.PACKET_LENGTH - SessionManager.PACKET_HEADER_LENGTH)
            throw new ETooMuchData(in_data);
        data = in_data;
        session_id = in_session_id;
        latch = new CountDownLatch(1);
        state = State.IN_QUEUE;
    }

    /** @brief Fetch the current state of the queued transmission. */
    public State getState() { return state; }

    /** @brief Notify that this queued transmission has been processed, ending in the given state. */
    protected void notifyResult(State new_state) {
        state = new_state;
        latch.countDown();
    }

    /** @brief Block until the queued transmission has been processed, returning the final state. */
    public State blockUntil() throws InterruptedException {
        latch.await();
        return state;
    }
}
