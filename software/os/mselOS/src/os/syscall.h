/** @file syscall.h

    Contains definitions for all internal syscall details

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

#ifndef _MSEL_SYSCALL_H_
#define _MSEL_SYSCALL_H_

#include "msel/syscalls.h"

typedef struct 
{
    size_t tasknum;
    msel_svc_number svcnum;
    void *arg;
} msel_svc_restart_args;

msel_status     msel_svc_handler(msel_svc_number,void*);
msel_status     msel_svc_restart(msel_svc_restart_args*);
msel_status     msel_svc_worker();

#endif
