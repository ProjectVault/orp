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
#include "orp_orp_PosixFile.h"

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

JNIEXPORT jint JNICALL Java_orp_orp_PosixFile_internal_1open(JNIEnv *env, jobject obj, jboolean useRoot, jstring in_path) {
    const char *native_path = (*env)->GetStringUTFChars(env, in_path, 0);
    int flags = O_RDWR | O_SYNC | O_DIRECT;
    if (JNI_FALSE == useRoot)
        flags |= O_DIRECT;
    int ret = open(native_path, flags);
    (*env)->ReleaseStringUTFChars(env, in_path, native_path);
    return ret;
}

JNIEXPORT jboolean JNICALL Java_orp_orp_PosixFile_internal_1close(JNIEnv *env, jobject obj, jboolean useRoot, jint in_fd) {
    return (0 == close(in_fd)) ? JNI_TRUE : JNI_FALSE;
}

JNIEXPORT jint JNICALL Java_orp_orp_PosixFile_internal_1write(JNIEnv *env, jobject obj, jboolean useRoot, jint in_fd, jbyteArray in_data) {
    jint ret = 0;

    jbyte* bufferPtr = (*env)->GetByteArrayElements(env, in_data, NULL);
    jsize lengthOfArray = (*env)->GetArrayLength(env, in_data);

    void *aligned_buf = memalign(PAGESIZE, 2048);
    if (!aligned_buf)
        return 0;

    do {
        if(lseek(in_fd, 0, SEEK_SET) == -1) {
            ret = -errno;
            break;
        }

        memset(aligned_buf, 0, 2048);
        memcpy(aligned_buf, bufferPtr, lengthOfArray);

        int i = 0;
        for (i = 0; i < 2048; i += 512) {
            int r = write(in_fd, aligned_buf + i, 512);
            if (-1 == r) {
                ret = -errno;
                break;
            }
        }

        ret = lengthOfArray;
    } while (0);

    (*env)->ReleaseByteArrayElements(env, in_data, bufferPtr, 0);
    return ret;
}

JNIEXPORT jint JNICALL Java_orp_orp_PosixFile_internal_1readFull(JNIEnv *env, jobject obj, jboolean useRoot, jint in_fd, jbyteArray out_data) {
    jint ret = 0;

    jbyte* bufferPtr = (*env)->GetByteArrayElements(env, out_data, NULL);
    jsize lengthOfArray = (*env)->GetArrayLength(env, out_data);

    void *aligned_buf = memalign(PAGESIZE, 2048);
    if (!aligned_buf)
        return 0;

    do {
        if(lseek(in_fd, 0, SEEK_SET) == -1) {
            break;
        }

        if (JNI_TRUE == useRoot)
            system("su -c \"echo 1 > /proc/sys/vm/drop_caches\"");

        memset(aligned_buf, 0 , 2048);
        ret = read(in_fd, aligned_buf, 2048);
        if (-1 == ret) {
            ret = -errno;
            break;
        }

        ret = lengthOfArray;
        memcpy(bufferPtr, aligned_buf, lengthOfArray);
    } while (0);

    (*env)->ReleaseByteArrayElements(env, out_data, bufferPtr, 0);
    return ret;
}

JNIEXPORT jint JNICALL Java_orp_orp_PosixFile_getErrno(JNIEnv *env, jobject obj) {
    return errno;
}
