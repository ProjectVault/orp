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

import java.math.BigInteger;
import java.util.Random;

import orp.orp.crypto.PrimeField;

public class PrimeFieldTest extends InstrumentationTestCase {
    public static final BigInteger p = BigInteger.ONE.shiftLeft(521).subtract(BigInteger.ONE);
    public static long TestTimeMillis = 60000; /* one minute */

    public BigInteger randomBigInteger(Random rnd, BigInteger upperBound) {
        BigInteger x = null;
        do {
            x = new BigInteger(upperBound.bitLength(), rnd);
        } while (x.compareTo(upperBound) >= 0);
        return x;
    }

    public void testMontgomery() {
        Random rnd = new Random();
        try {
            PrimeField Fp = new PrimeField(p);
            long test_count = 0;
            long start = System.currentTimeMillis();
            long current = 0;
            do {
                test_count++;
                BigInteger x = randomBigInteger(rnd, p);
                assertEquals(x, Fp.fromMontgomery(Fp.toMontgomery(x)));
                current = System.currentTimeMillis();
            } while (current - start <= TestTimeMillis);
            System.out.print("testMontgomery: "); System.out.print(test_count); System.out.println(" tests");
        }
        catch (Exception e) {
            fail("Exception");
        }
    }

    public void testMul() {
        Random rnd = new Random();
        try {
            PrimeField Fp = new PrimeField(p);
            long test_count = 0;
            long start = System.currentTimeMillis();
            long current = 0;
            do {
                test_count++;
                BigInteger x = randomBigInteger(rnd, p);
                BigInteger y = randomBigInteger(rnd, p);
                BigInteger z1 = x.multiply(y).mod(p);
                BigInteger z2 = Fp.fromMontgomery(Fp.mul(Fp.toMontgomery(x), Fp.toMontgomery(y)));
                assertEquals(z1, z2);
                current = System.currentTimeMillis();
            } while (current - start <= TestTimeMillis);
            System.out.print("testMul: "); System.out.print(test_count); System.out.println(" tests");
        }
        catch (Exception e) {
            fail("Exception");
        }
    }

    public void testAdd() {
        Random rnd = new Random();
        try {
            PrimeField Fp = new PrimeField(p);
            long test_count = 0;
            long start = System.currentTimeMillis();
            long current = 0;
            do {
                test_count++;
                BigInteger x = randomBigInteger(rnd, p);
                BigInteger y = randomBigInteger(rnd, p);
                BigInteger z1 = x.add(y).mod(p);
                BigInteger z2 = Fp.fromMontgomery(Fp.add(Fp.toMontgomery(x), Fp.toMontgomery(y)));
                assertEquals(z1, z2);
                current = System.currentTimeMillis();
            } while (current - start <= TestTimeMillis);
            System.out.print("testAdd: "); System.out.print(test_count); System.out.println(" tests");
        }
        catch (Exception e) {
            fail("Exception");
        }
    }

    public void testSub() {
        Random rnd = new Random();
        try {
            PrimeField Fp = new PrimeField(p);
            long test_count = 0;
            long start = System.currentTimeMillis();
            long current = 0;
            do {
                test_count++;
                BigInteger x = randomBigInteger(rnd, p);
                BigInteger y = randomBigInteger(rnd, p);
                BigInteger z1 = x.subtract(y).mod(p);
                BigInteger z2 = Fp.fromMontgomery(Fp.sub(Fp.toMontgomery(x), Fp.toMontgomery(y)));
                assertEquals(z1, z2);
                current = System.currentTimeMillis();
            } while (current - start <= TestTimeMillis);
            System.out.print("testSub: "); System.out.print(test_count); System.out.println(" tests");
        }
        catch (Exception e) {
            fail("Exception");
        }
    }

    public void testExp() {
        Random rnd = new Random();
        try {
            PrimeField Fp = new PrimeField(p);
            long test_count = 0;
            long start = System.currentTimeMillis();
            long current = 0;
            do {
                BigInteger x = randomBigInteger(rnd, p);
                BigInteger e = null;
                do {
                    e = randomBigInteger(rnd, p);
                } while (e.compareTo(BigInteger.ONE) <= 0);
                BigInteger z1 = x.modPow(e, p);
                BigInteger z2 = Fp.fromMontgomery(Fp.exp(Fp.toMontgomery(x), e));
                assertEquals(z1, z2);
                current = System.currentTimeMillis();
            } while (current - start <= TestTimeMillis);
            System.out.print("testExp: "); System.out.print(test_count); System.out.println(" tests");
        }
        catch (Exception e) {
            fail("Exception");
        }
    }

    public void testSqrt() {
        Random rnd = new Random();
        try {
            PrimeField Fp = new PrimeField(p);
            long test_count = 0;
            long start = System.currentTimeMillis();
            long current = 0;
            do {
                BigInteger x = Fp.toMontgomery(randomBigInteger(rnd, p));
                BigInteger x2 = Fp.mul(x, x);
                BigInteger y1 = Fp.sqrt(x2);
                BigInteger y2 = Fp.neg(y1);
                assertEquals(x2, Fp.mul(y1, y1));
                assertEquals(x2, Fp.mul(y2, y2));
                int cmp1 = x.compareTo(y1);
                int cmp2 = x.compareTo(y2);
                assertTrue(cmp1 == 0 || cmp2 == 0);
                current = System.currentTimeMillis();
            } while (current - start <= TestTimeMillis);
            System.out.print("testSqrt: "); System.out.print(test_count); System.out.println(" tests");
        }
        catch (Exception e) {
            fail("Exception");
        }
    }
}
