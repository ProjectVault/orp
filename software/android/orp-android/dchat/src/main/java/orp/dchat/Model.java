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
package orp.dchat;

import android.os.Environment;
import android.util.Base64;

import java.security.MessageDigest;
import java.util.ArrayList;

import orp.demo.urchat.ChatRoom;
import orp.demo.urchat.UrChat;
import orp.orp.FileBus;
import orp.orp.IBus;
import orp.orp.IPBus;
import orp.orp.SessionManager;

public class Model {
    public static enum PeripheralSelector {
        EMULATED_1,
        EMULATED_2,
        FAUX_FILE_SYSTEM
    }

    private static Model model = null;

    private PeripheralSelector selector = PeripheralSelector.EMULATED_1;
    private String demoUrl = "";
    private int room = 1;
    private String seekName = null;
    private String seekPhrase = null;
    private String userName = null;
    private String userPass = null;
    private UrChat urChat = null;
    private ChatRoom chatRoom = null;

    private Contacts contactsActivity = null;
    private Conversation conversationActivity = null;

    private Model() {

    }

    public static Model getModel() {
        if (null == model)
            model = new Model();
        return model;
    }

    public void setSelector(PeripheralSelector in_selector) {
        selector = in_selector;
    }

    public void setDemoUrl(String in_demoUrl) {
        demoUrl = in_demoUrl;
    }

    public void setRoom(int in_room) {
        room = in_room;
    }

    public void setSeekName(String in_seekName) {
        seekName = in_seekName;
    }

    public String getSeekName() {
        return seekName;
    }

    public void setSeekPhrase(String in_seekPhrase) {
        seekPhrase = in_seekPhrase;
    }

    public void setUserName(String in_userName) {
        userName = in_userName;
    }

    public String getUserName() {
        return userName;
    }

    public void setUserPass(String in_userPass) {
        userPass = in_userPass;
    }

    public void setContactsActivity(Contacts in_contactsActivity) {
        contactsActivity = in_contactsActivity;
    }

    public void setConversationActivity(Conversation in_conversationActivity) {
        conversationActivity = in_conversationActivity;
    }

    public boolean initialize() {
        try {
            IBus bus = null;
            switch (selector) {
                case EMULATED_1:
                    bus = new IPBus("10.0.2.2", 9998, 9999);
                    break;
                case EMULATED_2:
                    bus = new IPBus("10.0.2.2", 9988, 9989);
                    break;
                case FAUX_FILE_SYSTEM:
                    String sdPath = Environment.getExternalStorageDirectory().getPath();
                    String rfile = String.format("%s/RFILE", sdPath);
                    String wfile = String.format("%s/WFILE", sdPath);
                    bus = new FileBus(false, rfile, wfile);
                    break;
            }
            urChat = UrChat.getUrChat(new SessionManager(bus));
            return (null != urChat);
        }
        catch (Exception e) {
            return false;
        }
    }

    public boolean login() {
        try {
            MessageDigest digest = MessageDigest.getInstance("SHA-256");
            digest.reset();
            byte[] credential = digest.digest(String.format("%s:%s", userName, userPass).getBytes("UTF-8"));
            return urChat.login(credential);
        }
        catch (Exception e) {
            return false;
        }
    }

    public boolean connect() {
        try {
            chatRoom = new ChatRoom(demoUrl, room);
            return true;
        } catch (ChatRoom.CannotConnectException e) {
            return false;
        }
    }

    public static class Contact {
        protected String name = null;
        protected String status = null;
        protected boolean isOpen = false;
        protected int channel = UrChat.INVALID_CHANNEL;

        public Contact(String in_name) {
            name = in_name;
        }

        public String getName() {
            return name;
        }

        public String getStatus() {
            return status;
        }

        @Override
        public String toString() {
            return String.format("%s\n%s", getName(), getStatus());
        }

        public boolean channelIsOpen() {
            return isOpen && (channel != UrChat.INVALID_CHANNEL);
        }
    }

    private ArrayList<Contact> contacts = new ArrayList<>();

    public ArrayList<Contact> getContacts() {
        return contacts;
    }

    private void notifyContactsUpdate() {
        if (null != contactsActivity) {
            contactsActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    contactsActivity.notifyContactsChange();
                }
            });
        }
    }

    private Contact contact = null;

    public boolean initializeContact() {
        try {
            // Set our seek preferences on the peripheral
            {
                MessageDigest digest = MessageDigest.getInstance("SHA-256");
                digest.reset();
                byte[] seekBytes = digest.digest(seekPhrase.getBytes("UTF-8"));
                byte[][] seeking = { seekBytes };
                if (!urChat.setSeek(seeking))
                    return false;
            }

            Contact c = new Contact(seekName);
            c.status = "Seeking...";
            contacts.add(c);
            notifyContactsUpdate();

            // Fetch two seek strings
            byte[] seek1 = urChat.getSeek(0);
            byte[] seek2 = urChat.getSeek(0);

            // Transmit the first seek string
            chatRoom.say(Base64.encodeToString(seek1, 0));

            // Process incoming data until a seek-match occurs
            byte[] pubkey = null;
            while (true) {
                String incoming = chatRoom.incoming.take();
                byte[] decoded = Base64.decode(incoming, 0);
                if (null == decoded)
                    continue;
                UrChat.Decrypted decrypted = urChat.incoming(decoded);
                if (null == decrypted)
                    continue;
                if (UrChat.Decrypted.Kind.SEEK_FOUND == decrypted.kind) {
                    pubkey = decrypted.pubkey;
                    break;
                }
            }

            // Clear our seek preferences
            {
                byte[][] seeking = { };
                if (!urChat.setSeek(seeking))
                    return false;
            }

            // Transmit the second seek string
            chatRoom.say(Base64.encodeToString(seek2, 0));

            // Open a channel to the remote pubkey
            c.channel = urChat.getChannel(pubkey);
            if (UrChat.INVALID_CHANNEL == c.channel)
                return false;

            byte[] exchange1 = urChat.getExchange(c.channel);
            if (null == exchange1)
                return false;
            byte[] exchange2 = urChat.getExchange(c.channel);
            if (null == exchange2)
                return false;

            c.status = "Exchanging keys...";
            notifyContactsUpdate();

            // Send an exchange string
            chatRoom.say(Base64.encodeToString(exchange1, 0));

            // Loop until we recv an exchange
            while (true) {
                String incoming = chatRoom.incoming.take();
                byte[] decoded = Base64.decode(incoming, 0);
                if (null == decoded)
                    continue;
                UrChat.Decrypted decrypted = urChat.incoming(decoded);
                if (null == decrypted)
                    continue;
                if (UrChat.Decrypted.Kind.EXCHANGE_OCCURRED == decrypted.kind)
                    break;
            }

            // Send the second exchange string
            chatRoom.say(Base64.encodeToString(exchange2, 0));

            // Channel is open
            contact = c;
            c.isOpen = true;
            c.status = "Channel open";
            notifyContactsUpdate();
            return true;
        }
        catch (Exception e) {
            return false;
        }
    }

    private ArrayList<String> chatLog = new ArrayList<>();

    public ArrayList<String> getChatLog() {
        return chatLog;
    }

    private void notifyLogUpdate() {
        if (null != conversationActivity) {
            conversationActivity.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    conversationActivity.notifyLogChange();
                }
            });
        }
    }

    public boolean say(String message) {
        try {
            byte[] encrypted = urChat.encrypt(contact.channel, message);
            if (null == encrypted)
                return false;
            chatLog.add(String.format("me: %s", message));
            chatRoom.say(Base64.encodeToString(encrypted, 0));
            notifyLogUpdate();
            return true;
        }
        catch (Exception e) {
            return false;
        }
    }

    public void listenLoop() {
        try {
            while (true) {
                String incoming = chatRoom.incoming.take();
                byte[] decoded = Base64.decode(incoming, 0);
                if (null == decoded)
                    continue;
                UrChat.Decrypted decrypted = urChat.incoming(decoded);
                if (null == decrypted)
                    continue;
                if (UrChat.Decrypted.Kind.PLAINTEXT_GOT == decrypted.kind) {
                    chatLog.add(String.format("%s: %s", contact.name, decrypted.message));
                    notifyLogUpdate();
                }
            }
        }
        catch (Exception e) {

        }
    }
}
