/** @file ecc_driver.c

    This file contains the syscall to access the ECC crypto functions (in either
    software or hardware mode)
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

#include <stdint.h>
#include <msel.h>
#include <msel/stdc.h>
#include "ecc_driver.h"

#ifndef USE_SW_ECC
  #include "arch.h" /* pull in hw ecc driver */
#else /* USE_SW_ECC */
  #include "swcrypto/ed521.h"
  uint32_t scalar[ED521_LIMBS];
  uint32_t compressed[ED521_LIMBS];
  ec_point_t in, out;
#endif /* USE_SW_ECC */

/** @brief call the ECC power ladder point-scalar multiply */
msel_status msel_ecc_mul(ecc_ctx_t* ctx)
{
#ifdef USE_SW_ECC

	// Get the scalar value
	make_mp(scalar, ctx->scalar, ECC_SCALAR_LEN);
	make_mp(compressed, ctx->point, ECC_POINT_LEN);

	// The sign of y is the 7th bit of the first byte
	int y_sign = ctx->point[0] & 0x01;
	point_uncompress(&in, compressed, y_sign);

	// Do the multiply
	point_scalar(&out, &in, scalar);

	// Compress the point -- this removes from mont. form
	point_compress(compressed, &y_sign, &out);
	ctx->point[0] &= 0xfe; ctx->point[0] |= y_sign;

	// Load the new point into the buffer
	from_mp(ctx->point, compressed, ECC_POINT_LEN);

    return MSEL_OK;
#else /* USE_SW_ECC */
    return arch_hw_ecc_mul(ctx);
#endif /* USE_SW_ECC */
}




