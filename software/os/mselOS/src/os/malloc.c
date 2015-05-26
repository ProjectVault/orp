/*
 * malloc.c
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
#include <stdio.h>
#include <stdint.h>
#include "msel/stdc.h"
#include "msel/malloc.h"
#include "malloc_int.h"
#include "arch.h"

/* microSEL Specifics */

/* User an arch-specific method to locate the heap for the currently executing task */
inline void* malloc_get_task_heap()
{
    return arch_get_task_heap();
}

inline void* msel_malloc(uint32_t sz)
{
    return heap_malloc(malloc_get_task_heap(),sz);
}

inline void* msel_realloc(void *ptr, uint32_t sz)
{
    return heap_realloc(malloc_get_task_heap(),ptr,sz);
}

inline void msel_free(void* ptr)
{
    heap_free(malloc_get_task_heap(),ptr);
}

static inline uint32_t get_bucket_idx(uint32_t size)
{
	uint32_t ret;

	ret = size >> BUCKET_SHIFT;
	if(ret >= BUCKET_COUNT)
		ret = BUCKET_COUNT - 1;
	return ret;
}

static inline HEADER *get_header_from_ptr(void *ptr)
{
	uint8_t *p;

	p = (uint8_t *) ptr;
	p -= sizeof(HEADER);
	return ((HEADER *) p);
}

static inline FOOTER *get_footer_from_header(HEADER *hdr)
{
	uint8_t *ptr;

	ptr = (uint8_t *) hdr;
	ptr += sizeof(HEADER);
	ptr += CHUNK_SIZE(hdr);
	return ((FOOTER *) ptr);
}

static inline HEADER *get_header_from_footer(FOOTER *ftr)
{
	uint8_t *ptr;

	ptr = (uint8_t *) ftr;
	ptr -= sizeof(HEADER);
	ptr -= CHUNK_SIZE(ftr);
	return ((HEADER *) ptr);
}

static inline FOOTER *get_prev_footer(HEADER *hdr)
{
	uint8_t *ptr;

	ptr = (uint8_t *) hdr;
	ptr -= sizeof(FOOTER);
	return ((FOOTER *) ptr);
}

static inline HEADER *get_next_chunk(HEADER *hdr)
{
	uint8_t *ptr;

	ptr = (uint8_t *) hdr;
	ptr += sizeof(HEADER);
	ptr += CHUNK_SIZE(hdr);
	ptr += sizeof(FOOTER);
	return ((HEADER *) ptr);
}

static void freelist_unlink(HEAP *heap, HEADER *hdr)
{
	uint32_t bucket_idx;

	if(hdr->s.prev)
		hdr->s.prev->s.next = hdr->s.next;
	else
	{
		bucket_idx = get_bucket_idx(hdr->s.size);
		heap->free_list[bucket_idx] = hdr->s.next;
	}
	if(hdr->s.next)
		hdr->s.next->s.prev = hdr->s.prev;

#ifdef HEAP_DEBUG
	{
		FOOTER *ftr;
		ftr = get_footer_from_header(hdr);
		HEAP_ASSERT(hdr->s.size == ftr->s.size);
	}
#endif
	return;
}

static void chunk_split(HEAP *heap, HEADER *hdr, uint32_t size)
{
	uint32_t chunk_size, rem;
	uint8_t *ptr;
	HEADER *chk;
	FOOTER *ftr;

	chunk_size = CHUNK_SIZE(hdr);

	rem = chunk_size - size;
	if(rem < sizeof(HEADER) + sizeof(FOOTER) + ALIGN_SIZE)
	{
		ftr = get_footer_from_header(hdr);
		hdr->s.size &= ~CHUNK_FREE;
		ftr->s.size = hdr->s.size;
		return;
	}
	hdr->s.size = size;
	chk = get_next_chunk(hdr);
	ftr = get_footer_from_header(hdr);
	ftr->s.size = hdr->s.size;

	chk->s.size = rem - (sizeof(HEADER) + sizeof(FOOTER));
	ftr = get_footer_from_header(chk);
	ftr->s.size = chk->s.size;
	ptr = (uint8_t *) chk;
	ptr += sizeof(HEADER);
	heap_free(heap, ptr);
	return;
}

void heap_init(void *base_ptr, void *end_ptr)
{
	int i;
	uint8_t *ptr, *eptr;
	uint32_t bucket_idx;
	HEAP *heap;
	HEADER *hdr;
	FOOTER *ftr;


	ptr = (uint8_t *) base_ptr;
	eptr = (uint8_t *) end_ptr;

	HEAP_ASSERT(end_ptr > base_ptr);
	HEAP_ASSERT(eptr - ptr > HEAP_SIZE);
	heap = (HEAP *) base_ptr;
	for(i = 0; i < BUCKET_COUNT; i++)
		heap->free_list[i] = NULL;

	ptr += HEAP_SIZE;

	heap->start = ptr;
	heap->end = eptr;

	hdr = (HEADER *) ptr;

	ptr += sizeof(HEADER);
	eptr -= sizeof(FOOTER);

	HEAP_ASSERT(eptr - ptr >= ALIGN_SIZE);
	ftr = (FOOTER *) eptr;
	ftr->s.size = hdr->s.size = (eptr - ptr) | CHUNK_FREE;
	hdr->s.next = hdr->s.prev = NULL;

	bucket_idx = get_bucket_idx(hdr->s.size);
	heap->free_list[bucket_idx] = hdr;
	return;
}

#ifdef HEAP_DEBUG
void heap_print(void *base_ptr)
{
	int i;
	HEAP *heap;
	HEADER *hdr;
	FOOTER *ftr;

	heap = (HEAP *) base_ptr;
	printf("heap: %p\n", heap);
	for(i = 0; i < BUCKET_COUNT; i++)
	{
		printf("\tfree_list[%u]: %p\n", i, heap->free_list[i]);
		for(hdr = heap->free_list[i]; hdr; hdr = hdr->s.next)
		{
			printf("\t\tchk: %p prev: %p next: %p\n", hdr, hdr->s.prev, hdr->s.next);
		}
	}

	printf("heap:");
	for(hdr = heap->start; hdr < (HEADER *) heap->end; hdr = get_next_chunk(hdr))
	{
		printf("\theader: %p size: %u flags: %u\n", hdr, CHUNK_SIZE(hdr), CHUNK_FLAGS(hdr));
		ftr = get_footer_from_header(hdr);
		printf("\tfooter: %p size: %u flags: %u\n", ftr, CHUNK_SIZE(ftr), CHUNK_FLAGS(ftr));
		HEAP_ASSERT(hdr->s.size == ftr->s.size);
	}
	return;
}

int valid_fail(void *base_ptr, void **ptrs, uint32_t n_ptrs, void *fail_ptr, uint32_t fail_alloc)
{
	uint32_t i, bucket_idx, flags, size;
	HEAP *heap;
	HEADER *hdr, *chk;

	heap = (HEAP *) base_ptr;
	for(i = 0; i < BUCKET_COUNT; i++)
	{
		for(hdr = heap->free_list[i]; hdr; hdr = hdr->s.next)
		{
			if(hdr->s.prev)
				HEAP_ASSERT(hdr == hdr->s.prev->s.next);
			else
				HEAP_ASSERT(hdr == heap->free_list[i]);
			if(hdr->s.next)
				HEAP_ASSERT(hdr == hdr->s.next->s.prev);

			bucket_idx = get_bucket_idx(hdr->s.size);
			HEAP_ASSERT(bucket_idx == i);
		}
	}
	for(hdr = heap->start; hdr < (HEADER *) heap->end; hdr = get_next_chunk(hdr))
	{
		flags = CHUNK_FLAGS(hdr);
		if(flags & CHUNK_FREE)
		{
			/* chunk hdr is marked as free, make sure it is in the the appropriate free list */
			bucket_idx = get_bucket_idx(hdr->s.size);
			for(chk = heap->free_list[bucket_idx]; chk; chk = chk->s.next)
			{
				if(chk == hdr)
					break;
			}
			HEAP_ASSERT(chk != NULL);
		}
		else
		{
			/* chunk hdr is marked as in use, make sure it is in the list of pointers */
			for(i = 0; i < n_ptrs; i++)
			{
				if(!ptrs[i])
					continue;
				chk = get_header_from_ptr(ptrs[i]);
				if(chk == hdr)
					break;
			}
			HEAP_ASSERT(i < n_ptrs);
		}
	}

	/* make sure all pointers are marked as in use */
	for(i = 0; i < n_ptrs; i++)
	{
		if(!ptrs[i])
			continue;
		hdr = get_header_from_ptr(ptrs[i]);
		flags = CHUNK_FLAGS(hdr);
		HEAP_ASSERT((flags & CHUNK_FREE) == 0);

		for(chk = heap->start; chk < (HEADER *) heap->end; chk = get_next_chunk(chk))
		{
			if(chk == hdr)
				break;
		}
		HEAP_ASSERT(chk < (HEADER *) heap->end);
	}

	if(fail_ptr)
	{
		/* check why realloc failed */
		hdr = get_header_from_ptr(fail_ptr);

		chk = get_next_chunk(hdr);
		flags = CHUNK_FLAGS(chk);
		if(flags & CHUNK_FREE)
		{
			size = CHUNK_SIZE(hdr) + sizeof(HEADER) + sizeof(FOOTER) + CHUNK_SIZE(chk);
			HEAP_ASSERT(size < fail_alloc);
		}
	}

	/* check why malloc failed */
	bucket_idx = get_bucket_idx(fail_alloc);
	for(hdr = heap->free_list[bucket_idx]; hdr; hdr = hdr->s.next)
	{
		HEAP_ASSERT(CHUNK_SIZE(hdr) < fail_alloc);
	}
	return 0;
}
#endif /* HEAP_DEBUG */

void *heap_malloc(void *base_ptr, uint32_t size)
{
	uint32_t bucket_idx;
	uint8_t *ptr;
	HEAP *heap;
	HEADER *hdr;

	size = (size + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1);
	heap = (HEAP *) base_ptr;
	for(bucket_idx = get_bucket_idx(size); bucket_idx < BUCKET_COUNT; bucket_idx++)
	{
		for(hdr = heap->free_list[bucket_idx]; hdr; hdr = hdr->s.next)
		{
			if(CHUNK_SIZE(hdr) >= size)
			{
				freelist_unlink(heap, hdr);
				chunk_split(heap, hdr, size);
				ptr = (uint8_t *) hdr;
				ptr += sizeof(HEADER);
				return ((void *) ptr);
			}
		}
	}
	return NULL;
}

void *heap_realloc(void *base_ptr, void *ptr, uint32_t size)
{
	uint8_t *p;
	HEAP *heap;
	HEADER *hdr, *chk_hdr;
	FOOTER *ftr;

	size = (size + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1);
	heap = (HEAP *) base_ptr;
	p = (uint8_t *) ptr;
	p -= sizeof(HEADER);
	hdr = (HEADER *) p;

#ifdef HEAP_DEBUG
	HEAP_ASSERT((hdr->s.size & CHUNK_FREE) == 0);
	ftr = get_footer_from_header(hdr);
	HEAP_ASSERT(hdr->s.size == ftr->s.size);
#endif

	/* realloc requesting more memory than current chunk provides */
	if(CHUNK_SIZE(hdr) < size)
	{
		chk_hdr = get_next_chunk(hdr);
		/* if the next chunk is free, merge it */
		if((chk_hdr < (HEADER *) heap->end) && (CHUNK_FLAGS(chk_hdr) & CHUNK_FREE))
		{
			freelist_unlink(heap, chk_hdr);
			hdr->s.size += CHUNK_SIZE(chk_hdr) + sizeof(HEADER) + sizeof(FOOTER);
			ftr = get_footer_from_header(hdr);
			ftr->s.size = hdr->s.size;
		}

		/* if the current chunk is still too small, allocate new memory
		 * and copy to it
		 */
		if(CHUNK_SIZE(hdr) < size)
		{
			p = heap_malloc(base_ptr, size);
			if(!p)
				return NULL;
			msel_memcpy(p, ptr, size);
			heap_free(base_ptr, ptr);
			return ((void *) p);
		}

		/* falls through if CHUNK_SIZE(hdr) >= size after merger */
	}

	chunk_split(heap, hdr, size);
	return ptr;
}

void heap_free(void *base_ptr, void *ptr)
{
	uint32_t bucket_idx;
	HEAP *heap;
	HEADER *hdr, *chk_hdr;
	FOOTER *ftr, *chk_ftr;

	heap = (HEAP *) base_ptr;
	hdr = get_header_from_ptr(ptr);
	ftr = get_footer_from_header(hdr);

	HEAP_ASSERT(hdr >= (HEADER *) heap->start);
	HEAP_ASSERT(ftr < (FOOTER *) heap->end);
	HEAP_ASSERT((hdr->s.size & CHUNK_FREE) == 0);
	HEAP_ASSERT(hdr->s.size == ftr->s.size);

	hdr->s.size |= CHUNK_FREE;
	ftr->s.size = hdr->s.size;

	/* reverse merge */
	if(hdr > (HEADER *) heap->start)
	{
		chk_ftr = get_prev_footer(hdr);
		if(CHUNK_FLAGS(chk_ftr) & CHUNK_FREE)
		{
			chk_hdr = get_header_from_footer(chk_ftr);
			freelist_unlink(heap, chk_hdr);
			chk_hdr->s.size += CHUNK_SIZE(hdr) + sizeof(HEADER) + sizeof(FOOTER);
			hdr = chk_hdr;
			ftr->s.size = hdr->s.size;
		}
	}

	/* forward merge */
	chk_hdr = get_next_chunk(hdr);
	if((chk_hdr < (HEADER *) heap->end) && (CHUNK_FLAGS(chk_hdr) & CHUNK_FREE))
	{
		chk_ftr = get_footer_from_header(chk_hdr);
		freelist_unlink(heap, chk_hdr);
		hdr->s.size += CHUNK_SIZE(chk_hdr) + sizeof(HEADER) + sizeof(FOOTER);
		ftr = chk_ftr;
		ftr->s.size = hdr->s.size;
	}

	/* link into free list */
	bucket_idx = get_bucket_idx(hdr->s.size);
	hdr->s.prev = NULL;
	hdr->s.next = heap->free_list[bucket_idx];
	if(hdr->s.next)
		hdr->s.next->s.prev = hdr;
	heap->free_list[bucket_idx] = hdr;
	return;
}
