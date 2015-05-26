/** @file malloc.h
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

#ifndef MALLOC_H_
#define MALLOC_H_

#include <stdint.h>

#define ALIGN_SIZE 16 // in bytes, must be a power of 2

/* @brief This will attempt to allocate 'sz' bytes on the current
   thread's heap and return a pointer to the new allocation.

   @param sz Amount of data (in bytes) to allocate

   @return If successfull a non-zero pointer will be returned. A NULL
   return value indicates a failure to allocate.
*/
void* msel_malloc(uint32_t sz);

/* @brief This will take an existing allocation on a thread's heap and
   resize it to the given parameters, copying data if needed and
   returning a pointer to the resulting allocation.

   @param ptr Pointer to reallocate. This pointer must have been
   derived from a previous call to msel_malloc or msel_realloc.

   @param sz The size of the desired reallocation

   @return Returns NULL of failure and a pointer to the new allocation
   on success.
*/
void* msel_realloc(void *ptr, uint32_t sz);

/** @brief Take a pointer to an existing allocation on a thread's
   heap, mark it as unused and return it to the pool of available
   space on the heap.
   
   @param ptr Pointer to free. This pointer must have been
   derived from a previous call to msel_malloc or msel_realloc.
*/
void  msel_free(void* ptr);

/** @brief This returns a pointer to the start of the heap for the
    current thread. Use with caution. It is likely only useful if a
    thread needs to forgo the use of normal malloc/free calls and
    manage it's own memory.

    @return Pointer to start of heap
 */
void* malloc_get_task_heap();

/* Bare generic heap implementation API */

/** @brief Create a new heap that can be used for dynamic memory
    allocations given a starting address and an ending address. This
    is useful if a thread needs to manage it's own memory area
    directly and not use the default malloc/free API.

    @param base_ptr Starting address of new heap
    @param end_ptr Ending address of new heap
 */
extern void heap_init(void *base_ptr, void *end_ptr);

/** @brief Allocate the given amount of bytes on the given heap
    structure. 

    @param base_ptr Which heap structure to use for the
    allocation. "heap_init" must have been previously invoked on this
    pointer.

    @param size How many bytes to allocate.

    @return Returns NULL of failure and a pointer to the new
    allocation on success.
 */
extern void *heap_malloc(void *base_ptr, uint32_t size);

/* @brief This will take an existing allocation on a heap and resize
   it to the given parameters, copying data if needed and returning a
   pointer to the resulting allocation.

   @param base_ptr Which heap structure to use for the
   re-allocation. "heap_init" must have been previously invoked on
   this pointer.

   @param ptr Pointer to reallocate. This pointer must have been
   derived from a previous call to heap_malloc or heap_realloc.

   @param sz The size of the desired reallocation

   @return Returns NULL of failure and a pointer to the new allocation
   on success.
*/
extern void *heap_realloc(void *base_ptr, void *ptr, uint32_t size);

/** @brief Take a pointer to an existing allocation on a the given
   heap heap, mark it as unused and return it to the pool of available
   space on the heap.
   
   @param base_ptr Which heap structure to use for the
   re-allocation. "heap_init" must have been previously invoked on
   this pointer.

   @param ptr Pointer to free. This pointer must have been
   derived from a previous call to heap_malloc or heap_realloc.
*/
extern void heap_free(void *base_ptr, void *ptr);

#ifdef HEAP_DEBUG
extern void heap_print(void *base_ptr);
extern int valid_fail(void *base_ptr, void **ptrs, uint32_t n_ptrs, void *fail_ptr, uint32_t fail_alloc);
#endif

#endif /* MALLOC_H_ */
