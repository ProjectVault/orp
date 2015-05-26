/** @file aes_driver.h

    Declares syscall for accessing the AES crypto functions 

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

#ifndef _MSEL_AES_DRIVER_H_
#define _MSEL_AES_DRIVER_H_

#include <stdlib.h>
#include <stdint.h>
#include <msel.h>
#include <crypto/aes.h>


/** @defgroup driver Device Drivers
 *  @{
 */

/** @defgroup aes_driver AES driver
 *  @{
 */

/** @brief Encrypt or decrypt a block of data.
 *  Call this function using the MSEL_SVC_AES syscall
 *
 *  @param args Input/output parameters for AES
 *  @return MSEL status value: 
 *    - MSEL_OK for succesful operation
 *    - MSEL_EINVAL for invalid input
 */
msel_status msel_do_aes(aes_driver_ctx_t* args);

/** @} */

/** @} */

#endif
