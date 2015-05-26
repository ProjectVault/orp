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
package orp.demo.encryptor;

import java.nio.ByteBuffer;
import java.io.ByteArrayOutputStream;
import android.util.Log;

import orp.orp.Endpoint;
import orp.orp.Outbound;
import orp.orp.Session;
import orp.orp.SessionManager;
import orp.orp.TIDL;

/**
 * Created by drmorr on 3/25/15.
 */

/** @brief Wrapper class for the bulk encryptor application */
public class Encryptor {

    private Session session;
    public static final int DATA_LEN = 2040;
    public enum EncError {SESSION_FAILED, INITIALIZATION_FAILED, UNSUPPORTED_ALGORITHM, DATA_FAILED, RESET_FAILED, SHUTDOWN_FAILED};

    public static class EncryptorException extends Exception {

        public final EncError e;
        private EncryptorException(EncError in_e) {
            e = in_e;
        }

        @Override
        public String getMessage() { return "Error with encryptor: " + e; }
    }

    // Initialize the session and try to start the application
    public Encryptor(SessionManager sm) throws EncryptorException, Endpoint.InvalidHash, InterruptedException {
        /* enc_hash = "enc\n" */
        byte[] enc_hash = {0x65, 0x6e, 0x63, 0xa, 0x0, 0x0, 0x0, 0x0,
                0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,
                0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,
                0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0};
        Endpoint enc_ep = new Endpoint(enc_hash, (short)0x1337);
        Thread.sleep(1000);

        SessionManager.PreSession preSession = sm.newSession(enc_ep);
        session = preSession.getSession();
        if (preSession.getState() != SessionManager.PreSession.State.OK)
            throw new EncryptorException(EncError.SESSION_FAILED);
    }

    // Set the algorithm, mode, and key
    public void init(String key, EncryptorAlgo algo, EncryptorMode mode) throws EncryptorException, TIDL.TIDLException, Outbound.ETooMuchData, InterruptedException {
        // Write the initialization packet
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        EncryptorAlgo.Serialize(out, algo);
        EncryptorMode.Serialize(out, mode);
        TIDL.string_serialize(out, key);
        session.writeBlocking(out.toByteArray());

        // Check to make sure initialization was OK
        ByteBuffer rbuf = ByteBuffer.wrap(session.read());
        EncryptorResponse response = EncryptorResponse.Deserialize(rbuf);
        if (response == EncryptorResponse.ER_ERROR)
            throw new EncryptorException(EncError.INITIALIZATION_FAILED);
        else if (response == EncryptorResponse.ER_UNSUPPORTED)
            throw new EncryptorException(EncError.UNSUPPORTED_ALGORITHM);
    }

    // Do the encryption or decryption and return the result
    public byte[] go(byte[] data) throws EncryptorException, TIDL.TIDLException, Outbound.ETooMuchData, InterruptedException {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        byte[] ret = new byte[data.length];

        // Loop through the input data -- can only transfer DATA_LEN bytes at a time
        for (int i = 0; i < data.length; i += DATA_LEN) {
            Log.d("Encryptor.go", "Starting chunk " + i);
            EncryptorCommand.Serialize(out, EncryptorCommand.EC_DATA);
            int len = i + DATA_LEN > data.length ? data.length - i: DATA_LEN;
            out.write(data, i, len);
            session.writeBlocking(out.toByteArray());
            Log.d("Encryptor.go", "Wrote " + len + "bytes of chunk " + i + ", beginning read");

            ByteBuffer response = ByteBuffer.wrap(session.read());
            Log.d("Encryptor.go", "Done with read of chunk " + i);
            if (EncryptorResponse.Deserialize(response) != EncryptorResponse.ER_OK)
                throw new EncryptorException(EncError.DATA_FAILED);
            response.get(ret, i, len);
            out.reset();
        }
        return ret;
    }

    // Stop encrypting; must call when you're done (not automatic because you could have multiple things
    // to encrypt or decrypt with the same key)
    public void stop() throws EncryptorException, TIDL.TIDLException, Outbound.ETooMuchData, InterruptedException {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        EncryptorCommand.Serialize(out, EncryptorCommand.EC_DONE);
        session.writeBlocking(out.toByteArray());

        ByteBuffer response = ByteBuffer.wrap(session.read());
        if (EncryptorResponse.Deserialize(response) != EncryptorResponse.ER_OK)
            throw new EncryptorException(EncError.RESET_FAILED);
    }

    // Shut down the application.  There's no way to restart it, so this object is dead at that point.
    public void shutdown() throws EncryptorException, TIDL.TIDLException, Outbound.ETooMuchData, InterruptedException {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        EncryptorAlgo.Serialize(out, EncryptorAlgo.EC_GCM_128);     // Shutdown is the second part of the packet, so just fill in a dummy algo
        EncryptorMode.Serialize(out, EncryptorMode.EC_SHUTDOWN);
        session.writeBlocking(out.toByteArray());

        ByteBuffer response = ByteBuffer.wrap(session.read());
        if (EncryptorResponse.Deserialize(response) != EncryptorResponse.ER_OK)
            throw new EncryptorException(EncError.SHUTDOWN_FAILED);
    }
}
