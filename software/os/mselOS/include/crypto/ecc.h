/** @file ecc.h

    Structures and definitions for elliptic curve cryptographic operations.

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

#ifndef _MSEL_ECC_H_
#define _MSEL_ECC_H_

#include <stdlib.h>
#include <stdint.h>
#include <msel.h>

/** @addtogroup ecc_driver
 *  @{
 */

#define ECC_SCALAR_LEN  ((uint32_t) 128)
#define ECC_POINT_LEN   ((uint32_t) 128)

/** @brief Input/output data for the E-521 point/scalar multiplication */
typedef struct ecc_ctx_s
{
    /** @brief Bignum scalar */
    uint8_t* scalar;
    
    /** @brief Point on E-521 in compressed form */
    uint8_t* point;
} ecc_ctx_t;

/** @} */

#endif
