/* Copyright 2015, Google Inc.

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


// ORP FUSE layer:
// When mounted, create two directories called ENCRYPT and DECRYPT.
// Files placed in ENCRYPT appear (encrypted) in the DECRYPT folder, and vice versa

#include "orpfs.h"
#include "orpio.h"
#include "dir_table.h"
#include "xtsProto.h"

#include <fuse.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static const char* encrypt_path = "/ENCRYPT";
static const char* decrypt_path = "/DECRYPT";
static int session_id;

// This function returns a pointer to the end of the directory name and the start of the
// filename in a specified path.   FUSE will automatically figure out a 'canonical' path (remove
// /./, /../, ///), so all we have to do is find the last occurrence of '/' and do some pointer
// arithmetic.
//
// If the input is /path/to/a/file, then **pathend = 'a' and **filename = 'f'.
// The one exception is if the input is just "/"; then **filename = '\0' and **pathend = '/'
static void split_fuse_path(const char** pathend, const char** filename, const char* path)
{
    *filename = strrchr(path, '/');
    if (*filename) 
    { 
        *pathend = *filename == path ? *filename : *filename - 1;
        (*filename)++;
    }
    else { *filename = path + strlen(path); *pathend = *filename; }
}

// Return the attributes for a particular file (e.g., stat)
static int orp_getattr(const char* path, struct stat* attribs)
{
    // st_dev, st_ino, st_blksize are ignored
    attribs->st_dev = 0;
    attribs->st_ino = 0;
    attribs->st_blksize = 0;

    // Owned by the person who mounted
    attribs->st_uid = getuid();
    attribs->st_gid = getgid();

    // Access times are all "now"
    clock_gettime(CLOCK_REALTIME, &attribs->st_atim);
    attribs->st_mtim = attribs->st_atim;
    attribs->st_ctim = attribs->st_atim;

    // Querying a directory
    if (strcmp(path, "/") == 0 ||
        strcmp(path, encrypt_path) == 0 ||
        strcmp(path, decrypt_path) == 0)
    {
        // I'm not checking any permissions for demo purposes
        attribs->st_mode = S_IFDIR | 0777;
        attribs->st_nlink = 2;
        attribs->st_size = 4096;
    }
    
    // Querying a file
    else 
    {
        const char* pathend; const char* filename;
        split_fuse_path(&pathend, &filename, path);
        file_data_t* entry = NULL;

        if (strncmp(path, encrypt_path, pathend - path) == 0 ||
            strncmp(path, decrypt_path, pathend - path) == 0)
        {
            if ((entry = lookup(filename)))
            {
                // I'm not checking any permissions for demo purposes
                attribs->st_mode = S_IFREG | 0666;
                attribs->st_nlink = 1;
                attribs->st_size = entry->data_len;
            }
            else return -ENOENT;
        }
        else return -ENOENT;
    }

    return 0;
}

// Read the contents of a directory
static int orp_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset,
        struct fuse_file_info* fi)
{
    // Every directory should have . and ..
    filler(buf, ".", NULL, 0);
    filler(buf, "..", NULL, 0);

    // If we're at the root, add the ENCRYPT/ and DECRYPT/ folders
    if (strcmp(path, "/") == 0)
    {
        filler(buf, encrypt_path + 1, NULL, 0);
        filler(buf, decrypt_path + 1, NULL, 0);
    }

    // If we're inside ENCRYPT/ or DECRYPT/, return the contents of the directory table
    else if (strcmp(path, encrypt_path) == 0 ||
             strcmp(path, decrypt_path) == 0)
    {
        file_data_t* iter = dir_table_begin();
        while (iter != dir_table_end())
        {
            filler(buf, iter->filename, NULL, 0);
            iter = dir_table_next(iter);
        }
    }
    else return -EBADF;

    return 0;
}

// Create a new file
static int orp_create(const char* path, mode_t mode, struct fuse_file_info* fi)
{
    const char* pathend; const char* filename;
    split_fuse_path(&pathend, &filename, path);

    // If in ENCRYPT/ or DECRYPT/, insert the file into the directory table,
    // otherwise deny access
    if (strncmp(path, encrypt_path, pathend - path) == 0 ||
        strncmp(path, decrypt_path, pathend - path) == 0)
        insert(filename);
    else return -EACCES;

    return 0;
}

// Read the contents of a file; return the number of bytes read, or error
static int orp_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
    // First see if the file exists
    const char* pathend; const char* filename;
    split_fuse_path(&pathend, &filename, path);
    file_data_t* entry = lookup(filename);
    if (!entry) return -ENOENT;

    // Do we want to read the encrypted or unencrypted version?
    uint8_t* src = NULL;
    if (strncmp(path, encrypt_path, pathend - path) == 0)
        src = entry->plaintext;
    else if (strncmp(path, decrypt_path, pathend - path) == 0)
        src = entry->ciphertext;
    else return -EACCES;

    // Copy the data
    int realsize = 0;
    if (src)
    {
        // If we read past EOF, only read as many bytes as we can
        realsize = (offset + size > entry->data_len) ? entry->data_len - offset : size;
        memcpy(buf, src + offset, realsize);
    }

    return realsize;
}

// Write to a file; return the number of bytes written, or error
static int orp_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
    // Make sure the file exists
    const char* pathend; const char* filename;
    split_fuse_path(&pathend, &filename, path);

    file_data_t* entry = lookup(filename);
    if (!entry) return -ENOENT;

    // If we're trying to write past the end of the file, need to realloc memory; the memory for the
    // files is stored in ORP_BLOCK_SIZE chunks
    if (offset + size > entry->data_len)
    {
        // Compute the new file size: we want to write to a maximum length of offset+size,
        // so need to make sure we have at least that many blocks available
        size_t next_block_size = ((offset + size) / ORP_BLOCK_SIZE) * ORP_BLOCK_SIZE;

        // If we happen to write exactly on a block boundary, do nothing; otherwise we
        // need one extra block to store the residual data
        if ((offset + size) % ORP_BLOCK_SIZE != 0)
            next_block_size += ORP_BLOCK_SIZE;

        // Do the actual memory init
        entry->plaintext = realloc(entry->plaintext, next_block_size);
        entry->ciphertext = realloc(entry->ciphertext, next_block_size);
        memset(entry->plaintext + entry->data_len, 0, next_block_size - entry->data_len);
        memset(entry->ciphertext + entry->data_len, 0, next_block_size - entry->data_len);
        entry->data_len = next_block_size;
    }

    // buf1 is the file we wrote to, and buf2 is the computed file.  I.e., if we write
    // a file to ENCRYPT/, buf1 is the plaintext buffer and buf2 is the ciphertext buffer.
    // (And vice versa if we write a file to DECRYPT/)
    uint8_t* buf1; uint8_t* buf2; enum XtsCommand cmd;
    if (strncmp(path, encrypt_path, pathend - path) == 0)
        { buf1 = entry->plaintext; buf2 = entry->ciphertext; cmd = XTS_ENCRYPT; }
    else if (strncmp(path, decrypt_path, pathend - path) == 0)
        { buf1 = entry->ciphertext; buf2 = entry->plaintext; cmd = XTS_DECRYPT; }
    else return -EACCES;

    // Just copy the write to buf1
    memcpy(buf1 + offset, buf, size);

    // Do the encryption/decryption operation for buf2; we only update the changed blocks here,
    // so compute the start and end blocks (and offset by one if we're not exactly on a block
    // boundary)
    int block = offset / ORP_BLOCK_SIZE;
    int endblock = (offset + size) / ORP_BLOCK_SIZE;
    if ((offset + size) % ORP_BLOCK_SIZE != 0) ++endblock;

    // Encrypt or decrypt each block
    size_t remainder = size;
    while (block < endblock)
    {
        // Structure the data to be encrypted/decrypted
        uint8_t tmp[ORP_DATA_LEN]; uint32_t pos = 0;
        memset(tmp, 0, ORP_DATA_LEN);
        XtsCommand_serialize(tmp, ORP_DATA_LEN, &pos, cmd);
        uint64_serialize(tmp, ORP_DATA_LEN, &pos, block);

        int block_start = block * ORP_BLOCK_SIZE;
        size_t real_size = remainder > ORP_BLOCK_SIZE ? ORP_BLOCK_SIZE : remainder;
        memcpy(tmp + pos, buf1 + block_start, real_size);

        // Do the encryption/decryption
        int err = orp_dev_write(session_id, tmp);
        if (err != 0) 
            return -EIO;
        
        // Read the newly-en/de-crypted data from the device
        uint16_t ret_session_id;
        memset(tmp, 0, ORP_DATA_LEN);
        err = orp_dev_read(&ret_session_id, tmp);

        // Parse the response
        enum XtsResponse response; pos = 0;
        XtsResponse_deserialize(tmp, ORP_DATA_LEN, &pos, &response);
        if (err != 0 || ret_session_id != session_id || response != XTS_OK) 
            return -EIO;
        memcpy(buf2 + block_start, tmp + pos, ORP_BLOCK_SIZE);

        ++block; remainder -= ORP_BLOCK_SIZE;
    }

    return size;
}

// Rename a file
static int orp_rename(const char* old, const char* new)
{
    const char* oldpathend; const char* oldfile;
    const char* newpathend; const char* newfile;
    split_fuse_path(&oldpathend, &oldfile, old);
    split_fuse_path(&newpathend, &newfile, new);

    // If we rename, need to make sure it's a file in either 
    // ENCRYPT/ or DECRYPT/, and we can't move it from one directory
    // to the other -- the practical consequence of this is that
    // it will delete and recreate the file if you try to do one of
    // the above operations
    if ((strncmp(old, encrypt_path, oldpathend - old) == 0 ||
         strncmp(old, decrypt_path, oldpathend - old) == 0))
    {
        if (strncmp(old, new, oldpathend - old) != 0) return -EXDEV;
        if (strlen(oldfile) == 0 || strlen(newfile) == 0) return -EACCES;

        file_data_t* entry = lookup(oldfile);
        if (entry)
        {
            int status = rekey(entry, newfile);
            if (status < 0) return -EACCES;
        }
        else return -ENOENT;
    }
    else return -EACCES;

    return 0;
}

// Delete a file
static int orp_unlink(const char* path)
{
    const char* pathend; const char* filename;
    split_fuse_path(&pathend, &filename, path);

    if (strncmp(path, encrypt_path, pathend - path) == 0 ||
        strncmp(path, decrypt_path, pathend - path) == 0) 
    {
        // Just remove it from the directory table
        file_data_t* entry = lookup(filename);
        if (entry) erase(entry);
        else return -ENOENT;
    }
    else return -EACCES;

    return 0;
}

// These functions have to be implemented for FUSE to work, but I don't need them to do anything
static void* orp_init(struct fuse_conn_info* conn) { return NULL; }
static int orp_truncate(const char* path, off_t offset) { return 0; }
static int orp_utimens(const char* path, const struct timespec tv[2]) { return 0; }
static int orp_chmod(const char* path, mode_t mode) { return 0; }
static int orp_chown(const char* path, uid_t uid, gid_t gid) { return 0; }

static struct fuse_operations orp_oper = 
{
    .getattr = orp_getattr,
    .readdir = orp_readdir,
    .create = orp_create,
    .read = orp_read,
    .write = orp_write,
    .rename = orp_rename,
    .unlink = orp_unlink,

    .init = orp_init,
    .truncate = orp_truncate,
    .utimens = orp_utimens,
    .chmod = orp_chmod,
    .chown = orp_chown,
};

static void usage(const char* progname)
{
    printf("%s [fuse-opts] <encryption key> <orp device mount point> <orpfs mount point>\n", progname);
}

int main(int argc, char* argv[])
{
    int fuse_stat;

    if (argc < 4) 
    {
        usage(argv[0]);
        return -1;
    }

    if (getuid() == 0 || geteuid() == 0)
    {
        fprintf(stderr, "ORPFS cannot be run as root\n");
        return -1;
    }

    // Last two args are the ORP device mount point and 
    // the mount point for the FUSE layer
    session_id = orp_dev_connect(argv[argc-2], argv[argc-3]);
    if (session_id < 0)
    {
        fprintf(stderr, "Could not start ORP device\n");
        return -1;
    }
    argv[argc-3] = argv[argc-1];
    argv[argc-2] = NULL;
    argv[argc-1] = NULL;
    argc -= 2;

    init_dir_table();
    fuse_stat = fuse_main(argc, argv, &orp_oper, NULL);

    return fuse_stat;
}


