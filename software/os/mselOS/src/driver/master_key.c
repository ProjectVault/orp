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
#include <msel/mtc.h>
#include <msel/master_key.h>

#include "arch.h"

msel_status msel_master_key_read(mkey_ptr_t val)
{
    return arch_master_key_read(val);
}

msel_status msel_master_key_write(mkey_ptr_t val)
{
    return arch_master_key_write(val);
}

