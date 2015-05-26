/** @file stdc.h
    
    Minimal Standard-C like functionality

*/
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
#ifndef _MSEL_INC_STDC_H_
#define _MSEL_INT_STDC_H_

#include <stdlib.h> /* for size_t only */
#include <stdint.h>

/** @brief msel_memset sets a given region of memory to a given value

    @param dst Pointer to beginning of the data
    @param val Value to write to each address
    @param sz  Number of bytes to write

    @return returns dst
*/
void*  msel_memset(void *dst, int val, size_t sz);

/** @brief msel_memcpy will copy all data from one region of memory to
    another
    
    @param dst where data should be written
    @param src where data should be read from
    @param sz  how many bytes to copy

    @return returns dst
*/
void*  msel_memcpy(void *dst, const void *src, size_t sz);

/** @brief msel_memcmp will compare two memory regions, byte for byte,
    and provide a return value indicating if a>b, a<b or a==b.

    @param a pointer to first region
    @param b pointer to second region
    @param sz maximum about of bytes to compare

    @return Returns -1 if a<b, 0 if a==b or 1 if b<a
*/
int    msel_memcmp(const void *a, const void *b, size_t sz);

/** @brief Counts the number of characters in a NULL-terminated
    C-style string.

    @param src

    @return Returns the number of non-NULL characters occuring before
    the first NULL character.
*/
size_t msel_strlen(const char *src);

/** @brief Compare two C-strings

    @param a first string
    @param b second string

    @return -1 if a<b, 0 if a==b, 1 if b<a
 */
int    msel_strcmp(const char *a, const char *b);

/** @brief Copy a string somewhere else 
    
    @param dst Location where copied string is to be stored
    @param src Location from whence the copied bytes come

    @return returns number of bytes copied
*/
char*  msel_strcpy(char* dst, const char *src);

/** @brief msel_systicks is updated on every cycle of the system timer
    and may be used as some indication of the passage of time, but not
    necessarily wall time. */
extern uint64_t msel_systicks;

#endif
