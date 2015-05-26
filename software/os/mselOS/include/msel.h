/** @file include/msel.h

    This contains all PUBLIC definitions for top level mSEL structures

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

#ifndef _INC_MSEL_H_
#define _INC_MSEL_H_

/** @brief standardized error codes to be returned by all msel
 * system functions */
typedef enum {
    MSEL_EUNKNOWN  = -128,  /* default error, cause unknown */
    MSEL_EINVAL,            /* invalid function parameters */
    MSEL_ERESOURCE,         /* unable to allocate needed resources */
    MSEL_EBUSY,             /* requested resource is busy, try again later */
    MSEL_EINVSVC,           /* svcall number is not defined */
    MSEL_ENOTIMPL,          /* functionality is stubbed in, but ot implememted */
    MSEL_EBADF,             /* bad descriptor or reference */
    MSEL_ENOMEM,            /* not enough avail mem for operation */
    MSEL_EEXIST,            /* Attempt to create resource that already exists */
    MSEL_ESUSP,             /* Internal only: returned when a syscall suspended a task */
    MSEL_EAGAIN,            /* Temporarily unavailable */
    MSEL_EPERM,             /* Insufficient permissions */
    MSEL_OK = 0
} msel_status;

void        msel_init();
void        msel_start();

#endif
