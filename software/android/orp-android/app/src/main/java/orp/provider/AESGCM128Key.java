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

import javax.crypto.SecretKey;

public class AESGCM128Key implements SecretKey {
    private byte[] secret;

    public AESGCM128Key(byte in_secret) {
        secret = new byte[1];
        secret[0] = in_secret;
    }

    @Override
    public String getAlgorithm() {
        return "AESGCM128";
    }

    @Override
    public String getFormat() {
        return "AESGCM128";
    }

    @Override
    public byte[] getEncoded() {
        return secret;
    }

    protected byte getSecret() {
        return secret[0];
    }
}
