/** @file TIDL.java */
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
import java.io.UnsupportedEncodingException;
import java.nio.ByteBuffer;

/** @brief Static methods class for TIDL primitive serializers and deserializers.
 *
 */
public class TIDL {
    /** @brief Parent class of all TIDL exceptions. */
    public static class TIDLException extends Exception {
        protected TIDLException() { }
    }

    /** @brief Thrown when an array of improper length is encountered. */
    public static class ArrayLenException extends TIDLException {
        public final int length;

        public ArrayLenException(int in_length) {
            length = in_length;
        }
    }

    /** @brief Thrown when the recursion-depth limit has been reached. */
    public static class RecursionException extends TIDLException {
        public RecursionException() { }
    }

    /** @brief Thrown when an attempt to deserialize an enumeration fails due to an unrecognized value. */
    public static class UnrecognizedEnumException extends TIDLException {
        public final int value;

        public UnrecognizedEnumException(int in_value) {
            value = in_value;
        }
    }

    /** @brief Thrown when one tries to serialize a string whose length cannot be represented as a 'short'. */
    public static class StringLenException extends TIDLException {
        public final String str;

        public StringLenException(String in_str) {
            str = in_str;
        }
    }

    /** @brief Thrown when working with a string and it turns out the platform lacks UTF-8 encoding. */
    public static class StringCodeException extends TIDLException {
        public StringCodeException() { }
    }

    private TIDL() { }

    /** @brief String serializer. */
    public static void string_serialize(ByteArrayOutputStream out, String in) throws StringLenException, StringCodeException {
        byte[] encoded = null;
        try {
            encoded = in.getBytes("UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw new StringCodeException();
        }

        short length = (short)encoded.length;
        if ((int)length != encoded.length)
            throw new StringLenException(in);

        uint16_serialize(out, length);
        for (int i = 0; i < length; i++)
            uint8_serialize(out, encoded[i]);
    }

    /** @brief String deserializer. */
    public static String string_deserialize(ByteBuffer in) throws StringCodeException {
        short length = uint16_deserialize(in);
        byte[] encoded = new byte[length];
        for (int i = 0; i < length; i++)
            encoded[i] = uint8_deserialize(in);
        try {
            return new String(encoded, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            throw new StringCodeException();
        }
    }

    /** @brief 'int8' serializer. */
    public static void int8_serialize(ByteArrayOutputStream out, byte in) {
        out.write(in);
    }

    /** @brief 'int8' deserializer. */
    public static byte int8_deserialize(ByteBuffer in) {
        return in.get();
    }

    /** @brief 'uint8' serializer. */
    public static void uint8_serialize(ByteArrayOutputStream out, byte in) {
        int8_serialize(out, in);
    }

    /** @brief 'uint8' deserializer. */
    public static byte uint8_deserialize(ByteBuffer in) {
        return int8_deserialize(in);
    }

    /** @brief 'int16' serializer. */
    public static void int16_serialize(ByteArrayOutputStream out, short in) {
        out.write((in >> 8) & 0xff);
        out.write(in & 0xff);
    }

    /** @brief 'int16' deserializer. */
    public static short int16_deserialize(ByteBuffer in) {
        short ret = (short)((0xff & in.get()) << 8);
        ret |= (short)(0xff & in.get());
        return ret;
    }

    /** @brief 'uint16' serializer. */
    public static void uint16_serialize(ByteArrayOutputStream out, short in) {
        int16_serialize(out, in);
    }

    /** @brief 'uint16' deserializer. */
    public static short uint16_deserialize(ByteBuffer in) {
        return int16_deserialize(in);
    }

    /** @brief 'int32' serializer. */
    public static void int32_serialize(ByteArrayOutputStream out, int in) {
        out.write((in >> 24) & 0xff);
        out.write((in >> 16) & 0xff);
        out.write((in >> 8) & 0xff);
        out.write(in & 0xff);
    }

    /** @brief 'int32' deserializer. */
    public static int int32_deserialize(ByteBuffer in) {
        int ret = (0xff & in.get()) << 24;
        ret |= (0xff & in.get()) << 16;
        ret |= (0xff & in.get()) << 8;
        ret |= 0xff & in.get();
        return ret;
    }

    /** @brief 'uint32' serializer. */
    public static void uint32_serialize(ByteArrayOutputStream out, int in) {
        int32_serialize(out, in);
    }

    /** @brief 'uint32' deserializer. */
    public static int uint32_deserialize(ByteBuffer in) {
        return int32_deserialize(in);
    }

    /** @brief 'int64' serializer. */
    public static void int64_serialize(ByteArrayOutputStream out, long in) {
        out.write((int)((in >> 56) & 0xff));
        out.write((int)((in >> 48) & 0xff));
        out.write((int)((in >> 40) & 0xff));
        out.write((int)((in >> 32) & 0xff));
        out.write((int)((in >> 24) & 0xff));
        out.write((int)((in >> 16) & 0xff));
        out.write((int)((in >> 8) & 0xff));
        out.write((int)(in & 0xff));
    }

    /** @brief 'int64' deserializer. */
    public static long int64_deserialize(ByteBuffer in) {
        long ret = ((long)(0xff & in.get())) << 56;
        ret |= ((long)(0xff & in.get())) << 48;
        ret |= ((long)(0xff & in.get())) << 40;
        ret |= ((long)(0xff & in.get())) << 32;
        ret |= ((long)(0xff & in.get())) << 24;
        ret |= ((long)(0xff & in.get())) << 16;
        ret |= ((long)(0xff & in.get())) << 8;
        ret |= (long)(0xff & in.get());
        return ret;
    }

    /** @brief 'uint64' serializer. */
    public static void uint64_serialize(ByteArrayOutputStream out, long in) {
        int64_serialize(out, in);
    }

    /** @brief 'uint64' deserializer. */
    public static long uint64_deserialize(ByteBuffer in) {
        return int64_deserialize(in);
    }
}
