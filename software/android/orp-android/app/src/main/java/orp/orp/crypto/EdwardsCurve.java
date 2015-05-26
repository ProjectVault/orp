/** @file EdwardsCurve.java */
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

package orp.orp.crypto;

import java.math.BigInteger;

/** @brief An Edwards curve, defined by the equation 'x^2 + y^2 = 1 + dx^2y^2' over a prime field where 'd(1-d)' is nonzero. */
public class EdwardsCurve {
    /** @brief Thrown when one tries to create an Edwards curve where 'd(d-1)' is zero in the base field. */
    public static class InvalidParam extends Exception {
        public final BigInteger d;
        public InvalidParam(BigInteger in_d) {
            d = in_d;
        }
    }

    private static EdwardsCurve singleton_curveE521 = null;
    public final PrimeField Fp;
    public final BigInteger d;       /* invariant: d-1 is nonzero in Fp */
    public final EdwardsPoint g;     /* generator */
    public final BigInteger order;   /* group order */
    public final PrimeField modOrder;   /* field Z/order */
    public final EdwardsPoint id;    /* identity point */

    protected EdwardsCurve(PrimeField in_Fp, BigInteger in_d, BigInteger in_g_x, BigInteger in_g_y, BigInteger in_g_z, BigInteger in_order) throws InvalidParam, PrimeField.InvalidPrime {
        // Check that in_d(in_d - 1) is nonzero mod in_p
        if (0 == in_d.multiply(in_d.subtract(BigInteger.ONE)).mod(in_Fp.N).compareTo(BigInteger.ZERO))
            throw new InvalidParam(in_d);
        Fp = in_Fp;
        d = in_d;
        g = new EdwardsPoint(this, in_g_x, in_g_y, in_g_z);
        order = in_order;
        modOrder = new PrimeField(order);
        id = new EdwardsPoint(this, Fp.toMontgomery(BigInteger.ZERO), Fp.toMontgomery(BigInteger.ONE), Fp.toMontgomery(BigInteger.ONE));
    }

    /** @brief Returns curve E-521. */
    public static EdwardsCurve curveE521() {
        if (null == singleton_curveE521) {
            BigInteger e521_p = BigInteger.ONE.shiftLeft(521).subtract(BigInteger.valueOf(1));
            BigInteger e521_d = BigInteger.valueOf(-376014);
            BigInteger e521_order = new BigInteger("7ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd15b6c64746fc85f736b8af5e7ec53f04fbd8c4569a8f1f4540ea2435f5180d6b", 16);
            try {
                PrimeField e521_Fp = new PrimeField(e521_p);
                singleton_curveE521 = new EdwardsCurve(
                        e521_Fp,
                        e521_d,
                        e521_Fp.toMontgomery(new BigInteger("752cb45c48648b189df90cb2296b2878a3bfd9f42fc6c818ec8bf3c9c0c6203913f6ecc5ccc72434b1ae949d568fc99c6059d0fb13364838aa302a940a2f19ba6c", 16)),
                        e521_Fp.toMontgomery(new BigInteger("c", 16)),
                        e521_Fp.toMontgomery(BigInteger.ONE),
                        e521_order);
            }
            catch (InvalidParam ipE) {
                /* can't happen */
            } catch (PrimeField.InvalidPrime invalidPrime) {
                /* can't happen */
            }
        }

        return singleton_curveE521;
    }
}
