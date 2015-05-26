/** @file sw_sha.h

    Declares routines for running the SHA-256 transform function in software
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

#ifndef _MSEL_SW_SHA_H_
#define _MSEL_SW_SHA_H_

#include <stdint.h>

/** @addtogroup swcrypto
 *  @{
 */

/** @defgroup sw_sha Software SHA-256
 *  @{
 */

void c8to32(uint8_t* iv8, uint32_t* iv32);
void c32to8(uint32_t* iv32, uint8_t* iv8);
void sha256_transform(uint32_t* iv, uint8_t* bptr);

/** @} */

/** @} */

#endif // _MSEL_SW_SHA_H_
