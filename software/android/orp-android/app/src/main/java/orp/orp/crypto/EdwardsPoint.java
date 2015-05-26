/** @file EdwardsPoint.java */
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

/** @brief A projective point on an Edwards curve, with coordinates in Montgomery form. */
public class EdwardsPoint {
    public final EdwardsCurve curve;
    public final BigInteger x;
    public final BigInteger y;
    public final BigInteger z;

    protected EdwardsPoint(EdwardsCurve in_curve, BigInteger in_x, BigInteger in_y, BigInteger in_z) {
        curve = in_curve;
        x = in_x;
        y = in_y;
        z = in_z;
    }

    public EdwardsPoint(EdwardsCompressedPoint p) {
        // TODO: confirm p is a point on the curve!
        curve = p.curve;
        PrimeField Fp = curve.Fp;

        x = Fp.toMontgomery(p.x);
        z = Fp.toMontgomery(BigInteger.ONE);
            /* now solve for y */
        BigInteger x2 = Fp.mul(x, x);
        BigInteger one = Fp.toMontgomery(BigInteger.ONE);
        BigInteger y2 = Fp.mul(Fp.sub(one, x2), Fp.inv(Fp.sub(one, Fp.mul(Fp.toMontgomery(curve.d), x2))));
        BigInteger y1 = Fp.sqrt(y2);
        BigInteger y1_regular = Fp.fromMontgomery(y1);
        if (y1_regular.testBit(0) != p.sgn_y)
            y = Fp.N.subtract(y1_regular);
        else
            y = y1;
    }

    /** @brief Returns the sum (in the curve group) of this point with the given point. */
    public EdwardsPoint Add(EdwardsPoint p2) {
        BigInteger D = null;
        if (curve.d.compareTo(BigInteger.ZERO) <= 0)
            D = curve.Fp.neg(curve.Fp.toMontgomery(curve.d.negate()));
        else
            D = curve.Fp.toMontgomery(curve.d);

            /* http://hyperelliptic.org/EFD/g1p/auto-edwards-projective.html#addition-add-2007-bl-2 */
        PrimeField Fp = curve.Fp;
        //R1 = X1
        BigInteger r1 = x;
        //R2 = Y1
        BigInteger r2 = y;
        //R3 = Z1
        BigInteger r3 = z;
        //R4 = X2
        BigInteger r4 = p2.x;
        //R5 = Y2
        BigInteger r5 = p2.y;
        //R6 = Z2
        BigInteger r6 = p2.z;
        //R3 = R3*R6
        r3 = Fp.mul(r3, r6);
        //R7 = R1+R2
        BigInteger r7 = Fp.add(r1, r2);
        //R8 = R4+R5
        BigInteger r8 = Fp.add(r4, r5);
        //R1 = R1*R4
        r1 = Fp.mul(r1, r4);
        //R2 = R2*R5
        r2 = Fp.mul(r2, r5);
        //R7 = R7*R8
        r7 = Fp.mul(r7, r8);
        //R7 = R7-R1
        r7 = Fp.sub(r7, r1);
        //R7 = R7-R2
        r7 = Fp.sub(r7, r2);
        //R7 = R7*R3
        r7 = Fp.mul(r7, r3);
        //R8 = R1*R2
        r8 = Fp.mul(r1, r2);
        //R8 = d*R8
        r8 = Fp.mul(D, r8);
        //R2 = R2-R1
        r2 = Fp.sub(r2, r1);
        //R2 = R2*R3
        r2 = Fp.mul(r2, r3);
        //R3 = R3^2
        r3 = Fp.mul(r3, r3);
        //R1 = R3-R8
        r1 = Fp.sub(r3, r8);
        //R3 = R3+R8
        r3 = Fp.add(r3, r8);
        //R2 = R2*R3
        r2 = Fp.mul(r2, r3);
        //R3 = R3*R1
        r3 = Fp.mul(r3, r1);
        //R1 = R1*R7
        r1 = Fp.mul(r1, r7);
        //R3 = c*R3
            /* c==1 for our Edwards curves */
        //X3 = R1
        //Y3 = R2
        //Z3 = R3
        return new EdwardsPoint(curve, r1, r2, r3);
    }

    /** @brief Returns the sum (in the curve group) of this point with itself. */
    public EdwardsPoint Double() {
        PrimeField Fp = curve.Fp;
            /* http://hyperelliptic.org/EFD/g1p/auto-edwards-projective.html#doubling-dbl-2007-bl-3 */
        // R1 = X1
        BigInteger r1 = x;
        // R2 = Y1
        BigInteger r2 = y;
        // R3 = Z1
        BigInteger r3 = z;
        // R3 = c*R3
            /* c==1 for our Edwards curves */
        // R4 = R1^2
        BigInteger r4 = Fp.mul(r1, r1);
        // R1 = R1+R2
        r1 = Fp.add(r1, r2);
        // R1 = R1^2
        r1 = Fp.mul(r1, r1);
        // R2 = R2^2
        r2 = Fp.mul(r2, r2);
        // R3 = R3^2
        r3 = Fp.mul(r3, r3);
        // R3 = 2*R3
        r3 = Fp.add(r3, r3);
        // R4 = R2+R4
        r4 = Fp.add(r2, r4);
        // R2 = 2*R2
        r2 = Fp.add(r2, r2);
        // R2 = R4-R2
        r2 = Fp.sub(r4, r2);
        // R1 = R1-R4
        r1 = Fp.sub(r1, r4);
        // R2 = R2*R4
        r2 = Fp.mul(r2, r4);
        // R3 = R4-R3
        r3 = Fp.sub(r4, r3);
        // R1 = R1*R3
        r1 = Fp.mul(r1, r3);
        // R3 = R3*R4
        r3 = Fp.mul(r3, r4);
        // R1 = c*R1
            /* c==1 in our Edwards curves */
        // R2 = c*R2
            /* c==1 in our Edwards curves */
        // X3 = R1
        // Y3 = R2
        // Z3 = R3
        return new EdwardsPoint(curve, r1, r2, r3);
    }

    /** @brief Returns the n-fold sum (in the curve group) of this point with itself, where n > 1. */
    public EdwardsPoint Montgomery(BigInteger n) {
        int l = n.bitLength();
        EdwardsPoint p1 = this;
        EdwardsPoint p2 = this.Double();

        int i = l - 2;
        while (i >= 0) {
            if (!n.testBit(i)) {
                p2 = p1.Add(p2);
                p1 = p1.Double();
            }
            else {
                p1 = p1.Add(p2);
                p2 = p2.Double();
            }
            i--;
        }

        return p1;
    }
}
