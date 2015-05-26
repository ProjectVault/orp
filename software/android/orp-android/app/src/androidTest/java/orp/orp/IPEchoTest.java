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

import android.test.InstrumentationTestCase;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

public class IPEchoTest extends InstrumentationTestCase {
    public void testIPEcho() throws IOException, Endpoint.InvalidHash, InterruptedException, Outbound.ETooMuchData, TIDL.StringCodeException, TIDL.StringLenException {
        IBus bus = new IPBus("10.0.2.2", 9998, 9999);
        SessionManager sessionManager = new SessionManager(bus);
        byte[] echo_hash = {0x61, 0x62, 0x63, 0x64, 0xa, 0x0, 0x0, 0x0,
                            0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,
                            0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,
                            0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0};
        Endpoint echo_ep = new Endpoint(echo_hash, (short)0x1337);
        Thread.sleep(1000);
        SessionManager.PreSession preSession = sessionManager.newSession(echo_ep);
        Session session = preSession.getSession();
        assertEquals(SessionManager.PreSession.State.OK, preSession.state);
        String howdy_str = "howdy";
        {
            ByteArrayOutputStream out = new ByteArrayOutputStream();
            TIDL.string_serialize(out, howdy_str);
            session.writeBlocking(out.toByteArray());
        }
        String response_str = null;
        {
            ByteBuffer response = ByteBuffer.wrap(session.read());
            response_str = TIDL.string_deserialize(response);
        }
        assertEquals(howdy_str, response_str);
    }
}
