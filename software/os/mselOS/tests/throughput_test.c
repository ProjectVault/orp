/** @file main.c

    The reset vector for all the Libero-generated code
*/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <msel.h>
#include <msel/syscalls.h>
#include <msel/tasks.h>
#include <msel/malloc.h>
#include <msel/uuid.h>
#include <msel/stdc.h>
#include <msel/debug.h>

#include <crypto/aes.h>
#include <crypto/sha2.h>

void get_task(const uint8_t **endpoint, void (**task_fn)(void *arg, const size_t arg_sz),
        uint16_t *port, const uint8_t* data)
{
    *endpoint = NULL;
    *task_fn = NULL;
}

void easybreak(const char *unused) { }

const int numLoops = 1024 * 2;
const int loopDataLen = 512;

#include "mmio.h"


msel_status local_do_aes(aes_driver_ctx_t* ctx) {
    return msel_svc(MSEL_SVC_AES, ctx);

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

void throughput_task(void *arg, const size_t arg_sz)
{
    long aesEncDelta = 0;
    long aesEncBytes = 0;
    double aesEncBperSec = 0;

    long aesDecDelta = 0;
    long aesDecBytes = 0;
    double aesDecBperSec = 0;

    long shaHshDelta = 0;
    long shaHshBytes = 0;
    double shaHshBperSec = 0;

    uint8_t din[loopDataLen];
    for (int i = loopDataLen; i < sizeof(din); i++)
        din[i] = i;
    uint8_t dout[loopDataLen];
    uint8_t key[32] = { 0x00, 0x01, 0x02, 0x03
                      , 0x04, 0x05, 0x06, 0x07
                      , 0x08, 0x09, 0x0a, 0x0b
                      , 0x0c, 0x0d, 0x0e, 0x0f
                      , 0x10, 0x11, 0x12, 0x13
                      , 0x14, 0x15, 0x16, 0x17
                      , 0x18, 0x19, 0x1a, 0x1b
                      , 0x1c, 0x1d, 0x1e, 0x1f };

    easybreak("aes encrypt start");
    {
        // aes encrypt test
        aes_driver_ctx_t aes_enc;
        aes_enc.enc = 1;
        aes_enc.key_size = 0;
        aes_enc.key = key;
        aes_enc.data_len = loopDataLen;
        aes_enc.din = din;
        aes_enc.dout = dout;
        aesEncDelta = msel_systicks - aesEncDelta;
        for (int i = 0; i < numLoops; i++) {
            local_do_aes(&aes_enc);
            aesEncBytes += loopDataLen;
        }
        aesEncDelta = msel_systicks - aesEncDelta;
        aesEncBperSec = aesEncBytes / (aesEncDelta * 0.01);
    }
    easybreak("aes decrypt start");
    {
        // aes decrypt test
        
                aes_driver_ctx_t aes_dec;
        aes_dec.enc = 0;
        aes_dec.key_size = 0;
        aes_dec.key = key;
        aes_dec.data_len = loopDataLen;
        aes_dec.din = dout;
        aes_dec.dout = din;
        aesDecDelta = msel_systicks - aesDecDelta;
        for (int i = 0; i < numLoops; i++) {
            local_do_aes(&aes_dec);
            aesDecBytes += loopDataLen;
        }
        aesDecDelta = msel_systicks - aesDecDelta;
        aesDecBperSec = aesDecBytes / (aesDecDelta * 0.01);
    }
    easybreak("sha256 start");
    {
        // sha256 hash test
        shaHshDelta = msel_systicks - shaHshDelta;
        for (int i = 0; i < numLoops; i++) {
            sha256_hash(din, loopDataLen, dout);
            shaHshBytes += loopDataLen;
        }
        shaHshDelta = msel_systicks - shaHshDelta;
        shaHshBperSec = shaHshBytes / (shaHshDelta * 0.01);
    }

    (void)aesEncBperSec;
    (void)aesDecBperSec;
    (void)shaHshBperSec;
    
    easybreak("all tests done");
    while (1) { }
}

/** @brief Runs immediately after reset and gcc init. initializes system and never returns

    @return Never returns
*/
int main() {
    msel_status ret;

    /* let msel initialize itself */
    msel_init();

    if((ret = msel_task_create(throughput_task,NULL,0,NULL)) != MSEL_OK)
        goto err;

    /* give control over to msel */
    msel_start();
    
err:
    while(1);

    /* never reached */
    return 0;
}


