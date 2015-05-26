/** @file sha_driver.h
 *
 *  Declares syscall for accessing the SHA-256 transform function
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

#ifndef _MSEL_SHA_H_
#define _MSEL_SHA_H_

#include <stdlib.h>
#include <stdint.h>
#include <msel.h>
#include "crypto/sha2.h"

/** @addtogroup driver 
 *  @{
 */

/** @defgroup sha_driver SHA-256 driver 
 *  @{
 */

/** @brief Compute the SHA-256 transform for a block of data.
 *  Call this function using the MSEL_SVC_SHA syscall
 *
 *  @param ctx Input/output parameters for SHA
 *  @return MSEL status value: 
 *    - MSEL_OK for succesful operation
 */
msel_status msel_do_sha(sha_data_t* ctx);

/** @} */

/** @} */

#endif
