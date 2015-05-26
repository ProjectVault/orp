/** @file EdwardsCompressedPoint.java */
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
import java.nio.ByteBuffer;

/** @brief The compressed representation of a point on an Edwards curve. */
public class EdwardsCompressedPoint {
    /** @brief Thrown when one encounters an invalid value for 'sign y' when trying to deserialize a compressed point. */
    public static class EInvalidSgnY extends Exception {
        public final byte sgn_y;

        public EInvalidSgnY(byte in_sgn_y) {
            sgn_y = in_sgn_y;
        }
    }

    public final EdwardsCurve curve;
    public final BigInteger x;
    public final boolean sgn_y;

    protected EdwardsCompressedPoint(EdwardsCurve in_curve, BigInteger in_x, boolean in_sgn_y) {
        curve = in_curve;
        x = in_x;
        sgn_y = in_sgn_y;
    }

    public EdwardsCompressedPoint(EdwardsPoint p) {
        curve = p.curve;
        PrimeField Fp = curve.Fp;
        BigInteger zinv = Fp.inv(p.z);
        x = Fp.fromMontgomery(Fp.mul(p.x, zinv));
        BigInteger y = Fp.fromMontgomery(Fp.mul(p.y, zinv));
        sgn_y = y.testBit(0);
    }

    /** @brief Serialize the given compressed point. */
    public static void Serialize(ByteBuffer out, EdwardsCompressedPoint p) {
        p.curve.Fp.Serialize(out, p.x);
        out.put((byte)(p.sgn_y ? 1 : 0));
    }

    /** @brief Deserialize a compressed point on the given curve. */
    public static EdwardsCompressedPoint Deserialize(ByteBuffer in, EdwardsCurve curve) throws EInvalidSgnY {
        BigInteger x = curve.Fp.Deserialize(in);
        boolean sgn_y = false;
        byte y = in.get();
        switch (y) {
            case 0:
                sgn_y = false;
                break;
            case 1:
                sgn_y = true;
                break;
            default:
                throw new EInvalidSgnY(y);
        }
        return new EdwardsCompressedPoint(curve, x, sgn_y);
    }
}
