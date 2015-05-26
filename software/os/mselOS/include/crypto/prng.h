/** @file prng.h
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

#ifndef _PRNG_H_
#define _PRNG_H_

#include <stdint.h>
#include <crypto/aes.h>

/** @ingroup crypto
 *  @{
 */


/** @defgroup prng Psudo-random number generation routines
 *  @{
 */

/** @brief The context structure for the psudo-random number generator. */
typedef struct prng_ctx_s {
  aes_ctx_t aes;
  uint64_t v[2];
  uint64_t dt[2];
} prng_ctx_t;

/** @brief Initialize a PRNG context, specifying which AES variant to use
 *         and providing an initial seed.
 */
void prng_init(prng_ctx_t *ctx, aes_algo_t algo, uint8_t *k, uint64_t v_lo, uint64_t v_hi);

/** @brief Generate AES_BLOCK_SIZE random bytes within the given context.
 */
void prng_output(prng_ctx_t *ctx, uint8_t *data_out);

/** @} */

/** @} */

#endif /* _PRNG_H_ */
