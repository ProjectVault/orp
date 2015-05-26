/** @file IPReadChannel.java */
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

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.SocketException;

/** @brief Faux Filesystem (read channel) over IP */
public class IPReadChannel implements IReadChannel {
    private Socket socket;
    private InputStream inputStream;
    private OutputStream outputStream;

    protected IPReadChannel(String host, int port) throws IOException {
        socket = new Socket(host, port);
        inputStream = socket.getInputStream();
        outputStream = socket.getOutputStream();
    }

    @Override
    public byte[] read(int length) {
        try {
            /* Send read request */
            outputStream.write(0x01);

            /* Collect data */
            byte[] out = new byte[length];
            int bytesRecvd = 0;
            int totalBytesRcvd = 0;  // Total bytes received so far
            int bytesRcvd;           // Bytes received in last read
            while (totalBytesRcvd < out.length) {
                if ((bytesRcvd = inputStream.read(out, totalBytesRcvd, out.length - totalBytesRcvd)) == -1)
                    throw new SocketException("Connection close prematurely");
                totalBytesRcvd += bytesRcvd;
            }
            return out;
        } catch (IOException e) {
            e.printStackTrace();
        }
        return null;
    }

    @Override
    public void acknowledge(Status status) {
        try {
            byte[] formatted = new byte[SessionManager.STATUS_LENGTH];
            switch (status) {
                case ACKNOWLEDGE:
                    formatted[0] = 0x10;
                    break;
                case ERROR:
                    formatted[0] = 0x11;
                    break;
            }
            outputStream.write(formatted);
        }
        catch (IOException e) {
            e.printStackTrace();
        }
    }
}
