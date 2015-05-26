/*
 * malloc_int.h
 *
 *  Created on: Dec 17, 2014
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
#ifndef MALLOC_INT_H_
#define MALLOC_INT_H_

#ifdef HEAP_DEBUG
#include <assert.h>
#define HEAP_ASSERT(x) assert(x)
#else
#define HEAP_ASSERT(x) {}
#endif

#define BUCKET_COUNT 4
#define BUCKET_SHIFT 7
#define CHUNK_FREE	1

typedef union _HEADER {
	struct {
		uint32_t size;
		union _HEADER *next;
		union _HEADER *prev;
	} s;
	uint8_t x[ALIGN_SIZE];
} HEADER;

#define CHUNK_SIZE(x) ((x)->s.size & ~(ALIGN_SIZE - 1))
#define CHUNK_FLAGS(x) ((x)->s.size & (ALIGN_SIZE - 1))

typedef union _FOOTER {
	struct {
		uint32_t size;
	} s;
	uint8_t x[ALIGN_SIZE];
} FOOTER;

typedef struct _HEAP {
	HEADER *free_list[BUCKET_COUNT];
	void *start, *end; // start of heap, end of heap
} HEAP;

#define HEAP_SIZE ((sizeof(HEAP) + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1))

#endif /* MALLOC_INT_H_ */
