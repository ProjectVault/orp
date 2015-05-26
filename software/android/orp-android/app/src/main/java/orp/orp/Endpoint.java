/** @file Endpoint.java */
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

import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;
import java.util.Arrays;

/** @brief A Faux Filesystem endpoint. */
public class Endpoint {
    public static final int HashLength = 32;

    protected byte[] hash;
    protected short num;

    /** @brief Thrown when trying to construct an endpoint using a hash of the wrong length. */
    public static class InvalidHash extends Exception {
        public final byte[] invalid_hash;

        public InvalidHash(byte[] in_invalid_hash) {
            invalid_hash = in_invalid_hash;
        }

        @Override
        public String getMessage() {
            return "Supplied hash has invalid length";
        }
    }

    public Endpoint(byte[] in_hash, short in_num) throws InvalidHash {
        if (HashLength != in_hash.length)
            throw new InvalidHash(in_hash);
        hash = in_hash;
        num = in_num;
    }

    @Override
    public boolean equals(Object in_o) {
        Endpoint o = (Endpoint)in_o;

        return (num == o.num) && Arrays.equals(hash, o.hash);
    }

    /** @brief Serialize the given endpoint. */
    public static void serialize(ByteArrayOutputStream out, Endpoint ep) {
        for (int i = 0; i < ep.hash.length; i++)
            TIDL.uint8_serialize(out, ep.hash[i]);
        TIDL.uint16_serialize(out, ep.num);
    }

    /** @brief Deserialize an endpoint. */
    public static Endpoint deserialize(ByteBuffer in) {
        byte[] in_hash = new byte[HashLength];
        short in_num = 0;

        for (int i = 0; i < HashLength; i++)
            in_hash[i] = TIDL.uint8_deserialize(in);
        in_num = TIDL.uint16_deserialize(in);

        Endpoint ret = null;
        try {
            ret = new Endpoint(in_hash, in_num);
        } catch (InvalidHash invalidHash) {
            invalidHash.printStackTrace();
        }

        return ret;
    }
}
