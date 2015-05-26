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

import java.security.NoSuchAlgorithmException;
import java.util.Random;

import orp.orp.crypto.EdwardsCurve;
import orp.orp.crypto.EdwardsPair;
import orp.orp.crypto.EdwardsPoint;
import orp.orp.crypto.PrimeField;

public class EdwardsPairTest extends InstrumentationTestCase {
    public static long TestTimeMillis = 60000; /* one minute */

    public void testDHHandshake() {
        EdwardsCurve curve = EdwardsCurve.curveE521();
        PrimeField Fp = curve.Fp;
        long start = System.currentTimeMillis();
        long current = 0;
        do {
            EdwardsPair alice = new EdwardsPair(curve);
            EdwardsPair bob = new EdwardsPair(curve);
            EdwardsPoint alice_shared = alice.DHhandshake(bob.pub);
            EdwardsPoint bob_shared = bob.DHhandshake(alice.pub);
            assertEquals(Fp.mul(alice_shared.x, Fp.inv(alice_shared.z)), Fp.mul(bob_shared.x, Fp.inv(bob_shared.z)));
            assertEquals(Fp.mul(alice_shared.y, Fp.inv(alice_shared.z)), Fp.mul(bob_shared.y, Fp.inv(bob_shared.z)));
            current = System.currentTimeMillis();
        } while (current - start <= TestTimeMillis);
    }

    public void testDSASign() {
        EdwardsCurve curve = EdwardsCurve.curveE521();
        Random rng = new Random();
        byte[] data = new byte[1024];

        long start = System.currentTimeMillis();
        long current = 0;
        do {
            rng.nextBytes(data);
            EdwardsPair alice = new EdwardsPair(curve);
            EdwardsPair.DSASignature sig = null;
            try {
                sig = alice.signDSA(data);
                assertTrue(sig.validate(data));
            } catch (NoSuchAlgorithmException e) {
                fail("Exception NoSuchAlgorithmException");
            }
            current = System.currentTimeMillis();
        } while (current - start <= TestTimeMillis);
    }
}
