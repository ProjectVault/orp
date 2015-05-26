/** @file aes_gcm.c
 * 
 *  Functions to use AES in GCM mode
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

#include <crypto/aes_gcm.h>
#include <msel/stdc.h>

static void gcm_inc_ctr(uint8_t *a)
{
  uint32_t i;
  uint8_t c;

  for(i = 15, c = 1; i < AES_BLOCK_SIZE; i--)
  {
    a[i] += c;
    c = c & ((a[i]) ? 0 : 1);
  }
  return;
}

static const uint8_t gf_mul_reduce[] = { 0x00, 0xe1 };

static void gf_mul(uint8_t *x, uint8_t *y)
{
  uint32_t i, j, k;
  uint8_t t, s, Cin, Cout;
  uint8_t v[2][AES_BLOCK_SIZE];

  msel_memset(v[0], 0, AES_BLOCK_SIZE);
  msel_memcpy(v[1], x, AES_BLOCK_SIZE);
  msel_memset(x, 0, AES_BLOCK_SIZE);

  for(i = 0; i < AES_BLOCK_SIZE; i++)
  {
     t = y[i];
     for(j = 0; j < 8; j++)
     {
       s = t >> 7;
       t <<= 1;

       for(k = 0; k < AES_BLOCK_SIZE; k++)
         x[k] ^= v[s][k];

       for(k = 0, Cin = 0; k < AES_BLOCK_SIZE; k++)
       {
         Cout = v[1][k] & 1;
         v[1][k] = (v[1][k] >> 1) | (Cin << 7);
         Cin = Cout;
       }
       v[1][0] ^= gf_mul_reduce[Cin];
     }
  }
  return;
}

static void gcm_ghash_add(aes_gcm_ctx_t *ctx, uint8_t *x)
{
  uint32_t i;

  for(i = 0; i < AES_BLOCK_SIZE; i++)
    ctx->tag[i] ^= x[i];

  gf_mul(ctx->tag, ctx->h);
  return;
}

/** @brief Set up a GCM context with a specified key and IV
 *
 *  @param ctx The AES-GCM context to be initialized
 *  @param algo One of AES_GCM_128, AES_GCM_198, or AES_GCM_256
 *  @param key A pointer to the key for AES GCM mode
 *  @param iv A pointer to the IV for AES GCM mode
 *  @param iv_len The number of bytes of IV for AES GCM mode
 */
void aes_gcm_setkey(aes_gcm_ctx_t *ctx, aes_gcm_algo_t algo, void *key, void *iv, uint32_t iv_len)
{
  uint8_t *iv_ptr;
  aes_algo_t subalgo;
  uint32_t i, bit_len;

  msel_memset(ctx, 0, sizeof(*ctx));
  ctx->algo = algo;
  switch(algo)
  {
  case AES_GCM_128:
    subalgo = AES_128;
    break;
  case AES_GCM_192:
    subalgo = AES_192;
    break;
  case AES_GCM_256:
    subalgo = AES_256;
    break;
  default:
    /* unused, case complete */
    return;
  }

  aes_setkey(&ctx->e_ctx, subalgo, key);
  aes_ecb_encrypt(&ctx->e_ctx, ctx->h, ctx->h);

  if(iv_len == 12)
  {
    msel_memcpy(ctx->iv, iv, 12);
    ctx->iv[15] = 1;
  }
  else
  {
    iv_ptr = (uint8_t *) iv;
    bit_len = (iv_len << 3);

    while(iv_len >= AES_BLOCK_SIZE)
    {
      for(i = 0; i < AES_BLOCK_SIZE; i++)
        ctx->iv[i] ^= *(iv_ptr++);
      gf_mul(ctx->iv, ctx->h);
      iv_len -= AES_BLOCK_SIZE;
    }
    if(iv_len)
    {
      for(i = 0; i < iv_len; i++)
        ctx->iv[i] ^= *(iv_ptr++);
      gf_mul(ctx->iv, ctx->h);
    }
    for(i = 15; i < AES_BLOCK_SIZE; i--)
    {
      ctx->iv[i] ^= bit_len;
      bit_len >>= 8;
    }
    gf_mul(ctx->iv, ctx->h);
  }
  msel_memcpy(ctx->civ, ctx->iv, sizeof(ctx->civ));
  return;
}

/** @brief not used */
void aes_gcm_aad(aes_gcm_ctx_t *ctx, void *data_in, uint32_t data_len)
{
  uint8_t *din_ptr;
  uint8_t x[AES_BLOCK_SIZE];

  din_ptr = (uint8_t *) data_in;
  ctx->aad_len += data_len;

  while(data_len >= AES_BLOCK_SIZE)
  {
    gcm_ghash_add(ctx, din_ptr);
    din_ptr += AES_BLOCK_SIZE;
    data_len -= AES_BLOCK_SIZE;
  }

  if(data_len)
  {
    msel_memset(x, 0, sizeof(x));
    msel_memcpy(x, din_ptr, data_len);
    gcm_ghash_add(ctx, x);
  }
  return;
}

/** @brief Encrypt a block of data using AES GCM
 *
 *  @param ctx A valid AES_GCM context, with a set key
 *  @param data_in The data to encrypt
 *  @param data_len The number of bytes of data to encrypt
 *  @param data_out The resulting encrypted data
 */
void aes_gcm_encrypt(aes_gcm_ctx_t *ctx, void *data_in, uint32_t data_len, void *data_out)
{
  uint32_t i;
  uint8_t *din_ptr, *dout_ptr;
  uint8_t x[AES_BLOCK_SIZE];

  ctx->input_len += data_len;

  din_ptr = (uint8_t *) data_in;
  dout_ptr = (uint8_t *) data_out;

  while(data_len >= AES_BLOCK_SIZE)
  {
    gcm_inc_ctr(ctx->civ);
    aes_ecb_encrypt(&ctx->e_ctx, &ctx->civ, x);

    for(i = 0; i < AES_BLOCK_SIZE; i++)
      dout_ptr[i] = din_ptr[i] ^ x[i];
    gcm_ghash_add(ctx, dout_ptr);

    din_ptr += AES_BLOCK_SIZE;
    dout_ptr += AES_BLOCK_SIZE;
    data_len -= AES_BLOCK_SIZE;
  }

  if(data_len)
  {
    gcm_inc_ctr(ctx->civ);
    aes_ecb_encrypt(&ctx->e_ctx, ctx->civ, x);

    for(i = 0; i < data_len; i++)
      x[i] ^= din_ptr[i];
    for(; i < AES_BLOCK_SIZE; i++)
      x[i] = 0;
    gcm_ghash_add(ctx, x);

    for(i = 0; i < data_len; i++)
      dout_ptr[i] = x[i];
  }
  return;
}

/** @brief Decrypt a block of data using AES GCM
 *
 *  @param ctx A valid AES_GCM context, with a set key
 *  @param data_in The data to decrypt
 *  @param data_len The number of bytes of data to decrypt
 *  @param data_out The resulting decrypted data
 */
void aes_gcm_decrypt(aes_gcm_ctx_t *ctx, void *data_in, uint32_t data_len, void *data_out)
{
  uint32_t i;
  uint8_t x[AES_BLOCK_SIZE];
  uint8_t *din_ptr, *dout_ptr;

  din_ptr = (uint8_t *) data_in;
  dout_ptr = (uint8_t *) data_out;

  ctx->input_len += data_len;
  while(data_len >= AES_BLOCK_SIZE)
  {
    gcm_inc_ctr(ctx->civ);
    aes_ecb_encrypt(&ctx->e_ctx, ctx->civ, x);
    gcm_ghash_add(ctx, din_ptr);
    for(i = 0; i < AES_BLOCK_SIZE; i++)
      dout_ptr[i] = x[i] ^ din_ptr[i];

    din_ptr += AES_BLOCK_SIZE;
    dout_ptr += AES_BLOCK_SIZE;
    data_len -= AES_BLOCK_SIZE;
  }

  if(data_len > 0)
  {
    gcm_inc_ctr(ctx->civ);

    for(i = 0; i < data_len; i++)
      x[i] = din_ptr[i];
    for(; i < AES_BLOCK_SIZE; i++)
      x[i] = 0;
    gcm_ghash_add(ctx, x);

    aes_ecb_encrypt(&ctx->e_ctx, ctx->civ, x);
    for(i = 0; i < data_len; i++)
      dout_ptr[i] = din_ptr[i] ^ x[i];
  }
  return;
}

/** @brief not used */
void aes_gcm_final(aes_gcm_ctx_t *ctx, void *tag)
{
  uint32_t i;
  uint8_t *tptr;
  uint8_t x[AES_BLOCK_SIZE];
  uint64_t v;

  v = (ctx->aad_len << 3);
  for(i = 0; i < AES_BLOCK_SIZE / 2; i++)
  {
    x[i] = (v >> 56);
    v <<= 8;
  }
  v = (ctx->input_len << 3);
  for(; i < AES_BLOCK_SIZE; i++)
  {
    x[i] = (v >> 56);
    v <<= 8;
  }
  gcm_ghash_add(ctx, x);

  aes_ecb_encrypt(&ctx->e_ctx, ctx->iv, x);

  tptr = (uint8_t *) tag;
  for(i = 0; i < AES_BLOCK_SIZE; i++)
    *(tptr++) = ctx->tag[i] ^ x[i];
  return;
}
