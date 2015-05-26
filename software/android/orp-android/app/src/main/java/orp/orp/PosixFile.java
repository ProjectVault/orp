/** @file PosixFile.java */
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

/**
 * @brief Wrapper for POSIX file access (via native calls).
 */
public class PosixFile {
    public static class ErrnoException extends Exception {
        public final String path;
        public final int errno;

        private ErrnoException(String in_path, int in_errno) {
            path = in_path;
            errno = in_errno;
        }

        @Override
        public String getMessage() {
            return String.format("Error for file %s: %d", path, errno);
        }
    }

    static {
        System.loadLibrary("taPosix");
    }

    private final boolean useRoot;
    private final String path;
    private int fd = -1;

    public PosixFile(boolean in_useRoot, String in_path) {
        useRoot = in_useRoot;
        path = in_path;
    }

    private String bufToString(byte[] in) {
        StringBuilder sb = new StringBuilder(in.length * 2);
        for (int i = 0; i < in.length; i++)
            sb.append(String.format("%02x", in[i]));
        return sb.toString();
    }

    public boolean isOpen() {
        return (fd != -1);
    }

    public boolean open() throws ErrnoException {
        fd = internal_open(useRoot, path);
        if (!isOpen()) {
            throw new ErrnoException(path, getErrno());
        }
        return true;
    }

    public boolean close() {
        if (internal_close(useRoot, fd))
            fd = -1;
        return !isOpen();
    }

    public boolean write(byte[] in_data) {
        if (!isOpen())
            return false;
        int x = internal_write(useRoot, fd, in_data);
        boolean ret = (in_data.length == x);
        if (ret)
            Log.d("PosixFile write", String.format("%s wrote %d bytes: %s", path, x, bufToString(in_data)));
        else if (x > 0)
            Log.e("PosixFile write", String.format("%s fail wrote %d of %d", path, x, in_data.length));
        else
            Log.e("PosixFile write", String.format("%s fail errno %d trying to write %d bytes", path, - x, in_data.length));
        return ret;
    }

    public boolean readFully(byte[] out_data) {
        if (!isOpen())
            return false;
        int x = internal_readFull(useRoot, fd, out_data);
        boolean ret = (out_data.length == x);
        if (ret)
            Log.d("PosixFile readFully", String.format("%s read %d bytes: %s", path, x, bufToString(out_data)));
        else if (x > 0)
            Log.e("PosixFile readFully", String.format("%s fail read %d of %d bytes", path, x, out_data.length));
        else
            Log.e("PosixFile read", String.format("%s fail errno %d trying to read %d bytes", path, - x, out_data.length));
        return ret;
    }

    private native int internal_open(boolean in_useRoot, String in_path);
    private native boolean internal_close(boolean in_useRoot, int in_fd);
    private native int internal_write(boolean in_useRoot, int in_fd, byte[] in_data);
    private native int internal_readFull(boolean in_useRoot, int in_fd, byte[] out_data);
    private native int getErrno();
}
