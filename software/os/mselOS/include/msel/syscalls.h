/** @file syscalls.h

    Contains definitions needed by external mSEL tasks that will need
    to utilize the system call interface 
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

#ifndef _INC_MSEL_SYSCALLS_H_
#define _INC_MSEL_SYSCALLS_H_

#include <msel.h>

/** @defgroup syscalls System calls
 *  @{
 */

/** @brief defines the various system call numbers */
typedef enum {

    /* System management */

    /** @brief force a soft reset */
    MSEL_SVC_RESET = 42,      
    /** @brief halt the system */
    MSEL_SVC_HALT,            
    /** @brief restart a system call on behalf of a suspended task */
    MSEL_SVC_RESTART,         
    /** @brief kicks off worker function in SVC context */
    MSEL_SVC_WORKER,          

    /* Task management */

    /** @brief indicates the thread can sleep */
    MSEL_SVC_YIELD,           
    /** @brief the thread will exit */
    MSEL_SVC_EXIT,            
    /** @brief does nothing */
    MSEL_SVC_DEBUG,           

    /* MMIO devices */

    /** @brief generate a random number */ 
    MSEL_SVC_TRNG,            
    /** @brief encrypt/decrypt with AES */
    MSEL_SVC_AES,             
    /** @brief hash using SHA-256 */
    MSEL_SVC_SHA,             
    /** @brief point-scalar ECC multiply */
    MSEL_SVC_ECC,             
    /** @brief receive the next incoming data packet for a session */
    MSEL_SVC_FFS_SESSION_RECV,
    /** @brief send data from a session */
    MSEL_SVC_FFS_SESSION_SEND,

    /** @brief Debug output over serial (write-only API) */
    MSEL_SVC_UART_WRITE,

    /** @brief Accessing the monotonic counter */
    MSEL_SVC_MTC_READ_INC,

    /** @brief Allows tasks to re-provision the system */
    MSEL_SVC_PROVISION,

    /** @brief  Detects user presence by requiring a physical input (button,
     * touch sensor, etc) to toggle state within a given time frame */
    MSEL_SVC_POL

} msel_svc_number;

msel_status msel_svc(msel_svc_number svcnum, void *arg);

/** @} */

#endif
