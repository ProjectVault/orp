package orp.demo.encryptorxts;

import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;

import orp.orp.Endpoint;
import orp.orp.Outbound;
import orp.orp.Session;
import orp.orp.SessionManager;
import orp.orp.TIDL;
import orp.demo.encryptor.EncryptorResponse;

/**
 * Created by drmorr on 3/25/15.
 */

/** @brief Wrapper class for the bulk encryptor application */
public class XtsEncryptor {

    private Session session;
    public static final int DATA_BLOCK_SIZE = 512;
    public enum XtsError {SESSION_FAILED, INITIALIZATION_FAILED, UNSUPPORTED_ALGORITHM, DATA_FAILED, RESET_FAILED, SHUTDOWN_FAILED};

    public static class XtsEncryptorException extends Exception {

        public final XtsError e;
        private XtsEncryptorException(XtsError in_e) {
            e = in_e;
        }

        @Override
        public String getMessage() { return "Error with XTS encryptor: " + e; }
    }

    // Initialize the session and try to start the application
    public XtsEncryptor(SessionManager sm) throws XtsEncryptorException, Endpoint.InvalidHash, InterruptedException {
        /* enc_hash = "xts\n" */
        byte[] enc_hash = {0x78, 0x74, 0x73, 0xa, 0x0, 0x0, 0x0, 0x0,
                0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,
                0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,
                0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0};
        Endpoint enc_ep = new Endpoint(enc_hash, (short)0x1337);
        Thread.sleep(1000);

        SessionManager.PreSession preSession = sm.newSession(enc_ep);
        session = preSession.getSession();
        if (preSession.getState() != SessionManager.PreSession.State.OK)
            throw new XtsEncryptorException(XtsError.SESSION_FAILED);
    }

    // Set the algorithm, mode, and key
    public void init(String key, XtsAlgo algo) throws XtsEncryptorException, TIDL.TIDLException, Outbound.ETooMuchData, InterruptedException {
        // Write the initialization packet
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        XtsAlgo.Serialize(out, algo);
        TIDL.uint64_serialize(out, DATA_BLOCK_SIZE);
        TIDL.string_serialize(out, key);
        session.writeBlocking(out.toByteArray());

        // Check to make sure initialization was OK
        ByteBuffer rbuf = ByteBuffer.wrap(session.read());
        XtsResponse response = XtsResponse.Deserialize(rbuf);
        if (response == XtsResponse.XTS_ERROR)
            throw new XtsEncryptorException(XtsError.INITIALIZATION_FAILED);
        else if (response == XtsResponse.XTS_UNSUPPORTED)
            throw new XtsEncryptorException(XtsError.UNSUPPORTED_ALGORITHM);
    }

    // Do the encryption or decryption and return the result
    public byte[] go(long sequence, byte[] data, Boolean encrypt) throws XtsEncryptorException, TIDL.TIDLException, Outbound.ETooMuchData, InterruptedException {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        byte[] ret = new byte[data.length];

        Log.d("XtsEncryptor.go", "Starting sector " + sequence);
        if (encrypt) XtsCommand.Serialize(out, XtsCommand.XTS_ENCRYPT);
        else XtsCommand.Serialize(out, XtsCommand.XTS_DECRYPT);
        TIDL.uint64_serialize(out, sequence);

        out.write(data, 0, DATA_BLOCK_SIZE);
        session.writeBlocking(out.toByteArray());
        Log.d("XtsEncryptor.go", "Wrote " + DATA_BLOCK_SIZE + "bytes of sector " + sequence + ", beginning read");

        ByteBuffer response = ByteBuffer.wrap(session.read());
        Log.d("XtsEncryptor.go", "Done with read of encrypted sector " + sequence);
        if (XtsResponse.Deserialize(response) != XtsResponse.XTS_OK)
            throw new XtsEncryptorException(XtsError.DATA_FAILED);
        response.get(ret, 0, DATA_BLOCK_SIZE);
        out.reset();

        return ret;
    }

    // Stop encrypting; must call when you're done (not automatic because you could have multiple things
    // to encrypt or decrypt with the same key)
    public void stop() throws XtsEncryptorException, TIDL.TIDLException, Outbound.ETooMuchData, InterruptedException {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        XtsCommand.Serialize(out, XtsCommand.XTS_SHUTDOWN);
        session.writeBlocking(out.toByteArray());

        ByteBuffer response = ByteBuffer.wrap(session.read());
        if (EncryptorResponse.Deserialize(response) != EncryptorResponse.ER_OK)
            throw new XtsEncryptorException(XtsError.RESET_FAILED);
    }
}
