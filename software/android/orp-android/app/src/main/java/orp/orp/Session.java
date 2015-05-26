/** @file Session.java */
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

/** @brief A session. */
public class Session extends PrimSession {
    private SessionManager mgr;
    private final short id;

    protected Session(SessionManager in_mgr, short in_id) {
        super();
        mgr = in_mgr;
        id = in_id;
    }

    /** @brief Write to this session. */
    public Outbound write(byte[] out) throws Outbound.ETooMuchData {
        Outbound outbound = new Outbound(id, out);
        mgr.queueOutbound(outbound);
        return outbound;
    }

    /** @brief Write to this session, blocking until completion. */
    public Outbound.State writeBlocking(byte[] out) throws InterruptedException, Outbound.ETooMuchData {
        Outbound outbound = write(out);
        return outbound.blockUntil();
    }
}
