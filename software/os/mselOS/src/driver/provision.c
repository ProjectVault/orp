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
#include <msel.h>
#include <msel/syscalls.h>
#include <msel/provision.h>

#include "mtc.h"

msel_status msel_provision(struct provision_args* arg)
{
    msel_status ret;
    
    ret = msel_mtc_write(arg->mtc);

    if(ret != MSEL_OK)
        return ret;
    
    ret = msel_master_key_write(arg->key);

    if(ret != MSEL_OK)
        return ret;
    
    return MSEL_OK;
}

msel_status provision(mkey_ptr_t key, mtc_t* val)
{
    struct provision_args args;
    args.key = key;
    args.mtc = val;
    
    return msel_svc(MSEL_SVC_PROVISION, &args);
}
