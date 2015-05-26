/** @file aes_driver.c

    This file contains the syscall to access the AES crypto functions (in either
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

#include "aes_driver.h"

#ifdef USE_SW_AES
#include "swcrypto/sw_aes.h"
#else
#include "arch.h"
#endif

msel_status msel_do_aes(aes_driver_ctx_t* ctx)
{
    // Can't have an unsupported keysize or data length not divisible by 16
    if (ctx->key_size > AES_256) return MSEL_EINVAL;
    if (ctx->data_len % 16 != 0) return MSEL_EINVAL;

#ifdef USE_SW_AES
    sw_aes_ctx_t sw;
    msel_memset(&sw, 0, sizeof(sw));
    sw_aes_setkey(&sw, ctx->key_size, ctx->key);
    uint32_t i = 0;
    for (i = 0; i < ctx->data_len; i += AES_BLOCK_SIZE) {
      uint8_t din[AES_BLOCK_SIZE];
      uint8_t dout[AES_BLOCK_SIZE];
      msel_memcpy(din, ctx->din + i, AES_BLOCK_SIZE);
      if (ctx->enc)
        sw_aes_ecb_encrypt(&sw, din, dout);
      else
        sw_aes_ecb_decrypt(&sw, din, dout);
      msel_memcpy(ctx->dout + i, dout, AES_BLOCK_SIZE);
    }
    return MSEL_OK;
#else /* USE_SW_AES */
    return arch_do_hw_aes(ctx);
#endif /* USE_SW_AES */
}
