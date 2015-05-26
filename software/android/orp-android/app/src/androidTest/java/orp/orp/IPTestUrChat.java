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

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.Random;

import orp.demo.urchat.UrChat;
import orp.orp.IPBus;
import orp.orp.SessionManager;

/**
 */
public class IPTestUrChat extends InstrumentationTestCase {
    private UrChat alice;
    private UrChat bob;

    private byte[] makePass(String userName, String userPass) throws UnsupportedEncodingException, NoSuchAlgorithmException {
        MessageDigest digest = MessageDigest.getInstance("SHA-256");
        digest.reset();
        return digest.digest(String.format("%s:%s", userName, userPass).getBytes("UTF-8"));
    }

    private byte[] makeSeek(String seekphrase) throws UnsupportedEncodingException, NoSuchAlgorithmException {
        MessageDigest digest = MessageDigest.getInstance("SHA-256");
        digest.reset();
        return digest.digest(seekphrase.getBytes("UTF-8"));
    }

    final protected static char[] hexArray = "0123456789abcdef".toCharArray();
    public static String bytesToHex(byte[] bytes) {
        char[] hexChars = new char[bytes.length * 2];
        for ( int j = 0; j < bytes.length; j++ ) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }
        return new String(hexChars);
    }

    public void testChat() throws IOException, NoSuchAlgorithmException, InterruptedException {
        alice = UrChat.getUrChat(new SessionManager(new IPBus("10.0.2.2", 9998, 9999)));
        bob = UrChat.getUrChat(new SessionManager(new IPBus("10.0.2.2", 9988, 9989)));

        assertNotNull(alice);
        assertNotNull(bob);

        Log.d("IPTestUrChat", "alice and bob both have orp-urchat sessions");

        byte[] alice_old_pubkey = null;
        byte[] bob_old_pubkey = null;

        for (int i = 0; i < 2; i++){
            Log.d("IPTestUrChat", String.format("Running test number %d", i));
            String alice_passphrase = String.format("%s-%d", "password123", (new Random()).nextInt());
            String bob_passphrase = String.format("%s-%d", "p4s5w0rd", (new Random()).nextInt());
            byte[] alice_makepass = makePass("alice", alice_passphrase);
            byte[] bob_makepass = makePass("bob", bob_passphrase);
            assertFalse(Arrays.equals(alice_makepass, bob_makepass));
            assertTrue(alice.login(alice_makepass));
            assertTrue(bob.login(bob_makepass));

            Log.d("IPTestUrChat", "alice and bob both have logged-in");

            assertFalse(Arrays.equals(alice.getPubkey(), bob.getPubkey()));
            if (null != alice_old_pubkey)
                assertFalse(Arrays.equals(alice_old_pubkey, alice.getPubkey()));
            if (null != bob_old_pubkey)
                assertFalse(Arrays.equals(bob_old_pubkey, bob.getPubkey()));
            alice_old_pubkey = alice.getPubkey();
            bob_old_pubkey = bob.getPubkey();
            Log.d("IPTestUrChat", String.format("alice pub = %s", bytesToHex(alice.getPubkey())));
            Log.d("IPTestUrChat", String.format("bob pub   = %s", bytesToHex(bob.getPubkey())));

            /* Alice and Bob seek for one another */
            {
                byte[][] seekset = new byte[1][];
                String phrase = String.format("%s-%d", "our-m33tup-phrase-12!2$@%", (new Random()).nextInt());
                seekset[0] = makeSeek(phrase);
                assertTrue(alice.setSeek(seekset));
                assertTrue(bob.setSeek(seekset));
                byte[] alice_seeking_bob = alice.getSeek(0);
                byte[] bob_seeking_alice = bob.getSeek(0);
                assertNotNull(alice_seeking_bob);
                assertNotNull(bob_seeking_alice);
                UrChat.Decrypted alice_nevermind = alice.incoming(alice_seeking_bob);
                UrChat.Decrypted alice_found = alice.incoming(bob_seeking_alice);
                UrChat.Decrypted bob_found = bob.incoming(alice_seeking_bob);
                UrChat.Decrypted bob_nevermind = bob.incoming(bob_seeking_alice);
                assertEquals(UrChat.Decrypted.Kind.NEVERMIND, alice_nevermind.kind);
                assertEquals(UrChat.Decrypted.Kind.SEEK_FOUND, alice_found.kind);
                assertEquals(UrChat.Decrypted.Kind.NEVERMIND, bob_nevermind.kind);
                assertEquals(UrChat.Decrypted.Kind.SEEK_FOUND, bob_found.kind);

                assertTrue(Arrays.equals(alice_found.pubkey, bob.getPubkey()));
                assertTrue(Arrays.equals(bob_found.pubkey, alice.getPubkey()));

                seekset = new byte[0][];
                assertTrue(alice.setSeek(seekset));
                assertTrue(bob.setSeek(seekset));
            }

            /* Alice and Bob open channels to one another */
            int alice_bob_channel;
            int bob_alice_channel;
            {
                alice_bob_channel = alice.getChannel(bob.getPubkey());
                bob_alice_channel = bob.getChannel(alice.getPubkey());
                assertFalse(UrChat.INVALID_CHANNEL == alice_bob_channel);
                assertFalse(UrChat.INVALID_CHANNEL == bob_alice_channel);

                byte[] alice_ephemeral = alice.getExchange(alice_bob_channel);
                byte[] bob_ephemeral = bob.getExchange(bob_alice_channel);
                assertNotNull(alice_ephemeral);
                assertNotNull(bob_ephemeral);
                assertFalse(Arrays.equals(alice_ephemeral, bob_ephemeral));
                UrChat.Decrypted alice_nevermind = alice.incoming(alice_ephemeral);
                UrChat.Decrypted alice_exchanged = alice.incoming(bob_ephemeral);
                UrChat.Decrypted bob_exchanged = bob.incoming(alice_ephemeral);
                UrChat.Decrypted bob_nevermind = bob.incoming(bob_ephemeral);

                assertEquals(UrChat.Decrypted.Kind.NEVERMIND, alice_nevermind.kind);
                assertEquals(UrChat.Decrypted.Kind.EXCHANGE_OCCURRED, alice_exchanged.kind);
                assertEquals(UrChat.Decrypted.Kind.EXCHANGE_OCCURRED, bob_exchanged.kind);
                assertEquals(UrChat.Decrypted.Kind.NEVERMIND, bob_nevermind.kind);

                assertEquals(alice_bob_channel, alice_exchanged.channel_id);
                assertEquals(bob_alice_channel, bob_exchanged.channel_id);
            }

            /* Alice sends a message to Bob over the channel */
            {
                String hi = "Hello there";
                String bye = "Goodbye";
                byte[] alice_hi = alice.encrypt(alice_bob_channel, hi);
                {
                    {
                        UrChat.Decrypted d = bob.incoming(alice_hi);
                        assertEquals(UrChat.Decrypted.Kind.PLAINTEXT_GOT, d.kind);
                        assertEquals(hi, d.message);
                    }
                    {
                        UrChat.Decrypted d = bob.incoming(alice_hi);
                        assertEquals(UrChat.Decrypted.Kind.NEVERMIND, d.kind);
                    }
                    {
                        UrChat.Decrypted d = alice.incoming(alice_hi);
                        assertEquals(UrChat.Decrypted.Kind.NEVERMIND, d.kind);
                    }
                }
                byte[] bob_hi = bob.encrypt(bob_alice_channel, hi);
                {
                    {
                        UrChat.Decrypted d = alice.incoming(bob_hi);
                        assertEquals(UrChat.Decrypted.Kind.PLAINTEXT_GOT, d.kind);
                        assertEquals(hi, d.message);
                    }
                    {
                        UrChat.Decrypted d = bob.incoming(bob_hi);
                        assertEquals(UrChat.Decrypted.Kind.NEVERMIND, d.kind);
                    }
                }
                byte[] alice_hi2 = alice.encrypt(alice_bob_channel, hi);
                assertFalse(Arrays.equals(alice_hi, alice_hi2));
                {
                    {
                        UrChat.Decrypted d = bob.incoming(alice_hi2);
                        assertEquals(UrChat.Decrypted.Kind.PLAINTEXT_GOT, d.kind);
                        assertEquals(hi, d.message);
                    }
                    {
                        UrChat.Decrypted d = bob.incoming(alice_hi2);
                        assertEquals(UrChat.Decrypted.Kind.NEVERMIND, d.kind);
                    }
                }
                byte[] alice_bye = alice.encrypt(alice_bob_channel, bye);
                {
                    {
                        UrChat.Decrypted d = bob.incoming(alice_bye);
                        assertEquals(UrChat.Decrypted.Kind.PLAINTEXT_GOT, d.kind);
                        assertEquals(bye, d.message);
                    }
                    {
                        UrChat.Decrypted d = bob.incoming(alice_bye);
                        assertEquals(UrChat.Decrypted.Kind.NEVERMIND, d.kind);
                    }
                    {
                        UrChat.Decrypted d = alice.incoming(alice_bye);
                        assertEquals(UrChat.Decrypted.Kind.NEVERMIND, d.kind);
                    }
                }
            }

            /* Alice and Bob close their channels */
            assertTrue(alice.closeChannel(alice_bob_channel));
            assertTrue(bob.closeChannel(bob_alice_channel));

            /* Alice and Bob logout */
            assertTrue(alice.logout());
            assertTrue(bob.logout());

            Log.d("IPTestUrChat", "alice and bob have both logged out");
        }
    }
}
