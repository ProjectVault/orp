/** @file FileReadChannel.java */
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

/** @brief Faux Filesystem (read channel) over file system */
public class FileReadChannel implements IReadChannel {
    private PosixFile file;

    protected FileReadChannel(boolean in_useRoot, String filename) throws PosixFile.ErrnoException {
        file = new PosixFile(in_useRoot, filename);
        file.open();
    }

    @Override
    public synchronized byte[] read(int length) {
        byte[] ret = new byte[SessionManager.PACKET_LENGTH];
        file.readFully(ret);
        return ret;
    }

    @Override
    public synchronized void acknowledge(Status status) {
        byte[] formatted = new byte[SessionManager.STATUS_LENGTH];
        switch (status) {
            case ACKNOWLEDGE:
                formatted[0] = 0x10;
                break;
            case ERROR:
                formatted[0] = 0x11;
                break;
        }
        file.write(formatted);
    }
}
