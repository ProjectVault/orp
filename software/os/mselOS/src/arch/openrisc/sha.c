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
#include <msel/stdc.h>

#include "arch.h"
#include "mmio.h"

msel_status arch_do_hw_sha(sha_data_t *data)
{
    // Copy in the initial IV
    msel_memcpy(SHA_IV_ADDR, data->iv, 32);
    
    // Copy next bit of data 
    msel_memcpy(SHA_DIN_ADDR, data->din, 64);
    *(SHA_CTRL_ADDR) = 1;
    
    // Poll busy until done
    while (*(SHA_CTRL_ADDR) & (1 << 16)) { /* wait */ }
    
    // Copy out the resulting hash
    msel_memcpy(data->iv, SHA_IV_ADDR, 32);
    
    // Reset the core
    *(SHA_CTRL_ADDR) = (1 << 8);
    return MSEL_OK;
}
