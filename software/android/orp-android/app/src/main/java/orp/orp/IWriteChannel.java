/** @file IWriteChannel.java */
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

/** @brief Interface for a Faux Filesystem write channel. */
public interface IWriteChannel {
    public static enum ChannelStatus {
        CHANNEL_OK,
        CHANNEL_ERROR,
        CHANNEL_RETRY,
        CHANNEL_WAIT
    }

    /** @brief Fetch channel status. */
    public ChannelStatus status(int nonce);

    /** @brief Write to channel. */
    public void write(byte[] out);
}
