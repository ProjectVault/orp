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
#include "arch.h"
#include "os/task.h"
#include "os/mutex.h"
#include "m3.h"


msel_status arch_mutex_lock(msel_mutex* mut, msel_tcb* who)
{
    register uint32_t lock_err = 1; /* 1 for fail to lock */
    msel_status ret = MSEL_EUNKNOWN;

    /* Atomic mutex lock via LDR/STR (EX)clusive */
    __asm__ volatile(
	 "try_lock:                \n"
	 "ldrex r2,[%1]            \n" /* ld exclusive */
	 "cmp r2, #0               \n" /* chk zero for unlocked */
	 "ITE EQ                   \n"
	 "strexeq r2, %2, [%1]     \n" /* try-store `who' to `mut' to lock */
	 "bne end                  \n"
         "cmp r2, #1               \n" /* check strex result */
         "beq try_lock             \n" /* loop until not interrupted */
	 "mov %0, #0               \n" /* update ret to 0 */
         "end:                     \n"
	 :"=r"(lock_err)
	 :"r"(mut), "r"(who)
	 :"r2"
    );

    if(!lock_err)
	ret = MSEL_OK;
    else
	ret = MSEL_EBUSY;

    return ret;
}

void arch_platform_init()
{
    
    /* Setup and enable MPU to apply to non-priv code only. Each
     * thread's own mem permissions will be managed during context
     * switches */
    *MPU_CTRL |= MPU_PRIVDEFENA | MPU_ENABLE;
}

void arch_systick_handler()
{
    /* Nothing platform-specific here. SysTick automatically resets itself */
}


