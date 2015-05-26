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
package orp.demo.urchat;

import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;

import orp.orp.Endpoint;
import orp.orp.Outbound;
import orp.orp.Session;
import orp.orp.SessionManager;
import orp.orp.TIDL;

/**
 */
public class UrChat {
    public static enum State {
        AWAITING_LOGIN,
        AWAITING_COMMANDS
    }

    private static final short CHAT_PUBKEY_LEN = 70;
    public static final short SEEKS_SIZE = 4;
    public static final short SEEK_SIZE = 32;
    public static final short SESSION_BUFFER_SIZE = 512;

    public static final byte[] chat_hash =
            {0x63, 0x68, 0x61, 0x74, 0xa, 0x0, 0x0, 0x0,
             0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
             0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
             0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
    public static final short chat_epnum = 0x1337;

    private SessionManager sessionManager;
    private Session session;
    private State state;
    private byte[] pubkey;

    private UrChat() {
        sessionManager = null;
        session = null;
        state = State.AWAITING_LOGIN;
        pubkey = null;
    }

    public static UrChat getUrChat(SessionManager in_sessionManager) {
        UrChat ret = new UrChat();

        try {
            ret.sessionManager = in_sessionManager;
            Endpoint chat_ep = new Endpoint(chat_hash, (short) chat_epnum);
            SessionManager.PreSession preSession = ret.sessionManager.newSession(chat_ep);
            ret.session = preSession.getSession();
            ret.state = State.AWAITING_LOGIN;
        } catch (Exception e) {
            e.printStackTrace();
            return null;
        }

        return ret;
    }

    public byte[] getPubkey() {
        if (State.AWAITING_COMMANDS != state) return null;
        return pubkey;
    }

    public synchronized boolean login(byte[] pass) throws InterruptedException {
        if (State.AWAITING_LOGIN != state) {
            Log.d("UrChat", String.format("login failed: State.AWAITING_LOGIN != %s", state.toString()));
            return false;
        }
        if (pass.length != 32) {
            Log.d("UrChat", String.format("login failed: password length == %d != 32", pass.length));
            return false;
        }
        try {
            if (Outbound.State.SENT_ACK != session.writeBlocking(pass))
                return false;
            ByteBuffer response = ByteBuffer.wrap(session.read());
            ChatResponse response_type = ChatResponse.Deserialize(response);
            if (ChatResponse.CR_PUBKEY != response_type) return false;
            short keylen = TIDL.uint16_deserialize(response);
            if (CHAT_PUBKEY_LEN != keylen) return false;
            pubkey = new byte[keylen];
            for (int i = 0; i < pubkey.length; i++)
                pubkey[i] = TIDL.uint8_deserialize(response);
        } catch (Outbound.ETooMuchData eTooMuchData) {
            eTooMuchData.printStackTrace();
            return false;
        } catch (TIDL.TIDLException e) {
            e.printStackTrace();
            return false;
        }
        Log.d("UrChat", "Logged in");
        state = State.AWAITING_COMMANDS;
        return true;
    }

    public synchronized boolean logout() throws InterruptedException {
        try {
            ByteArrayOutputStream command = new ByteArrayOutputStream();
            ChatCommand.Serialize(command, ChatCommand.CC_LOGOUT);
            if (Outbound.State.SENT_ACK != session.writeBlocking(command.toByteArray()))
                return false;
            ByteBuffer response = ByteBuffer.wrap(session.read());
            ChatResponse response_type = ChatResponse.Deserialize(response);
            if (ChatResponse.CR_GOODBYE != response_type) return false;
        } catch (Outbound.ETooMuchData eTooMuchData) {
            eTooMuchData.printStackTrace();
            return false;
        } catch (TIDL.TIDLException e) {
            e.printStackTrace();
            return false;
        }
        state = State.AWAITING_LOGIN;
        pubkey = null;
        Log.d("UrChat", "Logged out");
        return true;
    }

    public synchronized boolean setSeek(byte[][] seeking) throws InterruptedException {
        if (State.AWAITING_COMMANDS != state) return false;
        if (!(seeking.length < SEEKS_SIZE)) {
            Log.d("UrChat", String.format("setSeek failed: seeking.length == %d > %d == SEEKS_SIZE", seeking.length, SEEKS_SIZE));
            return false;
        }
        for (int i = 0; i < seeking.length; i++) {
            if (SEEK_SIZE != seeking[i].length) {
                Log.d("UrChat", String.format("setSeek failed: seeking[i].length == %d != $d == SEEK_SIZE", seeking[i].length, SEEK_SIZE));
                return false;
            }
        }
        try {
            ByteArrayOutputStream command = new ByteArrayOutputStream();
            ChatCommand.Serialize(command, ChatCommand.CC_SETSEEK);
            TIDL.uint32_serialize(command, seeking.length);
            for (int i = 0; i < seeking.length; i++)
                for (int j = 0; j < seeking[i].length; j++)
                    TIDL.uint8_serialize(command, seeking[i][j]);
            if (Outbound.State.SENT_ACK != session.writeBlocking(command.toByteArray()))
                return false;
            ByteBuffer response = ByteBuffer.wrap(session.read());
            ChatResponse response_type = ChatResponse.Deserialize(response);
            if (ChatResponse.CR_SEEKUPDATED != response_type) return false;
        } catch (Outbound.ETooMuchData eTooMuchData) {
            eTooMuchData.printStackTrace();
            return false;
        } catch (TIDL.TIDLException e) {
            e.printStackTrace();
            return false;
        }
        Log.d("UrChat", String.format("Set seek (%d active)", seeking.length));
        return true;
    }

    public synchronized byte[] getSeek(int seeknum) throws InterruptedException {
        try {
            ByteArrayOutputStream command = new ByteArrayOutputStream();
            ChatCommand.Serialize(command, ChatCommand.CC_GETSEEK);
            TIDL.uint32_serialize(command, seeknum);
            if (Outbound.State.SENT_ACK != session.writeBlocking(command.toByteArray()))
                return null;
            ByteBuffer response = ByteBuffer.wrap(session.read());
            ChatResponse response_type = ChatResponse.Deserialize(response);
            if (ChatResponse.CR_SEEK != response_type) return null;
            byte[] ret = new byte[SESSION_BUFFER_SIZE];
            for (int i = 0; i < SESSION_BUFFER_SIZE; i++)
                ret[i] = TIDL.uint8_deserialize(response);
            Log.d("UrChat", String.format("Fetched seek string for seek-slot %d", seeknum));
            return ret;
        } catch (Outbound.ETooMuchData eTooMuchData) {
            eTooMuchData.printStackTrace();
        } catch (TIDL.TIDLException e) {
            e.printStackTrace();
        }
        return null;
    }

    public static final int INVALID_CHANNEL = -1;

    public synchronized int getChannel(byte[] remote_pubkey) throws InterruptedException {
        if (CHAT_PUBKEY_LEN != remote_pubkey.length) return INVALID_CHANNEL;
        try {
            ByteArrayOutputStream command = new ByteArrayOutputStream();
            ChatCommand.Serialize(command, ChatCommand.CC_GET_CHANNEL);
            TIDL.uint16_serialize(command, (short) remote_pubkey.length);
            for (int i = 0; i < remote_pubkey.length; i++)
                TIDL.uint8_serialize(command, remote_pubkey[i]);
            if (Outbound.State.SENT_ACK != session.writeBlocking(command.toByteArray()))
                return INVALID_CHANNEL;
            ByteBuffer response = ByteBuffer.wrap(session.read());
            ChatResponse response_type = ChatResponse.Deserialize(response);
            if (ChatResponse.CR_CHANNEL != response_type) return INVALID_CHANNEL;
            int chan = TIDL.uint32_deserialize(response);
            Log.d("UrChat", String.format("Obtained channel (%d) with contact", chan));
            return chan;
        } catch (Outbound.ETooMuchData eTooMuchData) {
            eTooMuchData.printStackTrace();
        } catch (TIDL.TIDLException e) {
            e.printStackTrace();
        }
        return INVALID_CHANNEL;
    }

    public synchronized boolean closeChannel(int channel_id) throws InterruptedException {
        try {
            ByteArrayOutputStream command = new ByteArrayOutputStream();
            ChatCommand.Serialize(command, ChatCommand.CC_CLOSE_CHANNEL);
            TIDL.uint32_serialize(command, channel_id);
            if (Outbound.State.SENT_ACK != session.writeBlocking(command.toByteArray()))
                return false;
            ByteBuffer response = ByteBuffer.wrap(session.read());
            ChatResponse response_type = ChatResponse.Deserialize(response);
            if (ChatResponse.CR_CHANNEL_CLOSED != response_type) return false;
        } catch (Outbound.ETooMuchData eTooMuchData) {
            eTooMuchData.printStackTrace();
            return false;
        } catch (TIDL.TIDLException e) {
            e.printStackTrace();
            return false;
        }
        Log.d("UrChat", String.format("Closed channel %d", channel_id));
        return true;
    }

    public synchronized byte[] getExchange(int channel_id) throws InterruptedException {
        try {
            ByteArrayOutputStream command = new ByteArrayOutputStream();
            ChatCommand.Serialize(command, ChatCommand.CC_GET_EXCHANGE);
            TIDL.uint32_serialize(command, channel_id);
            if (Outbound.State.SENT_ACK != session.writeBlocking(command.toByteArray()))
                return null;
            ByteBuffer response = ByteBuffer.wrap(session.read());
            ChatResponse response_type = ChatResponse.Deserialize(response);
            if (ChatResponse.CR_EXCHANGE != response_type) return null;
            byte[] ret = new byte[SESSION_BUFFER_SIZE];
            for (int i = 0; i < SESSION_BUFFER_SIZE; i++)
                ret[i] = TIDL.uint8_deserialize(response);
            Log.d("UrChat", String.format("Obtained exchange string for channel ", channel_id));
            return ret;
        } catch (Outbound.ETooMuchData eTooMuchData) {
            eTooMuchData.printStackTrace();
        } catch (TIDL.TIDLException e) {
            e.printStackTrace();
        }
        return null;
    }

    public synchronized byte[] encrypt(int channel_id, String message) throws InterruptedException {
        try {
            ByteArrayOutputStream command = new ByteArrayOutputStream();
            ChatCommand.Serialize(command, ChatCommand.CC_ENCRYPT);
            TIDL.uint32_serialize(command, channel_id);
            TIDL.string_serialize(command, message);
            if (Outbound.State.SENT_ACK != session.writeBlocking(command.toByteArray()))
                return null;
            ByteBuffer response = ByteBuffer.wrap(session.read());
            ChatResponse response_type = ChatResponse.Deserialize(response);
            if (ChatResponse.CR_CIPHERTEXT != response_type) return null;
            byte[] ret = new byte[SESSION_BUFFER_SIZE];
            for (int i = 0; i < SESSION_BUFFER_SIZE; i++)
                ret[i] = TIDL.uint8_deserialize(response);
            Log.d("UrChat", String.format("Encrypted message for channel %d", channel_id));
            return ret;
        } catch (Outbound.ETooMuchData eTooMuchData) {
            eTooMuchData.printStackTrace();
        } catch (TIDL.TIDLException e) {
            e.printStackTrace();
        }
        return null;
    }

    public static class Decrypted {
        public static enum Kind {
            SEEK_FOUND,
            EXCHANGE_OCCURRED,
            PLAINTEXT_GOT,
            NEVERMIND
        }

        public final Kind kind;
        public final int seek;
        public final byte[] pubkey;
        public final int channel_id;
        public final String message;

        private Decrypted(int in_seek, byte[] in_pubkey) {
            kind = Kind.SEEK_FOUND;
            seek = in_seek;
            pubkey = in_pubkey;
            channel_id = INVALID_CHANNEL;
            message = null;
        }
        private Decrypted(int in_channel_id) {
            kind = Kind.EXCHANGE_OCCURRED;
            seek = -1;
            pubkey = null;
            channel_id = in_channel_id;
            message = null;
        }
        private Decrypted(int in_channel_id, String in_message) {
            kind = Kind.PLAINTEXT_GOT;
            seek = -1;
            pubkey = null;
            channel_id = in_channel_id;
            message = in_message;
        }
        private Decrypted() {
            kind = Kind.NEVERMIND;
            seek = -1;
            pubkey = null;
            channel_id = INVALID_CHANNEL;
            message = null;
        }

        protected static Decrypted seekFound(int in_seek, byte[] in_pubkey) {
            return new Decrypted(in_seek, in_pubkey);
        }

        protected static Decrypted exchangeOccurred(int in_channel_id) {
            return new Decrypted(in_channel_id);
        }

        protected static Decrypted plaintextGot(int in_channel_id, String in_message) {
            return new Decrypted(in_channel_id, in_message);
        }

        protected  static Decrypted nevermind() {
            return new Decrypted();
        }
    }

    public synchronized Decrypted incoming(byte[] ciphertext) throws InterruptedException {
        if (SESSION_BUFFER_SIZE != ciphertext.length) return null;
        try {
            ByteArrayOutputStream command = new ByteArrayOutputStream();
            ChatCommand.Serialize(command, ChatCommand.CC_INCOMING);
            for (int i = 0; i < ciphertext.length; i++)
                TIDL.uint8_serialize(command, ciphertext[i]);
            if (Outbound.State.SENT_ACK != session.writeBlocking(command.toByteArray()))
                return null;
            ByteBuffer response = ByteBuffer.wrap(session.read());
            ChatResponse response_type = ChatResponse.Deserialize(response);
            switch (response_type) {
                case CR_FOUND: {
                    int seeknum = TIDL.uint32_deserialize(response);
                    short keylen = TIDL.uint16_deserialize(response);
                    if (CHAT_PUBKEY_LEN != keylen) return null;
                    byte[] pubkey = new byte[keylen];
                    for (int i = 0; i < pubkey.length; i++)
                        pubkey[i] = TIDL.uint8_deserialize(response);
                    Log.d("UrChat", String.format("Seek found (seeknum %d)", seeknum));
                    return Decrypted.seekFound(seeknum, pubkey);
                }
                case CR_EXCHANGED: {
                    int channel_id = TIDL.uint32_deserialize(response);
                    Log.d("UrChat", String.format("Exchange occurred (channel %d)", channel_id));
                    return Decrypted.exchangeOccurred(channel_id);
                }
                case CR_PLAINTEXT: {
                    int channel_id = TIDL.uint32_deserialize(response);
                    String message = TIDL.string_deserialize(response);
                    Log.d("UrChat", String.format("Plaintext got (channel %d)", channel_id));
                    return Decrypted.plaintextGot(channel_id, message);
                }
                case CR_NEVERMIND:
                    return Decrypted.nevermind();
                default:
                    return null;
            }
        } catch (Outbound.ETooMuchData eTooMuchData) {
            eTooMuchData.printStackTrace();
        } catch (TIDL.TIDLException e) {
            e.printStackTrace();
        }
        return null;
    }
}
