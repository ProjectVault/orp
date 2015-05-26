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


// dir_table.c

#include "dir_table.h"
#include <string.h>
#include <stdlib.h>

file_data_t* table[DIR_TABLE_SIZE];

// Hash function from http://www.cse.yorku.ca/~oz/hash.html
static unsigned long djb2(const char* str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
        hash = ((hash << 5) + hash) ^ c;

    return hash % DIR_TABLE_SIZE;
}

// Look up an item in the hash table
file_data_t* lookup(const char* filename)
{
    unsigned long hash = djb2(filename);
    file_data_t* entry = table[hash];

    // Loop through the linked list at the hash value until
    // we find a matching entry
    while (entry && strcmp(entry->filename, filename) != 0)
        entry = entry->next;

    return entry;
}

// Insert an item in the hash table
file_data_t* insert(const char* filename)
{
    unsigned long hash = djb2(filename);
    file_data_t* entry = table[hash];
    file_data_t* prev = NULL;

    // Traverse to the end of the linked list to insert
    while (entry)
    {
        // Don't allow duplicate entries
        if (strcmp(entry->filename, filename) == 0)
            return entry;

        prev = entry;
        entry = entry->next;
    }

    // Create a new table entry, filled with 0
    entry = (file_data_t*)calloc(1, sizeof(file_data_t));

    // Every entry must have a filename
    entry->filename = strdup(filename);

    // Fix up our previous/next links
    entry->prev = prev;
    if (prev) prev->next = entry;
    else table[hash] = entry;

    return entry;
}

// Change an entry's key -- fail if the new key already exists 
int rekey(file_data_t* entry, const char* new)
{
    unsigned long hash = djb2(new);
    file_data_t* curr = table[hash];
    file_data_t* prev = NULL;

    // Find the new location for the entry
    while (curr)
    {
        // Make sure there's no duplicates
        if (strcmp(curr->filename, new) == 0) return -1;

        prev = curr;
        curr = curr->next;
    }

    // Remove the entry from its old position in the table
    if (entry->prev) entry->prev->next = entry->next;
    else table[djb2(entry->filename)] = entry->next;

    // Remove the old key
    free(entry->filename);

    // Set up the entry with new key and new location
    entry->filename = strdup(new);
    if (prev) prev->next = entry;
    else table[hash] = entry;

    return 0;
}

void erase(file_data_t* entry)
{
    if (!entry) return;
    
    // Free data if it hasn't been done already
    if (entry->plaintext) 
        { free(entry->plaintext); entry->plaintext = NULL; }
    if (entry->ciphertext) 
        { free(entry->ciphertext); entry->ciphertext = NULL; }
    
    // Relink the list of remaining elements
    if (entry->prev) entry->prev->next = entry->next;
    else table[djb2(entry->filename)] = entry->next;
    
    // All entries must contain a filename
    // Free this last so the hash calculation above still works
    free(entry->filename);

    free(entry);
}

// Iterator functions: return a pointer to the first element of the table
file_data_t* dir_table_begin()
{
    unsigned i;
    for (i = 0; i < DIR_TABLE_SIZE; ++i)
        if (table[i]) return table[i];
    return dir_table_end(table);
}

// Iterator functions: return a pointer to the end of the table
file_data_t* dir_table_end()
{
    return NULL;
}

// Iterator functions: advance the iterator
file_data_t* dir_table_next(file_data_t* current)
{
    if (current)
    {
        // If we're at the end of a linked list, search the
        // hash table for the next element
        if (!current->next)
        {
            unsigned long hash = djb2(current->filename) + 1;
            for (hash; hash < DIR_TABLE_SIZE; ++hash)
                if (table[hash]) return table[hash];
            return dir_table_end(table);
        }

        // Otherwise, return the next thing in the list
        else return current->next;
    }
    else return dir_table_end(table);
}

void init_dir_table()
{
    memset(table, 0, DIR_TABLE_SIZE * sizeof(file_data_t*));
}
