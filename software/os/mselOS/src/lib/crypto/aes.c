/** @file aes.c
 *
 *  Functions to encrypt and decrypt data using AES
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

#include <crypto/aes.h>
#include <msel.h>
#include <msel/syscalls.h>

/** @brief Set the AES key and algorithm (128, 192, 256)
 *  
 *  @param ctx Pointer to a valid AES context
 *  @param algo One of AES_128, AES_192, or AES_256
 *  @param key Pointer to the AES key
 */
void aes_setkey(aes_ctx_t *ctx, aes_algo_t algo, void *key) {
  ctx->algo = algo;
  ctx->key = (uint8_t *)key;
}

/** @brief Encrypt data with AES
 *
 *  @param ctx Pointer to a valid AES context with a set key
 *  @param data_in The data to be encrypted
 *  @param data_out The resulting encrypted data
 */
void aes_ecb_encrypt(aes_ctx_t *ctx, void *data_in, void *data_out) {
  aes_driver_ctx_t driver_ctx;
  driver_ctx.enc = 1;
  driver_ctx.key_size = ctx->algo;
  driver_ctx.key = ctx->key;
  driver_ctx.data_len = 16;
  driver_ctx.din = data_in;
  driver_ctx.dout = data_out;
  msel_svc(MSEL_SVC_AES, &driver_ctx);
}

/** @brief Decrypt data with AES
 *
 *  @param ctx Pointer to a valid AES context with a set key
 *  @param data_in The data to be decrypted
 *  @param data_out The resulting unencrypted data
 */
void aes_ecb_decrypt(aes_ctx_t *ctx, void *data_in, void *data_out) {
  aes_driver_ctx_t driver_ctx;
  driver_ctx.enc = 0;
  driver_ctx.key_size = ctx->algo;
  driver_ctx.key = ctx->key;
  driver_ctx.data_len = 16;
  driver_ctx.din = data_in;
  driver_ctx.dout = data_out;
  msel_svc(MSEL_SVC_AES, &driver_ctx);
}
