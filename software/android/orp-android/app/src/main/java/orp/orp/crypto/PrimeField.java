/** @file PrimeField.java */
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
import java.nio.BufferUnderflowException;
import java.nio.ByteBuffer;

/** @brief A prime field (that is, Z/p where p is an odd prime). Only constructable for primes p where (p-1)/2 is odd. */
public class PrimeField {
    /** @brief Thrown when trying to construct a PrimeField over an invalid prime. */
    public static class InvalidPrime extends Exception {
        public final BigInteger p;

        protected InvalidPrime(BigInteger in_p) {
            p = in_p;
        }
    }

    protected final BigInteger N;       /* our prime */
    protected final int bytes_in_representation;    /* Number of bytes required to represent a number in this field */
    protected final BigInteger Np1o4;   /* (N+1)/4 */
    private BigInteger Beta;            /* Least number of the form 2^k which is > N */
    private BigInteger mu;              /* -1/N mod Beta */
    private BigInteger rsq;             /* montgomery conversion factor */

    public PrimeField(BigInteger in_N) throws InvalidPrime {
        N = in_N;
        if (!N.subtract(BigInteger.ONE).divide(BigInteger.valueOf(2)).testBit(0))
            throw new InvalidPrime(in_N);

        bytes_in_representation = (int)Math.ceil(N.bitLength() / 8.0);
        Np1o4 = N.add(BigInteger.ONE).divide(BigInteger.valueOf(4));
        Beta = BigInteger.ONE.shiftLeft(N.bitLength() + 1);
        mu = Beta.subtract(N.modInverse(Beta));

        rsq = mul(BigInteger.ONE, BigInteger.ONE);  /* (R/R) * (R/R) * (1/R) = 1/R */
        rsq = mul(rsq, BigInteger.ONE);             /* (1/R) * (R/R) * (1/R) = 1/R^2 */
        rsq = rsq.modInverse(N);                    /* = R^2 */
    }

    /** @brief Convert into Montgomery form. */
    public BigInteger toMontgomery(BigInteger a) {
        return mul(a, rsq);             /* a * R^2 * (1/R) = aR */
    }

    /** @brief Convert from Montgomery form. */
    public BigInteger fromMontgomery(BigInteger a) {
        return mul(a, BigInteger.ONE);  /* aR * (R/R) * (1/R) = a */
    }

    /** @brief Modular inverse. Assumes input is in Montgomery form; returns in Montgomery form. */
    public BigInteger inv(BigInteger a) {
        return toMontgomery(fromMontgomery(a).modInverse(N));
    }

    /** @brief Modular multiplication. Assumes inputs are in Montgomery form; returns in Montgomery form. */
    public BigInteger mul(BigInteger a, BigInteger b) {
        BigInteger C = a.multiply(b);
        BigInteger Q = mu.multiply(C).and(Beta.subtract(BigInteger.ONE));
        BigInteger R = (C.add(Q.multiply(N))).shiftRight(N.bitLength() + 1);
        BigInteger R2 = R.subtract(N);
        if (R.compareTo(N) >= 0)
            return R2;
        else
            return R;
    }

    /** @brief Modular negation. Assumes input is in Montgomery form; returns in Montgomery form. */
    public BigInteger neg(BigInteger a) {
        BigInteger diff = N.subtract(a);
        if (a.compareTo(BigInteger.ZERO) == 0)
            return a;
        return diff;
    }

    /** @brief Modular addition. Assumes input is in Montgomery form; returns in Montgomery form. */
    public BigInteger add(BigInteger a, BigInteger b) {
        BigInteger r1 = a.add(b);
        BigInteger r2 = r1.subtract(N);
        if (r1.compareTo(N) >= 0)
            return r2;
        else
            return r1;
    }

    /** @brief Modular subtraction. Assumes input is in Montgomery form; returns in Montgomery form. */
    public BigInteger sub(BigInteger a, BigInteger b) {
        return add(a, neg(b));
    }

    /** @brief Modular exponentiation. Assumes '0 < e' and assumes input is in Montgomery form; returns in Montgomery form. */
    public BigInteger exp(BigInteger a, BigInteger e) {
        int l = e.bitLength() - 1;
        BigInteger x = a;
        for (int i = l - 1; i >= 0; i--) {
            x = mul(x, x);
            if (e.testBit(i))
                x = mul(x, a);
        }
        return x;
    }

    /** @brief Given a square mod p, returns a square root. The other square root is the negative (mod p) of the given one. */
    public BigInteger sqrt(BigInteger a) {
        return exp(a, Np1o4);
    }

    /** @brief Serialize a number in this field, ignoring whether or not it is in Montgomery form. */
    public void Serialize(ByteBuffer out, BigInteger a) {
        for (int i = 0; i < bytes_in_representation; i++) {
            out.put(a.byteValue());
            a = a.shiftRight(8);
        }
    }

    /** @brief Deserialize a number in this field, ignoring whether or not it is in Montgomery form. */
    public BigInteger Deserialize(ByteBuffer in) {
        BigInteger x = BigInteger.ZERO;
        try {
            for (int i = 0; i < bytes_in_representation; i++) {
                int next_byte = 0xff & (int) in.get();
                x = x.or(BigInteger.valueOf(next_byte).shiftLeft(8 * i));
            }
        }
        catch (BufferUnderflowException e) {
            // do nothing, just return what we got
        }
        return x;
    }
}
