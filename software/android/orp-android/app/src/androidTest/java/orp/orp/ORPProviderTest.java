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
import android.util.Log;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.Key;
import java.security.KeyFactory;
import java.security.KeyPair;
import java.security.KeyPairGenerator;
import java.security.NoSuchAlgorithmException;
import java.security.PublicKey;
import java.security.Security;
import java.security.spec.InvalidKeySpecException;

import javax.crypto.Cipher;
import javax.crypto.CipherInputStream;
import javax.crypto.CipherOutputStream;
import javax.crypto.KeyAgreement;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKeyFactory;

import orp.demo.bulkencryption.BulkEncryption;
import orp.provider.E521AlgorithmParameterSpec;
import orp.provider.E521DHKeySpec;
import orp.provider.ORPProvider;

public class ORPProviderTest extends InstrumentationTestCase {
    public static String fromBytes(byte[] in) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < in.length; i++)
            sb.append(String.format("%02x", in[i]));
        return sb.toString();
    }

    public void testORPProvider() throws NoSuchAlgorithmException, InvalidAlgorithmParameterException, InvalidKeyException, InvalidKeySpecException, NoSuchPaddingException, IOException, TIDL.StringCodeException, TIDL.StringLenException {
        IBus bus = new IPBus("10.0.2.2", 9998, 9999);
        SessionManager sessionManager = new SessionManager(bus);

        BulkEncryption bulkEncryption = BulkEncryption.getBulkEncryption(sessionManager);
        assertNotNull(bulkEncryption);

        Security.addProvider(ORPProvider.getORPProvider(bulkEncryption));

        // Alice gets a key generator
        KeyPairGenerator aliceKpairGen = KeyPairGenerator.getInstance("E521DH");
        assertNotNull(aliceKpairGen);

        // Alice initializes her key generator
        E521AlgorithmParameterSpec aliceKeySpec = new E521AlgorithmParameterSpec();
        aliceKpairGen.initialize(aliceKeySpec);

        // Alice generates a key
        KeyPair aliceKpair = aliceKpairGen.generateKeyPair();
        byte[] alicePubKeyEnc = aliceKpair.getPublic().getEncoded();


        // Bob gets a key generator
        KeyPairGenerator bobKpairGen = KeyPairGenerator.getInstance("E521DH");
        assertNotNull(bobKpairGen);

        // Bob initializes his key generator
        E521AlgorithmParameterSpec bobKeySpec = new E521AlgorithmParameterSpec();
        bobKpairGen.initialize(bobKeySpec);

        // Bob generates a key
        KeyPair bobKpair = bobKpairGen.generateKeyPair();
        byte[] bobPubKeyEnc = bobKpair.getPublic().getEncoded();


        // Alice fetches Bob's key and performs the exchange
        KeyAgreement aliceKeyAgree = KeyAgreement.getInstance("E521DH");
        aliceKeyAgree.init(aliceKpair.getPrivate());
        KeyFactory aliceKeyFac = KeyFactory.getInstance("E521DH");
        PublicKey bobPubKey = aliceKeyFac.generatePublic(new E521DHKeySpec(bobPubKeyEnc));
        Key aliceAesKey = aliceKeyAgree.doPhase(bobPubKey, true);

        // Bob fetches Alice's key and performs the exchange
        KeyAgreement bobKeyAgree = KeyAgreement.getInstance("E521DH");
        bobKeyAgree.init(bobKpair.getPrivate());
        KeyFactory bobKeyFac = KeyFactory.getInstance("E521DH");
        PublicKey alicePubKey = aliceKeyFac.generatePublic(new E521DHKeySpec(alicePubKeyEnc));
        Key bobAesKey = bobKeyAgree.doPhase(alicePubKey, true);

        // Alice prepares for AES-GCM encrypt
        SecretKeyFactory aliceAesKeyFac = SecretKeyFactory.getInstance("AESGCM128");
        Cipher aliceAes = Cipher.getInstance("AESGCM128");
        aliceAes.init(Cipher.ENCRYPT_MODE, aliceAesKey);

        // Bob prepares for AES-GCM decrypt
        SecretKeyFactory bobAesKeyFac = SecretKeyFactory.getInstance("AESGCM128");
        Cipher bobAes = Cipher.getInstance("AESGCM128");
        bobAes.init(Cipher.DECRYPT_MODE, bobAesKey);


        // Alice encrypts a message
        String alice_says_hi = "Hello Bob, this is Alice.";
        byte[] alice_says_hi_enc = null;
        {
            ByteArrayOutputStream encoding = new ByteArrayOutputStream();
            TIDL.string_serialize(encoding, alice_says_hi);
            byte[] encoded = encoding.toByteArray();

            ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
            CipherOutputStream cipherOutputStream = new CipherOutputStream(outputStream, aliceAes);
            cipherOutputStream.write(encoded);
            Log.d("AESGCM", String.format("Encoded: %s", fromBytes(encoded)));
            cipherOutputStream.flush();
            cipherOutputStream.close();
            alice_says_hi_enc = outputStream.toByteArray();
            Log.d("AESGCM", String.format("Encrypted: %s", fromBytes(alice_says_hi_enc)));
        }

        // Bob decrypts the message
        String bob_recvs_alice_greeting = null;
        {
            ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
            ByteArrayInputStream inStream = new ByteArrayInputStream(alice_says_hi_enc);
            CipherInputStream cipherInputStream = new CipherInputStream(inStream, bobAes);
            byte[] buf = new byte[1024];
            int bytesRead;
            while ((bytesRead = cipherInputStream.read(buf)) >= 0) {
                outputStream.write(buf, 0, bytesRead);
            }
            byte[] encoded = outputStream.toByteArray();
            Log.d("AESGCM", String.format("Encoded: %s", fromBytes(encoded)));
            bob_recvs_alice_greeting = TIDL.string_deserialize(ByteBuffer.wrap(encoded));
        }

        assertEquals(alice_says_hi, bob_recvs_alice_greeting);
    }
}
