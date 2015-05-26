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

import java.security.PrivateKey;

public final class E521DHPrivateKey implements PrivateKey {
    private final byte[] privkey;

    protected E521DHPrivateKey(byte in_idx) {
        privkey = new byte[1];
        privkey[0] = in_idx;
    }

    @Override
    public String getAlgorithm() {
        return "E521DH";
    }

    @Override
    public String getFormat() {
        return "E521DH";
    }

    @Override
    public byte[] getEncoded() {
        return privkey;
    }

    protected byte getPrivkey() {
        return privkey[0];
    }
}
