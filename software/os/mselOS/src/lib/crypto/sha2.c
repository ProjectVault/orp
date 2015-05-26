/** @file sha2.c
 *
 *  Functions to use SHA-256 in mselOS
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


#include <msel.h>
#include <msel/stdc.h>
#include <msel/syscalls.h>

#include "crypto/sha2.h"

/** @brief Number of input bytes to SHA-256 */
#define SHA256_INPUT_SIZE (512 / 8)

/** @brief Number of output bytes for SHA-256 */
#define SHA256_HASH_SIZE  (256 / 8)

static const uint8_t sha2_iv[32] =
  { 0x6a, 0x09, 0xe6, 0x67, 0xbb, 0x67, 0xae, 0x85,
    0x3c, 0x6e, 0xf3, 0x72, 0xa5, 0x4f, 0xf5, 0x3a,
    0x51, 0x0e, 0x52, 0x7f, 0x9b, 0x05, 0x68, 0x8c,
    0x1f, 0x83, 0xd9, 0xab, 0x5b, 0xe0, 0xcd, 0x19
  };

/** @brief Stored stateful information for the SHA-2 algorithm */
typedef struct sha2_ctx_s {
	uint32_t pos;
	uint64_t Nl; /* low 64 bits of 128-bit counter of input bits */
	uint64_t Nh; /* high 64 bits of 128-bit counter of input bits */
    sha_data_t data;

} sha2_ctx_t;

static void sha2_update(sha2_ctx_t *ctx, const void *buf, uint64_t len)
{
	uint32_t ncpy;
	uint64_t Ntmp;
	const uint8_t *bptr = (const uint8_t *) buf;
	while(len)
	{
		ncpy = (SHA256_INPUT_SIZE - ctx->pos);
		ncpy = (ncpy > len) ? len : ncpy;
		msel_memcpy(&ctx->data.din[ctx->pos], bptr, ncpy);

		ctx->pos += ncpy;
		bptr += ncpy;
		len -= ncpy;

		Ntmp = ctx->Nl + (ncpy << 3);
		ctx->Nh += (Ntmp < ctx->Nl) ? 1 : 0;
		ctx->Nl = Ntmp;

		if(ctx->pos == SHA256_INPUT_SIZE)
		{
            msel_svc(MSEL_SVC_SHA, &ctx->data);
			ctx->pos = 0;
		}
	}
	return;
}

static void sha2_final(sha2_ctx_t *ctx, void *buf)
{
	uint32_t i;
	uint8_t *bptr;
	uint64_t Ntmp;

	ctx->data.din[ctx->pos++] = 0x80;
	if(ctx->pos > SHA256_INPUT_SIZE - 8)
	{
		msel_memset(&ctx->data.din[ctx->pos], 0, SHA256_INPUT_SIZE - ctx->pos);
        msel_svc(MSEL_SVC_SHA, &ctx->data);
		ctx->pos = 0;
	}

	msel_memset(&ctx->data.din[ctx->pos], 0, SHA256_INPUT_SIZE - ctx->pos);
	for(i = 0, Ntmp = ctx->Nh, bptr = &ctx->data.din[SHA256_INPUT_SIZE - 15]; i < 8; i++)
	{
		*(bptr++) = (Ntmp >> 56);
		Ntmp <<= 8;
	}
	for(i = 0, Ntmp = ctx->Nl, bptr = &ctx->data.din[SHA256_INPUT_SIZE - 8]; i < 8; i++)
	{
		*(bptr++) = (Ntmp >> 56);
		Ntmp <<= 8;
	}

    msel_svc(MSEL_SVC_SHA, &ctx->data);
    msel_memcpy(buf, ctx->data.iv, SHA256_HASH_SIZE);
	return;
}

/** @brief Compute the SHA-256 hash of an input
 *
 *  @param in The input data to hash
 *  @param len The length of the input data
 *  @param out A pointer to the output buffer to be filled (must be at least
 *    SHA256_HASH_SIZE bytes)
 */
void sha256_hash(const uint8_t *in, uint32_t len, uint8_t *out) {
  sha2_ctx_t ctx;
  msel_memset(&ctx, 0, sizeof(sha2_ctx_t));
  msel_memcpy(ctx.data.iv, sha2_iv, 32);
  sha2_update(&ctx, in, len);
  sha2_final(&ctx, out);
}

