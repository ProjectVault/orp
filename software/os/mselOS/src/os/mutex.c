/** @file mutex.c

    Defines support for mutexes to control access to shared resources

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

#include "mutex.h"
#include "arch.h"

/** @brief attempts to lock the requested mutex for the given thread.

    @param mut: the operand mutex pointer

    @param who: the thread requesting the operation 

    @return: On successful acquisition of the lock, returns MSEL_OK.
       MSEL_EBUSY: the lock is held by another thread
*/
msel_status msel_mutex_lock(msel_mutex* mut, msel_tcb* who)
{
    return arch_mutex_lock(mut, who);
}

/** @brief unlocks the given mutex for the referenced thread 
    
    @param mut: The mutex to attempt to unlock 

    @param who: what thread is asking

    @return: On successful clear of the lock, returns MSEL_OK.
       MSEL_EINVAL: 'mut' or 'who' are invalid
       MSEL_BADF:  the mutex is not held by 'who' and can't be unlocked for it
*/
msel_status msel_mutex_unlock(msel_mutex* mut, msel_tcb* who) 
{
    msel_status ret = MSEL_EUNKNOWN;

    /* bogus parameters */
    if(!mut || !who) {
	ret = MSEL_EINVAL;
	goto cleanup;
    }

    /* check that the unlocker actually owns the mutex */
    if(!msel_mutex_is_owned(mut,who)) {
	ret = MSEL_EBADF;
	goto cleanup;
    }
    
    /* Clear the lock.. no need for store exclusive here. Caller owns the lock due to check above */
    *((uint32_t*)mut) = 0;
    ret = MSEL_OK;

 cleanup:
    return ret;
}

/** @brief checks ownership of mutex 
    
    @param: 'mut' the mutex in question

    @param: 'who' the thread that would be the owner of mutex

    @return: non-zero if the thread owns the mutex. zero if not
*/
inline int msel_mutex_is_owned(msel_mutex* mut, msel_tcb* who)
{
    if(!mut)
	return 0;
    return (*((uint32_t*)mut) == (uint32_t)who);
}

/** @brief Initialize mutex for later use */
void msel_mutex_initialize(msel_mutex* mut) 
{
    *((uint32_t*)mut) = (uint32_t)MSEL_MUTEX_UNLOCKED;
}
