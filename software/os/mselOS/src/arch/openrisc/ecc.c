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
#include <msel/stdc.h>

#include "arch.h"
#include "mmio.h"

msel_status arch_hw_ecc_mul(ecc_ctx_t* ctx) {
    // Load the point and the scalar
    msel_memcpy(ECC_SCALAR_ADDR, ctx->scalar, 128);
    msel_memcpy(ECC_POINT_ADDR, ctx->point, 128);
    
    // Start the multiplication
    *(ECC_CTRL_ADDR) = 1;

    // Poll busy until done
    while (*(ECC_CTRL_ADDR) & (1 << 16)) { /* wait */ }

    // Read processed data
    msel_memcpy(ctx->point, ECC_POINT_ADDR, 128);

    // Reset
    *(ECC_CTRL_ADDR) = (1 << 8);
    return MSEL_OK;
}
