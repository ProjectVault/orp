/** @file FileWriteChannel.java */
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

import java.util.Arrays;

/** @brief Faux Filesystem (write channel) over file system */
public class FileWriteChannel implements IWriteChannel {
    private PosixFile file;

    protected FileWriteChannel(boolean in_useRoot, String filename) throws PosixFile.ErrnoException {
        file = new PosixFile(in_useRoot, filename);
        file.open();
    }

    @Override
    public ChannelStatus status(int nonce) {
        byte[] formatted = new byte[SessionManager.STATUS_LENGTH];
        file.readFully(formatted);
        int status = formatted[0];
        int nonce_ = formatted[1];
        if (0x10 == status && 0x00 == nonce)
            return ChannelStatus.CHANNEL_OK;
        if (nonce_ == nonce) {
            switch (status) {
                case 0x10: /* ready */
                    return ChannelStatus.CHANNEL_WAIT;
                case 0x12: /* success */
                    return ChannelStatus.CHANNEL_OK;
                case 0x11: /* error */
                case 0x14: /* einput */
                    return ChannelStatus.CHANNEL_ERROR;
                case 0x13: /* retry */
                    return ChannelStatus.CHANNEL_RETRY;
            }
        }
        return ChannelStatus.CHANNEL_WAIT;
    }

    @Override
    public void write(byte[] out) {
        byte[] formatted = Arrays.copyOf(out, SessionManager.PACKET_LENGTH);
        file.write(formatted);
    }
}
