/** @file taskmem.h */
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
#ifndef _MSEL_TASKMEM_H_
#define _MSEL_TASKMEM_H_

#include <stdlib.h>

/** @brief called at boot time to initialize this module */
void msel_init_taskmem();

/** @brief Initialize the RAM for a newly created task, by task number. */
void taskmem_init(size_t);

/** @brief Location of end of stack section for given task */
void* taskmem_stack_top(size_t);

/** @brief Location of start of stack section for given task */
void* taskmem_stack_bottom(size_t);

/** @brief Size of the stack for given task */
size_t taskmem_stack_size(size_t);

/** @brief Location of the start of heap for given task */
void* taskmem_heap_start(size_t);

/** @brief Location of the end of heap for a given task */
void* taskmem_heap_end(size_t);

/** @brief Size of the heap area for a given task (this excludes area for the mspace val) */
size_t taskmem_heap_size(size_t);

#endif

