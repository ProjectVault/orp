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
package orp.provider;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.AlgorithmParameterSpec;

import orp.demo.bulkencryption.GenDHPair_Cmd;

public final class E521AlgorithmParameterSpec implements AlgorithmParameterSpec {
    private final byte[] seed = new byte[GenDHPair_Cmd.SEED_LENGTH];

    public E521AlgorithmParameterSpec() {
        SecureRandom sr = new SecureRandom();
        sr.nextBytes(seed);
    }

    public E521AlgorithmParameterSpec(String in_seed) throws NoSuchAlgorithmException {
        MessageDigest digest = MessageDigest.getInstance("SHA-256");
        byte[] rough = digest.digest(in_seed.getBytes());
        int len = seed.length;
        if (len > rough.length) len = rough.length;
        for (int i = 0; i < len; i++)
            seed[i] = rough[i];
    }

    public byte[] getSeed() {
        return seed;
    }
}
