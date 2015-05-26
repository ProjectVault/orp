/** @file prng.c
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

#include <crypto/prng.h>
#include <msel/stdc.h>

void prng_init(prng_ctx_t *ctx, aes_algo_t algo, uint8_t *k, uint64_t v_lo, uint64_t v_hi) {
  /* assert(NULL != ctx); */
  /* assert(AES_IS_ALGO(algo)); */
  /* assert(NULL != k); */
  ctx->v[0] = v_lo;
  ctx->v[1] = v_hi;
  ctx->dt[0] = 1;
  ctx->dt[1] = 0;
  aes_setkey(&ctx->aes, algo, k);
}

void prng_output(prng_ctx_t *ctx, uint8_t *data_out) {
  /* assert(NULL != ctx); */
  /* assert(NULL != data_out); */
  uint64_t *R = (uint64_t *)data_out;
  uint64_t t[2];
  uint64_t I[2];
  msel_memset(t, 0, sizeof(t));
  msel_memset(I, 0, sizeof(t));

  aes_ecb_encrypt(&ctx->aes, ctx->dt, I);
  ctx->dt[0] += 1;
  ctx->dt[1] += (0 == ctx->dt[0]) ? 1 : 0; /* add carry to hi if low overflowed to 0 */

  t[0] = I[0] ^ ctx->v[0];
  t[1] = I[1] ^ ctx->v[1];
  aes_ecb_encrypt(&ctx->aes, t, R);

  t[0] = R[0] ^ I[0];
  t[1] = R[1] ^ I[1];
  aes_ecb_encrypt(&ctx->aes, t, ctx->v);
}
