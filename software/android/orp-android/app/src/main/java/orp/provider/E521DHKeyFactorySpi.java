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

import java.security.InvalidKeyException;
import java.security.Key;
import java.security.KeyFactorySpi;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;

public class E521DHKeyFactorySpi extends KeyFactorySpi {
    @Override
    protected PublicKey engineGeneratePublic(KeySpec keySpec) throws InvalidKeySpecException {
        if (!(keySpec instanceof E521DHKeySpec))
            throw new InvalidKeySpecException();
        E521DHKeySpec spec = (E521DHKeySpec)keySpec;
        return new E521DHPublicKey(spec.getKey());
    }

    @Override
    protected PrivateKey engineGeneratePrivate(KeySpec keySpec) throws InvalidKeySpecException {
        if (!(keySpec instanceof E521DHKeySpec)) throw new InvalidKeySpecException();
        E521DHKeySpec spec = (E521DHKeySpec)keySpec;
        return new E521DHPrivateKey(spec.getKey()[0]);
    }

    @Override
    protected <T extends KeySpec> T engineGetKeySpec(Key key, Class<T> tClass) throws InvalidKeySpecException {
        throw new InvalidKeySpecException();
    }

    @Override
    protected Key engineTranslateKey(Key key) throws InvalidKeyException {
        throw new InvalidKeyException();
    }
}
