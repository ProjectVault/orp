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

import java.security.InvalidAlgorithmParameterException;
import java.security.KeyPair;
import java.security.KeyPairGeneratorSpi;
import java.security.SecureRandom;
import java.security.spec.AlgorithmParameterSpec;

import orp.demo.bulkencryption.BulkEncryption;
import orp.demo.bulkencryption.GenDHPair_Cmd;
import orp.demo.bulkencryption.GenDHPair_Res;

public class E521DHKeyPairGeneratorSpi extends KeyPairGeneratorSpi {
    private byte[] seed = null;

    @Override
    public KeyPair generateKeyPair() {
        BulkEncryption bulkEncryption = ORPProvider.bulkEncryption;
        if (null == bulkEncryption) return null;

        try {
            GenDHPair_Cmd cmd = new GenDHPair_Cmd();
            cmd.seed = seed;
            GenDHPair_Res res = bulkEncryption.DoGenDHPair(cmd);
            if (null == res) return null;
            return new KeyPair(new E521DHPublicKey(res.pubkey), new E521DHPrivateKey(res.privkey_idx));
        }
        catch (InterruptedException e) {
            e.printStackTrace();
            return null;
        }
    }

    @Override
    public void initialize (AlgorithmParameterSpec params, SecureRandom random) throws InvalidAlgorithmParameterException {
        if (!(params instanceof E521AlgorithmParameterSpec))
            throw new InvalidAlgorithmParameterException();

        E521AlgorithmParameterSpec e521params = (E521AlgorithmParameterSpec)params;
        seed = e521params.getSeed();
    }

    @Override
    public void initialize(int i, SecureRandom secureRandom) {
        E521AlgorithmParameterSpec e521params = new E521AlgorithmParameterSpec();
        seed = e521params.getSeed();
    }
}
