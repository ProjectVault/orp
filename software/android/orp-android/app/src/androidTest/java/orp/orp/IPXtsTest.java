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
import android.test.MoreAsserts;

import java.io.IOException;
import java.util.Random;
import java.nio.ByteBuffer;

import orp.demo.encryptorxts.XtsAlgo;
import orp.demo.encryptorxts.XtsEncryptor;

public class IPXtsTest extends InstrumentationTestCase {

    public static final int NUM_BLOCKS = 100;

    public void testIPXts() throws IOException, XtsEncryptor.XtsEncryptorException, Endpoint.InvalidHash, InterruptedException, TIDL.TIDLException, Outbound.ETooMuchData {
        IBus bus = new IPBus("10.0.2.2", 9998, 9999);
        SessionManager sessionManager = new SessionManager(bus);

        byte[] data = new byte[XtsEncryptor.DATA_BLOCK_SIZE * NUM_BLOCKS];
        byte[] output = new byte[XtsEncryptor.DATA_BLOCK_SIZE * NUM_BLOCKS];
        byte[] original = new byte[XtsEncryptor.DATA_BLOCK_SIZE * NUM_BLOCKS];

        new Random().nextBytes(data);

        XtsEncryptor xts = new XtsEncryptor(sessionManager);

        xts.init("/path/to/file", XtsAlgo.XTS_128);

        ByteBuffer outbuf = ByteBuffer.wrap(output);
        byte[] block = new byte[XtsEncryptor.DATA_BLOCK_SIZE];
        for (int i = 0; i < NUM_BLOCKS; i++) {
            System.arraycopy(data, i * XtsEncryptor.DATA_BLOCK_SIZE, block, 0, XtsEncryptor.DATA_BLOCK_SIZE);
            outbuf.put(xts.go(i, block, true));
        }

        ByteBuffer origbuf = ByteBuffer.wrap(original);
        for (int i = 0; i < NUM_BLOCKS; i++) {
            System.arraycopy(output, i * XtsEncryptor.DATA_BLOCK_SIZE, block, 0, XtsEncryptor.DATA_BLOCK_SIZE);
            origbuf.put(xts.go(i, block, false));
        }
        xts.stop();

        MoreAsserts.assertEquals(original, data);
    }
}
