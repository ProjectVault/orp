/** @file aes_gcm.h
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

#ifndef _AES_GCM_H_
#define _AES_GCM_H_

#include <stdint.h>
#include <crypto/aes.h>

/** @ingroup aes
 *  @{
 */

/** @defgroup gcm Galois Counter Mode
 *  @{
 */


/** @brief Algorithm selector. See `AES_GCM_128`, `AES_GCM_192`, and `AES_GCM_256`. */
typedef uint32_t aes_gcm_algo_t;

/** @brief AES GCM 128 algorithm. */
#define AES_GCM_128   ((aes_gcm_algo_t) 0)

/** @brief AES GCM 192 algorithm. */
#define AES_GCM_192   ((aes_gcm_algo_t) 1)

/** @brief AES GCM 256 algorithm. */
#define AES_GCM_256   ((aes_gcm_algo_t) 2)

/** @brief AES GCM context object. */
typedef struct aes_gcm_ctx_s {
	aes_gcm_algo_t algo;
	uint64_t aad_len;
	uint64_t input_len;
	uint8_t tag[AES_BLOCK_SIZE];
	uint8_t h[AES_BLOCK_SIZE];
	uint8_t iv[AES_BLOCK_SIZE];
	uint8_t civ[AES_BLOCK_SIZE];
	aes_ctx_t e_ctx;
} aes_gcm_ctx_t;

/** @brief Size of AES GCM 128 key in bytes. */
#define AES_GCM_128_KEY_SIZE ((128 + 128) / 8)

/** @brief Size of AES GCM 192 key in bytes. */
#define AES_GCM_192_KEY_SIZE ((192 + 128) / 8)

/** @brief Size of AES GCM 256 key in bytes. */
#define AES_GCM_256_KEY_SIZE ((256 + 128) / 8)

/** @brief Set key in GCM context. */
void aes_gcm_setkey(aes_gcm_ctx_t *ctx, aes_gcm_algo_t algo, void *key, void *iv, uint32_t iv_len);

void aes_gcm_aad(aes_gcm_ctx_t *ctx, void *data_in, uint32_t data_len);

/** @brief AES GCM encrypt. */
void aes_gcm_encrypt(aes_gcm_ctx_t *ctx, void *data_in, uint32_t data_len, void *data_out);

/** @brief AES GCM decrypt. */
void aes_gcm_decrypt(aes_gcm_ctx_t *ctx, void *data_in, uint32_t data_len, void *data_out);

void aes_gcm_final(aes_gcm_ctx_t *ctx, void *tag);

/** @} */

/** @} */

#endif /* _AES_GCM_H_ */
