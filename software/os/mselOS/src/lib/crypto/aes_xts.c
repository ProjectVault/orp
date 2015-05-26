/** @file aes_xts.c
 *
 *  Functions to use AES-XTS
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

#include <crypto/aes_xts.h>

/** @brief Set the key for AES XTS mode
 *  
 *  @param ctx A pointer to the AES-XTS context to be initialized
 *  @param key A pointer to the key data
 */
void aes_xts_setkey(aes_xts_ctx_t *ctx, aes_xts_algo_t algo, void *key)
{
  uint8_t *kptr;
  aes_algo_t subalgo;
  uint32_t subkey_size;

  ctx->algo = algo;
  switch(algo)
  {
  case AES_XTS_128:
    subalgo = AES_128;
    subkey_size = (128 / 8);
    break;
  case AES_XTS_256:
    subalgo = AES_256;
    subkey_size = (256 / 8);
    break;
  default:
    /* unused, case complete */
    return;
  }

  kptr = (uint8_t *) key;
  aes_setkey(&ctx->e_ctx, subalgo, &kptr[0]);
  aes_setkey(&ctx->t_ctx, subalgo, &kptr[subkey_size]);
  return;
}

/* IEEE 1619 - Annex C.2 */
static const uint8_t gf_mulx_reduce[] = { 0x00, 0x87 };

/** @brief Encrypt a block of data using AES XTS mode
 *
 *  @param ctx A valid AES_XTS context, with a set key
 *  @param data_in The data to encrypt
 *  @param block_count The size of the data to encrypt, in terms of 
 *    number of AES blocks
 *  @param sequence The location/sequence ID of the data to encrypt (e.g.,
 *    the sector ID when encrypting a filesystem)
 *  @param data_out The resulting encrypted data
 */
void aes_xts_encrypt(aes_xts_ctx_t *ctx, void *data_in, uint32_t block_count,
    uint64_t sequence, void *data_out)
{
  uint32_t i,j;
  uint8_t T[AES_BLOCK_SIZE], x[AES_BLOCK_SIZE];
  uint8_t Cin, Cout;
  uint8_t *din_ptr, *dout_ptr;

  din_ptr = (uint8_t *) data_in;
  dout_ptr = (uint8_t *) data_out;

  for(j = 0; j < AES_BLOCK_SIZE; j++)
  {
    T[j] = sequence;
    sequence >>= 8;
  }

  aes_ecb_encrypt(&ctx->t_ctx, T, T);

  for(i = 0; i < block_count; i++)
  {
    for(j = 0; j < AES_BLOCK_SIZE; j++)
      x[j] = *(din_ptr++) ^ T[j];

    aes_ecb_encrypt(&ctx->e_ctx, x, x);

    for(j = 0; j < AES_BLOCK_SIZE; j++)
      *(dout_ptr++) = x[j] ^ T[j];

    for(j = 0, Cin = 0; j < AES_BLOCK_SIZE; j++)
    {
      Cout = T[j] >> 7;
      T[j] = (T[j] << 1) | Cin;
      Cin = Cout;
    }
    T[0] ^= gf_mulx_reduce[Cout];
  }
  return;
}

/** @brief Decrypt a block of data using AES XTS mode
 *
 *  @param ctx A valid AES_XTS context, with a set key
 *  @param data_in The data to decrypt
 *  @param block_count The size of the data to decrypt, in terms of 
 *    number of AES blocks
 *  @param sequence The location/sequence ID of the data to decrypt (e.g.,
 *    the sector ID when encrypting a filesystem)
 *  @param data_out The resulting decrypted data
 */
void aes_xts_decrypt(aes_xts_ctx_t *ctx, void *data_in, uint32_t block_count,
    uint64_t sequence, void *data_out)
{
  uint32_t i, j;
  uint8_t T[AES_BLOCK_SIZE], x[AES_BLOCK_SIZE];
  uint8_t Cin, Cout;
  uint8_t *din_ptr, *dout_ptr;

  din_ptr = (uint8_t *) data_in;
  dout_ptr = (uint8_t *) data_out;

  for(j = 0; j < AES_BLOCK_SIZE; j++)
  {
    T[j] = sequence;
    sequence >>= 8;
  }

  aes_ecb_encrypt(&ctx->t_ctx, T, T);
  for(i = 0; i < block_count; i++)
  {
    for(j = 0; j < AES_BLOCK_SIZE; j++)
      x[j] = *(din_ptr++) ^ T[j];

    aes_ecb_decrypt(&ctx->e_ctx, x, x);

    for(j = 0; j < AES_BLOCK_SIZE; j++)
      *(dout_ptr++) = x[j] ^ T[j];

    for(j = 0, Cin = 0; j < AES_BLOCK_SIZE; j++)
    {
      Cout = T[j] >> 7;
      T[j] = (T[j] << 1) | Cin;
      Cin = Cout;
    }
    T[0] ^= gf_mulx_reduce[Cout];
  }
  return;
}
