/** @file trng_driver.h

    Declares routines for accessing the hardware random number generator

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

#ifndef _MSEL_TRNG_H_
#define _MSEL_TRNG_H_

#include <stdlib.h>
#include "msel.h"

/** @addtogroup driver
 *  @{
 */

/** @defgroup True RNG driver
 *  @{
 */

/** @brief Return a random number via the hardware RNG
 *  Call this function using the MSEL_SVC_TRNG syscall
 *
 *  @param[out] out An 8-byte random value
 *  @return MSEL status value: 
 *    - MSEL_OK for succesful operation
 */
msel_status msel_trng_read(uint8_t* out);

/** @} */

/** @} */

#endif
