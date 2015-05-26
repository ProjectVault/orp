/** @file ecc_driver.h
 *
 *  Declares syscall for accessing the ECC crypto functions 
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

#ifndef _MSEL_ECC_DRIVER_H_
#define _MSEL_ECC_DRIVER_H_

#include <stdlib.h>
#include <stdint.h>
#include <msel.h>
#include <crypto/ecc.h>

/** @addtogroup driver
 *  @{
 */

/** @defgroup ecc_driver E-521 EC driver
 *  @{
 */

/** @brief Compute the product of a point on E-521 with a scalar value.
 *  Call this function using the MSEL_SVC_ECC syscall
 *
 *  @param args Input/output parameters for ECC
 *  @return MSEL status value: 
 *    - MSEL_OK for succesful operation
 */
msel_status msel_ecc_mul(ecc_ctx_t* args);

/** @} */

/** @} */

#endif
