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

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.security.AlgorithmParameters;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.Key;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.AlgorithmParameterSpec;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.CipherSpi;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.ShortBufferException;

import orp.demo.bulkencryption.BulkEncryption;
import orp.demo.bulkencryption.GCM_Mode;
import orp.demo.bulkencryption.GcmDoBlock_Cmd;
import orp.demo.bulkencryption.GcmDoBlock_Res;
import orp.demo.bulkencryption.GcmInit_Cmd;
import orp.demo.bulkencryption.GcmInit_Res;

public class AESGCM128CipherSpi extends CipherSpi {
    public static final int BLOCK_LENGTH = GcmDoBlock_Cmd.BLOCK_LENGTH;

    private boolean isInitialized = false;
    private byte gcm_idx = -1;
    private ByteArrayOutputStream buffer = new ByteArrayOutputStream();

    @Override
    protected void engineSetMode(String s) throws NoSuchAlgorithmException {
        throw new NoSuchAlgorithmException();
    }

    @Override
    protected void engineSetPadding(String s) throws NoSuchPaddingException {
        throw new NoSuchPaddingException();
    }

    @Override
    protected int engineGetBlockSize() {
        return BLOCK_LENGTH;
    }

    @Override
    protected int engineGetOutputSize(int i) {
        int x = i + buffer.toByteArray().length;
        int re = x % BLOCK_LENGTH;
        if (0 < re)
            x += BLOCK_LENGTH - re;
        return x;
    }

    @Override
    protected byte[] engineGetIV() {
        return null;
    }

    @Override
    protected AlgorithmParameters engineGetParameters() {
        return null;
    }

    @Override
    protected void engineInit(int i, Key key, SecureRandom secureRandom) throws InvalidKeyException {
        if (!(key instanceof AESGCM128Key)) throw new InvalidKeyException();

        BulkEncryption bulkEncryption = ORPProvider.bulkEncryption;
        if (null == bulkEncryption) return;

        AESGCM128Key shared = (AESGCM128Key)key;

        try {
            GcmInit_Cmd cmd = new GcmInit_Cmd();

            switch (i) {
                case Cipher.DECRYPT_MODE:
                    cmd.mode = GCM_Mode.GCM128_DECRYPT;
                    break;
                case Cipher.ENCRYPT_MODE:
                    cmd.mode = GCM_Mode.GCM128_ENCRYPT;
                    break;
                default:
                    return;
            }
            cmd.shared_idx = shared.getSecret();
            GcmInit_Res res = bulkEncryption.DoGcmInit(cmd);
            if (null == res) return;
            isInitialized = true;
            gcm_idx = res.gcm_idx;
        }
        catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    @Override
    protected void engineInit(int i, Key key, AlgorithmParameterSpec algorithmParameterSpec, SecureRandom secureRandom) throws InvalidKeyException, InvalidAlgorithmParameterException {

    }

    @Override
    protected void engineInit(int i, Key key, AlgorithmParameters algorithmParameters, SecureRandom secureRandom) throws InvalidKeyException, InvalidAlgorithmParameterException {

    }

    @Override
    protected byte[] engineUpdate(byte[] bytes, int i, int i2) {
        if (!isInitialized) return null;
        BulkEncryption bulkEncryption = ORPProvider.bulkEncryption;
        if (null == bulkEncryption) return null;

        try {
            if (null != bytes)
                buffer.write(bytes, i, i2);

            byte[] consuming = buffer.toByteArray();
            ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
            int x = 0;
            while (x + BLOCK_LENGTH < consuming.length) {
                GcmDoBlock_Cmd cmd = new GcmDoBlock_Cmd();
                cmd.gcm_idx = gcm_idx;
                cmd.block = new byte[GcmDoBlock_Cmd.BLOCK_LENGTH];
                for (int pos = 0; pos < BLOCK_LENGTH; pos++)
                    cmd.block[pos] = consuming[x + pos];
                GcmDoBlock_Res res = bulkEncryption.DoGcmBlock(cmd);
                if (null == res) return null;
                outputStream.write(res.block);

                x += BLOCK_LENGTH;
            }

            buffer = new ByteArrayOutputStream();
            buffer.write(consuming, x, consuming.length - x);

            byte[] ret = outputStream.toByteArray();
            return ret;
        } catch (InterruptedException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return null;
    }

    @Override
    protected int engineUpdate(byte[] bytes, int i, int i2, byte[] bytes2, int i3) throws ShortBufferException {
        byte[] out = engineUpdate(bytes, i, i2);
        if (i3 + out.length > bytes2.length)
            throw new ShortBufferException();
        for (int z = 0; z < out.length; z++)
            bytes2[i3 + z] = out[z];
        return out.length;
    }

    @Override
    protected byte[] engineDoFinal(byte[] bytes, int i, int i2) throws IllegalBlockSizeException, BadPaddingException {
        BulkEncryption bulkEncryption = ORPProvider.bulkEncryption;
        if (null == bulkEncryption) return null;

        ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
        byte[] pre_process = engineUpdate(bytes, i, i2);
        if (null == pre_process)
            return null;

        try {
            outputStream.write(pre_process);
            byte[] consuming = buffer.toByteArray();

            for (int x = 0; x < consuming.length; x += BLOCK_LENGTH) {
                GcmDoBlock_Cmd cmd = new GcmDoBlock_Cmd();
                cmd.gcm_idx = gcm_idx;
                cmd.block = new byte[GcmDoBlock_Cmd.BLOCK_LENGTH];
                int writeLen = Math.min(BLOCK_LENGTH, consuming.length - x);
                for (int pos = 0; pos < writeLen; pos++)
                    cmd.block[pos] = consuming[x + pos];
                GcmDoBlock_Res res = bulkEncryption.DoGcmBlock(cmd);
                if (null == res) return null;
                outputStream.write(res.block);
            }

            buffer = new ByteArrayOutputStream();
            byte[] ret = outputStream.toByteArray();
            return ret;
        }
        catch (Exception e) {
            e.printStackTrace();
            return null;
        }
    }

    @Override
    protected int engineDoFinal(byte[] bytes, int i, int i2, byte[] bytes2, int i3) throws ShortBufferException, IllegalBlockSizeException, BadPaddingException {
        byte[] out = engineDoFinal(bytes, i, i2);
        if (i3 + out.length > bytes2.length)
            throw new ShortBufferException();
        for (int z = 0; z < out.length; z++)
            bytes2[i3 + z] = out[z];
        return out.length;
    }
}
