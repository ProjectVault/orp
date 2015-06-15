/** @file sha2.c

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

#include <stdint.h>
#include <string.h>

#include "hw/openrisc/sha2.h"

//void sha2_update(sha2_ctx_t *ctx,const  void *buf, uint64_t len);
//void sha2_final(sha2_ctx_t *ctx, void *buf);

#define Ch(x, y, z) (((x) & (y)) ^ ((~(x)) & (z)))
#define Maj(x, y, z) (((x) & ((y) ^ (z))) ^ ((y) & (z)))

#define ROTR32(x, y) (((x) >> (y)) | ((x) << (32 - (y))))

#define S0_32(x) (ROTR32((x), 2) ^ ROTR32((x), 13) ^ ROTR32((x), 22))
#define S1_32(x) (ROTR32((x), 6) ^ ROTR32((x), 11) ^ ROTR32((x), 25))
#define s0_32(x) (ROTR32((x), 7) ^ ROTR32((x), 18) ^ ((x) >> 3))
#define s1_32(x) (ROTR32((x), 17) ^ ROTR32((x), 19) ^ ((x) >> 10))

#define ROTR64(x, y) (((uint64_t) (x) >> (y)) | ((uint64_t) (x) << (64 - (y))))

#define S0_64(x) (ROTR64((x), 28) ^ ROTR64((x), 34) ^ ROTR64((x), 39))
#define S1_64(x) (ROTR64((x), 14) ^ ROTR64((x), 18) ^ ROTR64((x), 41))
#define s0_64(x) (ROTR64((x), 1)  ^ ROTR64((x), 8)  ^ ((x) >> 7))
#define s1_64(x) (ROTR64((x), 19) ^ ROTR64((x), 61) ^ ((x) >> 6))

static const uint32_t sha256_K[64] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
	0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
	0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
	0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
	0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
	0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
	0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
	0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
	0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

void sha256_transform(uint32_t* iv, uint8_t* bptr)
{
	uint32_t i;
	uint32_t a, b, c, d, e, f, g, h;
	uint32_t T1, T2;
	uint32_t W[64 + 16];

	a = iv[0];
	b = iv[1];
	c = iv[2];
	d = iv[3];
	e = iv[4];
	f = iv[5];
	g = iv[6];
	h = iv[7];

	for(i = 0; i < 16; i++, bptr += 4)
	{
		W[i] =    (((uint32_t) bptr[0]) << 24)
				| (((uint32_t) bptr[1]) << 16)
				| (((uint32_t) bptr[2]) <<  8)
				| (((uint32_t) bptr[3]) <<  0);
	}

	for(i = 0; i < 64; i++)
	{
		/* S1 + K[i] + W[i] can be computed ahead of time in FPGA */
		/* W[i + 16] can be computed ahead of time in FPGA */
		/* a, b, c, d, e, f, g, and h can be moved to a shift register
		 * if synthesis is too "simple" to pick up the pattern
		 */
		T1 = h + S1_32(e) + Ch(e, f, g) + sha256_K[i] + W[i];
		T2 = S0_32(a) + Maj(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + T1;
		d = c;
		c = b;
		b = a;
		a = T1 + T2;

		W[i + 16] = s1_32(W[i + 16 - 2]) + W[i + 16 - 7] + s0_32(W[i + 16 - 15]) + W[i + 16 - 16];
	}

	iv[0] += a;
	iv[1] += b;
	iv[2] += c;
	iv[3] += d;
	iv[4] += e;
	iv[5] += f;
	iv[6] += g;
	iv[7] += h;
	return;
}


