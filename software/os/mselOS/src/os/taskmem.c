/* @file taskmem.c

   This is a custom allocator for use by task.c to break of chunks of
   ram for task's heap and stack that is backed by a "master" mspace
   encompasing all free RAM. Only task create and task delete may
   utilize this mspace. In order to do dynamic allocations, each task
   must create it's own mspace within a slice that it has been
   allocated from here. Interrupts may not utilize dynamic memory at
   all because interrupt pre-emption would cause re-entrancy issues in
   malloc.

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

#include <stdint.h>
#include "msel/stdc.h"
#include "msel/malloc.h"
#include "system.h"
#include "taskmem.h"
#include "util.h"
#include "arch.h"

/* Pull in info about the static tasks memory blocks from the linker script */
extern uint32_t tasks_start;
extern uint32_t tasks_size;
extern uint32_t tasks_stack_size;
extern uint32_t tasks_heap_size;

void msel_init_taskmem() 
{
    /* Task 0 is the only task setup with ram by default */
    taskmem_init(0);
}

void taskmem_init(size_t tasknum)
{
    /* Zero all task-owned mem (DO NOT reset task 0 stack, for it is the current stack) */
    if(tasknum != 0)
        msel_memset(taskmem_stack_bottom(tasknum), 0, ((size_t)&tasks_size));

    /* Pre-initialize heap for task */
    heap_init(taskmem_heap_start(tasknum),taskmem_heap_end(tasknum));
}

inline void* taskmem_stack_top(size_t tasknum)
{
    return (void *)(((uint32_t)&tasks_start) + ((uint32_t)&tasks_size)*tasknum + ((uint32_t)&tasks_stack_size));
}

inline void* taskmem_stack_bottom(size_t tasknum)
{
    return (void *)(((uint32_t)&tasks_start) + ((uint32_t)&tasks_size)*tasknum);
}

inline size_t taskmem_stack_size(size_t tasknum)
{
    return ((size_t)&tasks_stack_size);
}

inline void* taskmem_heap_start(size_t tasknum)
{
    return taskmem_stack_top(tasknum);
}

inline void* taskmem_heap_end(size_t tasknum)
{
    return (void *)(((uint32_t)&tasks_start) + ((uint32_t)&tasks_size)*tasknum + ((uint32_t)&tasks_size));
}

inline size_t taskmem_heap_size(size_t tasknum)
{
    return (size_t)&tasks_heap_size;
}


