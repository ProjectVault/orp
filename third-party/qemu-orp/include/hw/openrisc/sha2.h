/** @file sha2.h
 *
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

#ifndef _SHA2_H_
#define _SHA2_H_

/** @ingroup crypto
 *  @{
 */


/** @defgroup sha SHA-256
 *  @{
 */

void sha256_transform(uint32_t* iv, uint8_t* bptr);


/** @} */

/** @} */

#endif /* _SHA2_H_ */

