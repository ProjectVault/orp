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
#include <stdlib.h> /* for size_t only */
#include <stdint.h>

#include <msel/stdc.h>

void* msel_memset(void *dst, int val, size_t sz)
{
    uint8_t* dst_c = dst;
    size_t   idx;

    for(idx=0; idx<sz; idx++)
        dst_c[idx] = (uint8_t)val;
    
    return dst;
}

void* msel_memcpy(void *dst, const void *src, size_t sz)
{
    const uint8_t* src_c = src;
    uint8_t* dst_c = dst;
    size_t   idx;

    for(idx=0; idx<sz; idx++)
        dst_c[idx] = src_c[idx];
    return NULL;
}

int msel_memcmp(const void *a, const void *b, size_t sz)
{
    const uint8_t* a_c = a;
    const uint8_t* b_c = b;
    size_t idx = 0;
    while(idx < sz - 1 && a_c[idx] == b_c[idx])
        idx++;

    if(a_c[idx] < b_c[idx])
        return -1;
    if(b_c[idx] < a_c[idx])
        return 1;
    return 0;
}

size_t msel_strlen(const char *src)
{
    size_t ctr=0;

    while(src[ctr]) ctr++;
    
    return ctr;
}

int msel_strcmp(const char *a, const char *b)
{
    size_t idx = 0;
    while(a[idx] && a[idx] == b[idx])
        idx++;

    if(a[idx] < b[idx])
        return -1;
    if(b[idx] < a[idx])
        return 1;
    return 0;
}

char* msel_strcpy(char* dst, const char *src)
{
    size_t idx = 0;

    while(src[idx])
    {
        dst[idx] = src[idx];
        idx++;
    }
    
    return dst;
}

