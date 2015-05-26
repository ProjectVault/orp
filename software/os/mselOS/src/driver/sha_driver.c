/** @file sha_driver.c

    This file contains the syscall to access the SHA-256 transform functions (in either
    software or hardware mode)

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

#include <msel.h>
#include <msel/stdc.h>

#include "arch.h"
#include "sha_driver.h"

#ifdef USE_SW_SHA2
#include "swcrypto/sw_sha.h"
#endif

#include <stdint.h>

msel_status msel_do_sha(sha_data_t *data)
{
#ifdef USE_SW_SHA2
    uint32_t iv32[8];
    c8to32(data->iv, iv32);
    sha256_transform(iv32, data->din);
    c32to8(iv32, data->iv);
    return MSEL_OK;
#else /* USE_SW_SHA2 */
    return arch_do_hw_sha(data);
#endif
}



