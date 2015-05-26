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
import java.security.InvalidKeyException;
import java.security.Key;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.AlgorithmParameterSpec;

import javax.crypto.KeyAgreementSpi;
import javax.crypto.SecretKey;
import javax.crypto.ShortBufferException;

import orp.demo.bulkencryption.BulkEncryption;
import orp.demo.bulkencryption.DHExchange_Cmd;
import orp.demo.bulkencryption.DHExchange_Res;

public class E521DHKeyAgreementSpi extends KeyAgreementSpi {
    private byte privkey_idx = -1;
    boolean isInitialized = false;

    @Override
    protected Key engineDoPhase(Key key, boolean b) throws InvalidKeyException, IllegalStateException {
        if (!isInitialized)
            throw new IllegalStateException();
        if (!(key instanceof E521DHPublicKey))
            throw new InvalidKeyException();
        if (!b) /* we only allow one DoPhase, thus b must be true */
            throw new IllegalStateException();
        E521DHPublicKey pub = (E521DHPublicKey)key;

        BulkEncryption bulkEncryption = ORPProvider.bulkEncryption;
        if (null == bulkEncryption) return null;

        try {
            DHExchange_Cmd cmd = new DHExchange_Cmd();
            cmd.local_privkey_idx = privkey_idx;
            cmd.remote_pubkey = pub.getPubkey();
            DHExchange_Res res = bulkEncryption.DoDHExchange(cmd);
            if (null == res) return null;
            return new AESGCM128Key(res.shared_idx);
        }
        catch (InterruptedException e) {
            e.printStackTrace();
            return null;
        }
    }

    @Override
    protected byte[] engineGenerateSecret() throws IllegalStateException {
        throw new IllegalStateException();
    }

    @Override
    protected int engineGenerateSecret(byte[] bytes, int i) throws IllegalStateException, ShortBufferException {
        throw new IllegalStateException();
    }

    @Override
    protected SecretKey engineGenerateSecret(String s) throws IllegalStateException, NoSuchAlgorithmException, InvalidKeyException {
        throw new IllegalStateException();
    }

    @Override
    protected void engineInit(Key key, SecureRandom secureRandom) throws InvalidKeyException {
        if (!(key instanceof E521DHPrivateKey))
            throw new InvalidKeyException();
        E521DHPrivateKey priv = (E521DHPrivateKey)key;
        privkey_idx = priv.getPrivkey();
        isInitialized = true;
    }

    @Override
    protected void engineInit(Key key, AlgorithmParameterSpec algorithmParameterSpec, SecureRandom secureRandom) throws InvalidKeyException, InvalidAlgorithmParameterException {
        if (!(key instanceof E521DHPrivateKey))
            throw new InvalidKeyException();
        if (null != algorithmParameterSpec)
            throw new InvalidAlgorithmParameterException();
        E521DHPrivateKey priv = (E521DHPrivateKey)key;
        privkey_idx = priv.getPrivkey();
        isInitialized = true;
    }
}
