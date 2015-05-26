/** @file EdwardsPair.java */
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
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;

/** @brief Key pair over an Edwards curve. */
public class EdwardsPair {
    private static SecureRandom rng = null;
    public final EdwardsCurve curve;
    public final BigInteger priv;
    public final EdwardsPoint pub;

    protected static BigInteger RandomBigInteger(SecureRandom rng, PrimeField Fp, int depth) {
        if (0 >= depth) {
            byte[] Ra = new byte[64];
            rng.nextBytes(Ra);
            return Fp.toMontgomery((new BigInteger(Ra)).mod(Fp.N));
        }
        else {
            BigInteger a = RandomBigInteger(rng, Fp, depth - 1);
            BigInteger b = RandomBigInteger(rng, Fp, depth - 1);
            BigInteger c = RandomBigInteger(rng, Fp, depth - 1);
            return Fp.add(a, Fp.mul(b, c));
        }
    }

    public EdwardsPair(EdwardsCurve in_curve) {
        curve = in_curve;
        if (null == rng)
            rng = new SecureRandom();

        priv = RandomBigInteger(rng, curve.modOrder, 4);
        pub = curve.g.Montgomery(priv);
    }

    public EdwardsPoint DHhandshake(EdwardsPoint other) {
        return other.Montgomery(priv);
    }

    public static class DSASignature {
        public final EdwardsPoint pub;
        public final BigInteger r;
        public final BigInteger s;

        public DSASignature(EdwardsPoint in_pub, BigInteger in_r, BigInteger in_s) {
            pub = in_pub;
            r = in_r;
            s = in_s;
        }

        public boolean validate(byte[] in) throws NoSuchAlgorithmException {
            EdwardsCurve curve = pub.curve;
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            digest.reset();
            PrimeField modN = curve.modOrder;
            PrimeField Fp = curve.Fp;

            byte[] e = digest.digest(in);
            BigInteger z = modN.toMontgomery(modN.Deserialize(ByteBuffer.wrap(e)));

            BigInteger s = modN.toMontgomery(this.s);
            BigInteger r = modN.toMontgomery(this.r);

            BigInteger w = modN.inv(s);
            BigInteger u1 = modN.fromMontgomery(modN.mul(z, w));
            BigInteger u2 = modN.fromMontgomery(modN.mul(r, w));

            EdwardsPoint pt = curve.g.Montgomery(u1).Add(pub.Montgomery(u2));
            BigInteger x = Fp.fromMontgomery(Fp.mul(pt.x, Fp.inv(pt.z))).mod(curve.order);

            return 0 == this.r.compareTo(x);
        }
    }

    public DSASignature signDSA(byte[] in) throws NoSuchAlgorithmException {
        MessageDigest digest = MessageDigest.getInstance("SHA-256");
        digest.reset();
        PrimeField modN = curve.modOrder;
        PrimeField Fp = curve.Fp;

        BigInteger dA = modN.toMontgomery(priv);

        byte[] e = digest.digest(in);
        BigInteger z = modN.toMontgomery(modN.Deserialize(ByteBuffer.wrap(e)));

        BigInteger k = modN.mul(z, dA);
        EdwardsPoint kg = curve.g.Montgomery(modN.fromMontgomery(k));

        BigInteger x1 = Fp.fromMontgomery(Fp.mul(kg.x, Fp.inv(kg.z)));
        BigInteger r = modN.toMontgomery(x1.mod(curve.order));
        BigInteger s = modN.mul(modN.inv(k), modN.add(z, modN.mul(r, dA)));

        return new DSASignature(pub, modN.fromMontgomery(r), modN.fromMontgomery(s));
    }
}
