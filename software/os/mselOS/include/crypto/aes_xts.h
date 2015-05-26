/** @file aes_xts.h
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

#ifndef _AES_XTS_H_
#define _AES_XTS_H_

#include <stdint.h>
#include <crypto/aes.h>

/** @ingroup aes
 *  @{
 */

/** @defgroup xts XTS Mode
 *  @{
 */

/** @brief Algorithm selector. See `AES_XTS_128`, `AES_XTS_192`, and `AES_XTS_256`. */
typedef uint32_t aes_xts_algo_t;

/** @brief AES XTS 128 algorithm. */
#define AES_XTS_128   ((aes_xts_algo_t) 0)

/** @brief AES XTS 192 algorithm. */
#define AES_XTS_192   ((aes_xts_algo_t) 1)

/** @brief AES XTS 256 algorithm. */
#define AES_XTS_256   ((aes_xts_algo_t) 2)

/** @brief Size of AES XTS 128 key in bytes. */
#define AES_XTS_128_KEY_SIZE (128 * 2 / 8)

/** @brief Size of AES XTS 256 key in bytes. */
#define AES_XTS_256_KEY_SIZE (256 * 2 / 8)

/** @brief AES XTS context object. */
typedef struct aes_xts_ctx_s {
	aes_xts_algo_t algo;
	aes_ctx_t t_ctx; /**< @brief Tweak vector context. */
	aes_ctx_t e_ctx; /**< @brief Encryption vector context. */
} aes_xts_ctx_t;

/** @brief Set key in XTS context. */
void aes_xts_setkey(aes_xts_ctx_t *ctx, aes_xts_algo_t algo, void *key);

/** @brief AES XTS encrypt. */
void aes_xts_encrypt(aes_xts_ctx_t *ctx, void *data_in, uint32_t block_count, uint64_t sequence, void *data_out);

/** @brief AES XTS encrypt. */
void aes_xts_decrypt(aes_xts_ctx_t *ctx, void *data_in, uint32_t block_count, uint64_t sequence, void *data_out);

/** @} */

/** @} */


#endif /* _AES_XTS_H_ */
