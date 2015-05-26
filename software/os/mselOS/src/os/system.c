/** @file system.c

    Implements rudimentary ARM Cortex-M3 operating system functionality with MPU support
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
#include <string.h>

/* initialization basically requires an include-all */

#include "arch.h"

#include "syscall.h"
#include "task.h"
#include "taskmem.h"
#include "mutex.h"

#include "driver/uart.h"
#include "driver/ffs_session.h"
#include "driver/uart.h"
#include "driver/led.h"

/* Global number of ticks since boot */
uint64_t msel_systicks;

/* Function definitions */

/** @brief this is the main task in the system 
    
    @return never returns
*/
void msel_main() 
{
    while(1) 
    {
        /* Handle suspended tasks */
        {
            size_t task_idx;
            for(task_idx=1;task_idx<msel_num_tasks;task_idx++)
            {
                msel_tcb *task = &msel_task_list[task_idx];
                msel_svc_restart_args rs_args;
                
                if(msel_task_is_waiting(task))
                {
                    switch(task->wait_op)
                    {

                    case MSEL_TASK_WAIT_POL:
                        rs_args.tasknum = task_idx;
                        rs_args.svcnum = MSEL_SVC_POL;
                        rs_args.arg = task->state.pol.retptr;
                        msel_svc(MSEL_SVC_RESTART, &rs_args);
                        break;
                        
                    case MSEL_TASK_WAIT_TIME:
                        /* NOT IMPL, just resume for now */
                        msel_task_resume(task);
                        break;
                        
                    default:
                        /* TODO: throw error here, is_waiting might be busted */
                        break;
                    }
                } /* else no need to resume */
                
            }
        }
        
        msel_svc(MSEL_SVC_WORKER,NULL);
        
        /* give up the time slice before re-running main loop */
        msel_svc(MSEL_SVC_YIELD,NULL);
    }
}


/** @brief contains the MSEL reset logic... this is not actually the
    reset vector. the actual reset code is in startup_m2sxxx.s 
*/
void msel_init() {
    ARCH_DISABLE_INTERRUPTS();
    
    msel_systicks = 0;

    /* Do any platform-specific initalization first */
    arch_platform_init();

    msel_init_uart();
    
    /* Setup Interrupt handling */
    msel_init_isr();

    // Set up the FFS data queues
    msel_init_ffs_queues();
    
    /* Initialize sub-components */
    msel_init_led();
    msel_init_taskmem();
    msel_init_task();
    msel_init_pol();
    
    ARCH_ENABLE_INTERRUPTS();
}


/** @brief hand over full control of the processor to msel, never to return

    @return never returns
*/
void msel_start() {
    msel_task_launch_main(); /* launch the main task, never returns */
}


/** @brief handle the systick timer */
void msel_systick_handler() {

    /* Increment the system 'timer' */
    msel_systicks++;

    arch_systick_handler();

    msel_set_status_leds();

    msel_task_schedule();
}

/* Blink leds for each task with freq proportional to runtime */
void msel_set_status_leds()
{
    ledio_t   leds = 0;
    size_t    i;

    for(i=0; i<msel_num_tasks && i<8; i++)
    {
        if(msel_task_is_valid(&(msel_task_list[i])))
        {
            size_t runs = msel_task_list[i].ctrs.runs;
            if((runs%100) > 30)
            {
                /* Start task leds from LED1 not LED0 */
                leds |= (1 << (i+1));
            }
        }
    }

    msel_led_set(MSEL_LED_ALL, leds);
}

/** @brief: Something has gome horribly wrong! */
void msel_panic(const char *what)
{
    /** TODO: do something more useful and ensure tasking is halted properly */
    uart_print_now(what);

    /* lock the cpu */
    while(1);
}

void msel_init_isr()
{
    arch_init_isr();
}
