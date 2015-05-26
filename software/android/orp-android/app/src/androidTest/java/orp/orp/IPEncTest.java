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

import java.io.IOException;
import java.util.Random;
import android.test.MoreAsserts;

import orp.demo.encryptor.Encryptor;
import orp.demo.encryptor.EncryptorAlgo;
import orp.demo.encryptor.EncryptorMode;

public class IPEncTest extends InstrumentationTestCase {
    public void testIPEnc() throws IOException, Encryptor.EncryptorException, Endpoint.InvalidHash, InterruptedException, TIDL.TIDLException, Outbound.ETooMuchData {
        IBus bus = new IPBus("10.0.2.2", 9998, 9999);
        SessionManager sessionManager = new SessionManager(bus);

        byte[] data = new byte[10000];
        new Random().nextBytes(data);

        Encryptor e = new Encryptor(sessionManager);
        e.init("Encryption key", EncryptorAlgo.EC_GCM_128, EncryptorMode.EC_ENCRYPT);
        byte[] output = e.go(data);
        e.stop();

        e.init("Encryption key", EncryptorAlgo.EC_GCM_128, EncryptorMode.EC_DECRYPT);
        byte[] original = e.go(output);
        e.stop();

        MoreAsserts.assertEquals(original, data);

        e.shutdown();
    }
}
