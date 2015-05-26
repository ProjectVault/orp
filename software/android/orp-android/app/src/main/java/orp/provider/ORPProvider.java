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

import java.security.Provider;

import orp.demo.bulkencryption.BulkEncryption;

public final class ORPProvider extends Provider {
    protected static BulkEncryption bulkEncryption = null;
    protected static ORPProvider provider = null;

    protected ORPProvider(BulkEncryption in_bulkEncryption) {
        super("ORP Bulk Encryption Provider", 0.1, "Performing cryptographic operations on ORP");
        assert(null == bulkEncryption);
        bulkEncryption = in_bulkEncryption;
        assert(null != bulkEncryption);

        put("KeyPairGenerator.E521DH", "orp.provider.E521DHKeyPairGeneratorSpi");
        put("KeyAgreement.E521DH", "orp.provider.E521DHKeyAgreementSpi");
        put("Cipher.AESGCM128", "orp.provider.AESGCM128CipherSpi");

        put("KeyFactory.E521DH", "orp.provider.E521DHKeyFactorySpi");
        put("SecretKeyFactory.AESGCM128", "orp.provider.AESGCM128SecretKeyFactorySpi");
    }

    public static ORPProvider getORPProvider(BulkEncryption in_bulkEncryption) {
        if (null == provider) {
            provider = new ORPProvider(in_bulkEncryption);
            if (null == provider)
                bulkEncryption = null;
        }
        return provider;
    }
}
