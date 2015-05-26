/** @file task.h

    definitions for debug tasks in task.c
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

#ifndef _MSEL_TASK_H
#define _MSEL_TASK_H

#include <msel/tasks.h>
#include <msel/malloc.h>

#include "config.h"
#include "system.h"
#include "driver/pol_int.h"

/** @brief The msel_main task is always first on the list (for now) */
#define MSEL_TASK_MAIN 0

/** @brief embedded env with so be conservative */
#define MSEL_TASK_MEM_MAX   PAGE_SIZE

typedef enum {
    MSEL_TASK_WAIT_NONE,
    MSEL_TASK_WAIT_TIME,
    MSEL_TASK_WAIT_UART,
    
    MSEL_TASK_WAIT_POL
} msel_task_wait_op;

/** @brief Thread Control Block: describes a thread to be added to the
 * task_list and/or a running thread */
typedef struct {

    /* Stack mgmt */
    uint8_t*             stack;       /* saved stack ptr is always the
                                       * first item for easy access
                                       * (MUST be 4-byte aligned). In effect this is (saved_regs*) */
    uint8_t*             stack_top;   /* the top of the allocated stack */
    size_t               stack_sz;    /* the size of the stack -- NOTE
                                       * free(stack_top-stack_sz) on
                                       * thread exit, dont mod these
                                       * fields whilre running */
    
    /* Heap mgmt */
    uint8_t*             heap;
    size_t               heap_sz;    

    /* Initial State */
    msel_thread_entry    entry;
    void*                arg;
    size_t               arg_sz;
    
    /* Scheduler state */
    unsigned int         valid:1;     /* is this task even real */
    unsigned int         killed:1;    /* has it been force terminated? */
    char*                reason;      /* why has it been killed (if killed) */
    msel_task_wait_op    wait_op;     /* if blocking, on what operation */

    /* union for the state of the suspending operation */
    union 
    {
        int              __reserved; /* Didn't you see the underscores?!?! */
        msel_ws_pol      pol;        /* Waiting for user to prove physical presence */
    } state;

    /* Let each architecture add their special sauce */
    struct _arch {

#ifdef BUILD_ARCH_OPENRISC
        uint8_t         ctx_id;
#else
        uint8_t         reserved;
#endif
    } arch;

    /* Save some performance/logging counters */
    struct _ctrs {
        uint32_t runs; /* Records how many times task has been resumed */
    } ctrs;

    size_t num; /* Task number */
} msel_tcb;

void        msel_init_task();
void        msel_task_setup_mm(msel_tcb*);
void        msel_task_launch_main();
msel_status msel_task_schedule();
int         msel_task_is_waiting(const msel_tcb const*);
int         msel_task_is_killed(const msel_tcb const*);
int         msel_task_is_valid(const msel_tcb const*);
void        msel_task_force_kill(size_t tasknum, char *reason); 
void        msel_task_resume(msel_tcb *);
void        msel_task_cleanup(msel_tcb*);
void        msel_task_update_ctrs_resume();

/* Export these globals unless included from task.c, where they are defined */
#ifndef _TASK_EXPORTS
extern msel_tcb msel_task_list[MSEL_TASKS_MAX];
extern size_t msel_num_tasks;
extern uint8_t msel_active_task_num;
extern msel_tcb* msel_active_task;
#endif


#endif
