/** @file task.c

    Controls task switching and manages contexts within microSEL
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
#include <stdlib.h>

#include <msel.h>
#include <msel/malloc.h>
#include <msel/stdc.h>

#define _TASK_EXPORTS
#include "task.h"

#include "taskmem.h"
#include "system.h"
#include "syscall.h"
#include "util.h"
#include "arch.h"

#include "driver/uart.h"

/* Global variables */

/** @brief the list of all tasks... TCB */
msel_tcb msel_task_list[MSEL_TASKS_MAX];

/** @brief number of active tasks in the task list */
size_t msel_num_tasks = 0;

/** @brief the offset into msel_task_list of the currently running task */
uint8_t msel_active_task_num = 0;

/** @brief direct pointer to the current tcb structure */
msel_tcb* msel_active_task = 0;

/* Function definitions */

/** @brief initialize anything that needs to be inside the tasking
 * system before msel_start() gets called */
void msel_init_task() 
{
    msel_memset(msel_task_list, 0, sizeof(msel_task_list));

    /* initialize the main thread (thread 0) */
    msel_task_create(msel_main,NULL,0,NULL);
    msel_active_task_num = 0;
    msel_active_task = &msel_task_list[0];

    /* Architecture-specific initialization */
    arch_init_task();
}

/** @brief switch to the next available thread, if needed. This should
    be called from privileged code in an interrupt context any time
    the active thread sleeps or otherwise blocks, as well at the
    system tick interval to update system counters and force time
    sharing. Do _NOT_ call this outside of an ISR, it relies on return
    eventually doing a context restore.
    
    @return doesn't return on success. If the task list is empty, the
    return value is MSEL_EINVAL, if nothing is able to be awoken
    return MSEL_EBUSY
*/
msel_status msel_task_schedule() {
    msel_status retval = MSEL_EUNKNOWN;
    size_t tnum;
    /* Sanity checks */
    if(msel_num_tasks == 0) {
	retval = MSEL_EINVAL;
	goto end;
    }

    /* Cleanup finished tasks */
    for(tnum=0;tnum<msel_num_tasks;tnum++)
    {
        if(msel_task_is_killed(&(msel_task_list[tnum])))
        {
            char c = tnum + '0';
            /* FIXME: we really shouldn't block here for so long */
            uart_print_now("Killed task ");
            uart_write_now(&c, 1);
            uart_print_now(". REASON: ");
            uart_print_now(msel_task_list[tnum].reason);
            uart_print_now("\r\n");
            msel_task_cleanup(&(msel_task_list[tnum]));
        }
    }
    
    /* for now just blind round-robin to next non-waiting thread */
    do {
        msel_active_task_num = (msel_active_task_num + 1) % msel_num_tasks;
        msel_active_task = &msel_task_list[msel_active_task_num];
    } while(!msel_task_is_valid(msel_active_task));

    /* Make sure the memory region permissions are accurate */
    msel_task_setup_mm(msel_active_task);

    msel_set_status_leds();

    retval = MSEL_OK;
 end:
    return retval;
}

/** @brief allows for termination of an errant task */
void msel_task_force_kill(size_t tasknum, char *reason)
{
    msel_task_list[tasknum].killed = 1;
    msel_task_list[tasknum].reason = reason;
    msel_task_schedule();
}
    

/** @brief launches into thread mode with a new stack from an ISR */
void msel_task_launch_main()
{
    msel_task_setup_mm(&(msel_task_list[0]));
    arch_task_launch_main((msel_tcb*)&(msel_task_list[0]));
}

/** @brief helper method to ease task status query */
inline int msel_task_is_waiting(const msel_tcb const *tcb)
{
    return (tcb) && (tcb->wait_op != MSEL_TASK_WAIT_NONE) && (tcb->valid != 0) && !msel_task_is_killed(tcb);
}

inline int msel_task_is_killed(const msel_tcb const *tcb)
{
    return (tcb) && (tcb->killed != 0);
}

inline int msel_task_is_valid(const msel_tcb const *tcb)
{
    return (tcb) && (tcb->valid != 0) && !msel_task_is_killed(tcb) && !msel_task_is_waiting(tcb);
}

/** @brief helper method to clear a waiting state */
inline void msel_task_resume(msel_tcb* tcb)
{
    tcb->wait_op = MSEL_TASK_WAIT_NONE;
    /* don't clear state here, these will get used as arguments to the resumed system call */
}

/* Create a new task and ready for launch */
msel_status msel_task_create(const msel_thread_entry entry, void *arg, size_t arg_sz, uint8_t* tid) 
{
    msel_status        ret            = MSEL_EUNKNOWN;
    msel_tcb*          task           = NULL;
    uint8_t            tasknum;

    /* Find next available task entry */
    for(tasknum = 0; tasknum < MSEL_TASKS_MAX; tasknum++)
    {
        if(!msel_task_list[tasknum].valid)
            break;
    }

    /* sanity checks (currently this function can only be called with
     * static values, but lets validate just for fun) */
    if(tasknum == MSEL_TASKS_MAX)
    {
        ret = MSEL_ERESOURCE;
        goto cleanup;
    }
    
    if(!entry || (arg && !arg_sz) || arg_sz > taskmem_heap_size(tasknum)) {
        ret = MSEL_EINVAL;
        goto cleanup;
    }

    /* Initialize all relevant data structures */
    task = &msel_task_list[tasknum];
    msel_memset(task, 0, sizeof(msel_tcb));

    /* Setup task number to refer to its place in the task array */
    task->num = tasknum;
    
    /* Setup stack & heap */
    taskmem_init(tasknum);
    task->stack = task->stack_top = taskmem_stack_top(tasknum);
	task->stack_sz = taskmem_stack_size(tasknum);
	task->heap = taskmem_heap_start(tasknum);
	task->heap_sz = taskmem_heap_size(tasknum);

    /* Setup initial state */
    task->entry = entry;
    
    /* Task should not be blocking when started */
    task->wait_op  = MSEL_TASK_WAIT_NONE;
    
    /* Do arch-specific setup */
    if(MSEL_OK != arch_task_create(task))
    {
        ret = MSEL_EUNKNOWN;
        goto cleanup;
    }
    
    /* Save off arguments. Values must be copied because user tasks
     * will never be able to peek into other tasks' stack or heap */
    if(arg_sz && arg)
    {
        task->arg_sz = arg_sz;
        task->arg = heap_malloc(taskmem_heap_start(tasknum),arg_sz);
        if(!task->arg)
        {
            ret = MSEL_ERESOURCE;
            goto cleanup;
        }
        msel_memcpy(task->arg,arg,arg_sz);
    }

    /* Add new task to task list and return */
    task->valid    = 1;
    msel_num_tasks++;
    if (tid) *tid = tasknum;
    ret = MSEL_OK;
    
 cleanup:
    if(ret != MSEL_OK && task) 
    {
        /* Ensure task is re-marked as available if error */
        if (NULL != task)
          task->valid = 0;
    }
    return ret;
}

void msel_task_cleanup(msel_tcb* task)
{
    arch_task_cleanup(task);
    msel_memset(task,0,sizeof(*task));
}

/** @brief Setup memory management (if needed) for the current task */
void msel_task_setup_mm(msel_tcb* task) {
    arch_task_setup_mm(task);
}

void msel_task_exit()
{
    (void)msel_svc(MSEL_SVC_EXIT, NULL);
}

void msel_task_update_ctrs_resume()
{
    msel_task_list[msel_active_task_num].ctrs.runs = msel_task_list[msel_active_task_num].ctrs.runs + 1;
}

