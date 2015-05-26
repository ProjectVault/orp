/** @file FileBus.java */
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

import android.util.Log;

/** @brief Faux Filesystem over file system. */
public class FileBus implements IBus {
    private final FileReadChannel readChannel;
    private final FileWriteChannel writeChannel;

    public FileBus(boolean in_useRoot, String rFile, String wFile) throws PosixFile.ErrnoException {
        Log.d("FileBus", String.format("rfile: %s, wfile: %s", rFile, wFile));
        readChannel = new FileReadChannel(in_useRoot, rFile);
        writeChannel = new FileWriteChannel(in_useRoot, wFile);
    }

    @Override
    public IReadChannel readChannel() {
        return readChannel;
    }

    @Override
    public IWriteChannel writeChannel() {
        return writeChannel;
    }
}
