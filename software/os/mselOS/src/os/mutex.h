/** @file mutex.h

    Contains declarations to use for locking and unlocking shared resources 
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

#ifndef _MSEL_MUTEX_H_
#define _MSEL_MUTEX_H_

#include <stdint.h>

#include "system.h"
#include "task.h" /* because mutexen are bound to a task */

/** @brief msels implmentation of mutexes. a single word is used to
 * store the address of the thread's tcb that holds a lock, or 0 for
 * unlocked. All mutex locking operations use ldrex/strex instructions
 * to guarantee atomicity of the process. */
typedef uint32_t msel_mutex; 

/* @brief value indicating mutex is unlocked */
#define MSEL_MUTEX_UNLOCKED ((msel_tcb *)0)

/* @brief used if 'who' is not a task but a generic lock. Useful for locking between the main task and interrupt mode */
#define MSEL_MUTEX_GENERIC ((msel_tcb *)101)

msel_status msel_mutex_lock(msel_mutex*, msel_tcb*);
msel_status msel_mutex_unlock(msel_mutex*, msel_tcb*);
void        msel_mutex_initialize(msel_mutex*); 
int         msel_mutex_is_owned(msel_mutex*, msel_tcb*);

#endif
