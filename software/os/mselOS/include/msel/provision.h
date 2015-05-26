/** @file provision.h

    API for device provisioning 
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
#ifndef _MSEL_INC_PROVISION_H_
#define _MSEL_INC_PROVISION_H_

#include <msel.h>
#include <msel/mtc.h>
#include <msel/master_key.h>

struct provision_args
{
    mkey_ptr_t key;
    mtc_t* mtc;
};

/** @brief This API is a thread-callable interface that resets all
    device crypto status, destroying the previous state in the
    process. It is a write only API such that a thread is never
    allowed to read the master key directly.

    @param key New master key to save

    @param val New starting monotonic counter value

    @return On successful completion, returns MSEL_OK
*/
msel_status provision(mkey_ptr_t key, mtc_t* val);

/** @brief This behaves exactly like the "provision" system call,
    except that it is callable during system initialization, before
    "msel_start".

    @return On successful completion, returns MSEL_OK

*/
msel_status msel_provision(struct provision_args* arg);

#endif


