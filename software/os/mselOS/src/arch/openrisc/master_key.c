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
#include <stdlib.h>
#include <stdint.h>
#include <msel.h>
#include <msel/stdc.h>
#include <msel/master_key.h>
#include "flash.h"

#define MKEY_BANK_NUM 1022 /* 0xE7FC0000 - 0xE7FCffff */

msel_status arch_master_key_read(mkey_ptr_t val)
{
    flash_readmem(val, FLASH_ADDR(MKEY_BANK_NUM,0,0), MASTER_KEY_SIZE);
    return MSEL_OK;
}

msel_status arch_master_key_write(mkey_ptr_t val)
{
    flash_erase_bank(MKEY_BANK_NUM);
    return flash_writemem((void*)FLASH_ADDR(MKEY_BANK_NUM,0,0), val, MASTER_KEY_SIZE);
}

