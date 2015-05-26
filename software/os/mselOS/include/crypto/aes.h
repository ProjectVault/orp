/** @file aes.h
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

#ifndef _AES_H_
#define _AES_H_

#include <stdint.h>

/** @defgroup crypto Cryptographic services
 *  @{
 */

/** @defgroup aes AES
 *  @{
 */

typedef enum { AES_128 = 0, AES_192 = 1, AES_256 = 2 } AES_size;

/** @brief Algorithm selector. See `AES_128`, `AES_192`, and `AES_256`. */
typedef uint32_t aes_algo_t;

/** @brief Size (in bytes) of an AES block. */
#define AES_BLOCK_SIZE	16

/** @brief AES context object. */
typedef struct aes_ctx_s {
	AES_size algo;
	uint8_t *key;
} aes_ctx_t;

/** @brief Size (in bytes) of an AES-128 key. */
#define AES_128_KEY_SIZE (128 / 8)

/** @brief Size (in bytes) of an AES-192 key. */
#define AES_192_KEY_SIZE (192 / 8)

/** @brief Size (in bytes) of an AES-256 key. */
#define AES_256_KEY_SIZE (256 / 8)

/** @brief Given a valid algo, yields the appropriate key size. */
#define AES_ALGO_KEY_SIZE(x) ( (AES_128 == (x)) ? AES_128_KEY_SIZE : AES_196 == (x)) ? AES_196_KEY_SIZE : AES_256_KEY_SIZE )

/** @brief Sets the key in the given context. */
void aes_setkey(aes_ctx_t *ctx, aes_algo_t algo, void *key);

/** @brief Performs encryption (electronic codebook mode) of the given data in the given context. */
void aes_ecb_encrypt(aes_ctx_t *ctx, void *data_in, void *data_out);

/** @brief Performs decryption (electronic codebook mode) of the given data in the given context. */
void aes_ecb_decrypt(aes_ctx_t *ctx, void *data_in, void *data_out);

/** @addtogroup aes_driver
 *  @{
 */

/** @brief Input data for the AES driver */
typedef struct aes_driver_ctx_s
{
    /** @brief Boolean value: encrypt data = 1, decrypt data = 0 */
    uint8_t enc;

    /** @brief Algorithm key size (AES-128, AES-192, AES-256) */
    AES_size key_size;

    /** @brief Encryption key */
    uint8_t* key;

    /** @brief Data to encrypt or decrypt */
    uint8_t* din;

    /** @brief Output from encryption/decryption */
    uint8_t* dout;

    /** @brief Number of bytes of din */
    unsigned data_len;
} aes_driver_ctx_t;

/** @} */

/** @} */

/** @} */

#endif /* _AES_H_ */
