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

import java.io.BufferedInputStream;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.UnsupportedEncodingException;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.SocketTimeoutException;
import java.net.URL;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.util.Scanner;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 */
public class ChatRoom {
    public static class CannotConnectException extends Exception {
        public final Exception inner;

        public CannotConnectException() {
            inner = null;
        }

        public CannotConnectException(Exception in_inner) {
            inner = in_inner;
        }
    }

    protected static class IncomingConsumer implements Runnable {
        private final String demoUrl;
        private final String client_id;
        private final String client_pass;
        private BlockingQueue<String> incoming;

        protected IncomingConsumer(String in_demoUrl, String in_client_id, String in_client_pass, BlockingQueue<String> in_incoming) {
            demoUrl = in_demoUrl;
            client_id = in_client_id;
            client_pass = in_client_pass;
            incoming = in_incoming;
        }

        @Override
        public void run() {
            consumer_loop: while(true) {
                try {
                    URL msgs_url = new URL(ChatRoom.msgsUrl(demoUrl));
                    HttpURLConnection msgs_connection = (HttpURLConnection)msgs_url.openConnection();
                    msgs_connection.setDoOutput(true);  /* Sets HTTP POST */
                    msgs_connection.addRequestProperty("UrWeb-Pass", client_pass);
                    msgs_connection.addRequestProperty("UrWeb-Client", client_id);
                    msgs_connection.setReadTimeout(15000);
                    msgs_connection.connect();

                    try {
                        inbound_loop:
                        while (200 == msgs_connection.getResponseCode()) {
                            try {
                                BufferedReader reader = new BufferedReader(new InputStreamReader(msgs_connection.getInputStream()));
                                String code = reader.readLine();
                                String encoded = reader.readLine();
                                String decoded = ChatRoom.decode(encoded);
                                if (decoded.startsWith("zyx:"))
                                    incoming.put(decoded.substring(4));
                                break inbound_loop;
                            } catch (SocketTimeoutException e) {
                                // is fine
                            }
                        }
                    } catch (SocketTimeoutException e) {
                        continue consumer_loop;
                    }
                } catch (MalformedURLException e) {
                    e.printStackTrace();
                    break consumer_loop;
                } catch (IOException e) {
                    e.printStackTrace();
                    break consumer_loop;
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    public final String demoUrl;
    public final int room;
    public String client_id;
    public String client_pass;
    public BlockingQueue<String> incoming;
    public Thread consumer;

    public static String encode(String d0) {
        String d1 = d0.replaceAll("\\.", ".2E");
        String d2 = d1;
        try {
            d2 = URLEncoder.encode(d1, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
        return d2.replace('%', '.');
    }

    public static String decode(String d0) {
        String d1 = d0.replaceAll("\\.", "%");
        String d2 = d1.replaceAll("%2E", ".");
        String d3 = d2;
        try {
            d3 = URLDecoder.decode(d2, "UTF-8");
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }
        return d3;
    }

    public static String chatUrl(String demoUrl, int room) {
        return String.format("%s/Chat/chat/%d", demoUrl, room);
    }

    public static String msgsUrl(String demoUrl) {
        return String.format("%s/.msgs", demoUrl);
    }

    public static String speakUrl(String demoUrl, int room, String msg) {
        return String.format("%s/Chat/speak/%d/%s", demoUrl, room, encode(msg));
    }

    public ChatRoom(String in_demoUrl, int in_room) throws CannotConnectException {
        demoUrl = in_demoUrl;
        room = in_room;
        incoming = new LinkedBlockingQueue<>();
        try {
            URL join_url = new URL(chatUrl(demoUrl, room));
            HttpURLConnection join_connection = (HttpURLConnection)join_url.openConnection();
            join_connection.setDoOutput(true);  /* Sets HTTP POST */
            InputStream join_stream = new BufferedInputStream(join_connection.getInputStream());

            /* Find our client_id and client_pass */
            Scanner join_scanner = new Scanner(join_stream);
            Pattern credentials_pattern = Pattern.compile("client_id=\\d+;client_pass=\\d+;");
            String credential_string = join_scanner.findWithinHorizon(credentials_pattern, 0);

            Pattern value_pattern = Pattern.compile("=(\\d+);");
            Matcher value_matcher = value_pattern.matcher(credential_string);

            if (!value_matcher.find())
                throw new CannotConnectException();
            client_id = value_matcher.group(1);

            if (!value_matcher.find())
                throw new CannotConnectException();
            client_pass = value_matcher.group(1);

            join_connection.disconnect();

            consumer = new Thread(new IncomingConsumer(demoUrl, client_id, client_pass, incoming));
            consumer.start();
        } catch (IOException e) {
            throw new CannotConnectException(e);
        }
    }

    public void say(String msg) throws IOException {
        try {
            URL say_url = new URL(speakUrl(demoUrl, room, String.format("zyx:%s", msg)));
            HttpURLConnection say_connection = (HttpURLConnection)say_url.openConnection();
            say_connection.setDoOutput(true);  /* Sets HTTP POST */
            say_connection.addRequestProperty("UrWeb-Pass", client_pass);
            say_connection.addRequestProperty("UrWeb-Client", client_id);
            say_connection.connect();
            say_connection.getInputStream().close();
            say_connection.disconnect();
        } catch (MalformedURLException e) {
            e.printStackTrace();
        }
    }
}
