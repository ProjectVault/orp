/** @file sha2.h
 *
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

#ifndef _SHA2_H_
#define _SHA2_H_

#include <stdint.h>

/** @ingroup crypto
 *  @{
 */

/** @defgroup sha2 SHA-256 hash functionality
 *  @{
 */

/** @brief Number of bytes in SHA-256 output. */
#define SHA256_OUTPUT_LEN   ((uint32_t) (256/8))

/** @brief Compute SHA-256 hash of input.
 *
 * @param input Input buffer to hash. Assumed non-null.
 * @param l_input Number of bytes in `input`.
 * @param out Output buffer. Assumed non-null and pointing to a
 *        buffer of length `SHA256_OUTPUT_LEN`.
 */
void sha256_hash(const uint8_t *input, uint32_t l_input, uint8_t *out);

/** @} */
/** @} */

/** @addtogroup sha_driver
 *  @{
 */

/** @brief Data for the SHA-256 driver */
typedef struct sha_data_s
{
    /** @brief Initialization vector/output hash */
    uint8_t iv[32];

    /** @brief Input data to hash */
    uint8_t din[64];
} sha_data_t;

/** @} */


#endif /* _SHA2_H_ */

