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


/*
 * Stored data for encryption/decryption
 */
#ifndef _FILE_DATA_H
#define _FILE_DATA_H

#include <stdint.h>
#include <stdio.h>
#include <limits.h>

#define DIR_TABLE_SIZE 2048

// A hash table entry
typedef struct file_data_s
{
    char* filename;
    uint8_t* ciphertext;
    uint8_t* plaintext;
    unsigned data_len;

    // Implemented as a doubly-linked list for chaining
    // in the hash table.  Really the file_data and the
    // links ought to be separate, but I don't care for 
    // demo purposes
    struct file_data_s* next;
    struct file_data_s* prev;
} file_data_t;

// Hash table functions
file_data_t* lookup(const char* filename);
file_data_t* insert(const char* filename);
int rekey(file_data_t* entry, const char* new);
void erase(file_data_t* entry);

// Iterator functions
file_data_t* dir_table_begin();
file_data_t* dir_table_end();
file_data_t* dir_table_next(file_data_t* current);

void init_dir_table();

#endif // _FILE_DATA_H
