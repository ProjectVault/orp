/** @file system.h
   
   Contains all internal definitions for the MicroSEL system. 
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

#ifndef _MSEL_SYSTEM_H_
#define _MSEL_SYSTEM_H_

#include <stdlib.h>
#include <stdint.h>

#include "msel.h"

/* Project configuration */

/** @brief total eSRAM in the system */
#define MSEL_RAM_SIZE   0x10000ul

/** @brief address at which eSRAM is mapped */
#define MSEL_RAM_START  0x20000000ul

/** @brief frequency of the SYSTICK interrupt, in HZ */
#define MSEL_TIMER_FREQ 5000

/** @brief maximum size of the task lists structure */
#define MSEL_TASKS_MAX 5

/** System management ASM macros, most are privileged operations */

/* fn decls */

void        msel_mss_init();
void        msel_main();
void        msel_systick_handler();
void        msel_panic();
void        msel_set_status_leds();
void        msel_init_isr();

#endif
