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

#include "driver/aes_driver.h"

msel_status arch_do_hw_aes(aes_driver_ctx_t* ctx)
{
    // Load the key
    msel_memcpy(AES_KEY_ADDR, ctx->key, (ctx->key_size + 2 ) * 8);
    
    // Set up the ctrl register
    uint32_t ctrl = 1; // go bit set
    if (!ctx->enc) ctrl |= (1 << 1); // decrypt instead of encrypt
    switch (ctx->key_size)
    {
        case AES_256: ctrl |= (1 << 3); break;
        case AES_192: ctrl |= (1 << 2); break;
        case AES_128: /* do nothing -- default to AES_128 */
        default: break;
    }

    // Loop through the data and encrypt/decrypt in 16-byte chunks
    unsigned i;
    for(i = 0; i < ctx->data_len; i += 16)
    {
        // Write data to be processed
        msel_memcpy(AES_DIN_ADDR, ctx->din + i, 16);

        // Start the encryption/decryption
        *(AES_CTRL_ADDR) = ctrl;

        // Poll busy until done
        while (*(AES_CTRL_ADDR) & (1 << 16)) { /* wait */ }

        // Read processed data
        msel_memcpy(ctx->dout + i, AES_DOUT_ADDR, 16);
    } 

    // Reset
    *(AES_CTRL_ADDR) = (1 << 8);
    return MSEL_OK;

}
