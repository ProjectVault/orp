/** @file include/msel/tasks.h

    Includes definitions for all publicly accessible functions related
    to task switching

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

#ifndef _INC_MSEL_TASKS_H_
#define _INC_MSEL_TASKS_H_

#include <stdlib.h>
#include <msel.h>
#include <stdint.h>

/** @brief fn pointer to a thread's entry routine */
typedef void (*msel_thread_entry)(void *arg, const size_t arg_sz);

/** @brief add a new thread to the task struct. This can only be
    called from privileged code running from the main stack. Currently
    the only way to do this is after a fresh reset before the main
    task is started.
  
    @param entry the function to execute in its own thread

    @param stack_sz amount of stack to alloccate. HW mem mgmt, if available, will be used to enforce r/w boundaries

    @param heap_sz amount of heap space dediated to this thread. HW mem mgmt, if available, will be used to enforce r/w boundaries

    @param arg an arbirary word of data to be passed to the thread on its initial invocation.

    @return On success, MSEL_OK.  Fails with `MSEL_ERESOURCE` if the
    number of tasks exceeds MSEL_TASKS_MAX. Fails with `MSEL_EINVAL`
    when thread is NULL, thread->entry is NULL, or heap/stack size are greater
    than the maximum values.
*/
msel_status msel_task_create(const msel_thread_entry, void *arg, size_t arg_sz, uint8_t* tid);

/** @brief Terminates execution of the currently running thread */
void msel_task_exit();

#endif
