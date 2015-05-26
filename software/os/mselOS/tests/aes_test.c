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

void get_task(const uint8_t **endpoint, void (**task_fn)(void *arg, const size_t arg_sz),
        uint16_t *port, const uint8_t* data)
{
    *endpoint = NULL;
    *task_fn = NULL;
}

// Three different test keys
uint8_t key[3][32] = {{0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0 },
                  {0x8e, 0x73, 0xb0, 0xf7, 0xda, 0x0e, 0x64, 0x52, 0xc8, 0x10, 0xf3, 0x2b, 0x80, 0x90, 0x79, 0xe5, 0x62, 0xf8, 0xea, 0xd2, 0x52, 0x2c, 0x6b, 0x7b, 0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0 },
                  {0x60, 0x3d, 0xeb, 0x10, 0x15, 0xca, 0x71, 0xbe, 0x2b, 0x73, 0xae, 0xf0, 0x85, 0x7d, 0x77, 0x81, 0x1f, 0x35, 0x2c, 0x07, 0x3b, 0x61, 0x08, 0xd7, 0x2d, 0x98, 0x10, 0xa3, 0x09, 0x14, 0xdf, 0xf4}};

// Four test input vectors
uint8_t din[4][16] = {{0x6b, 0xc1, 0xbe, 0xe2, 0x2e, 0x40, 0x9f, 0x96, 0xe9, 0x3d, 0x7e, 0x11, 0x73, 0x93, 0x17, 0x2a},
                  {0xae, 0x2d, 0x8a, 0x57, 0x1e, 0x03, 0xac, 0x9c, 0x9e, 0xb7, 0x6f, 0xac, 0x45, 0xaf, 0x8e, 0x51},
                  {0x30, 0xc8, 0x1c, 0x46, 0xa3, 0x5c, 0xe4, 0x11, 0xe5, 0xfb, 0xc1, 0x19, 0x1a, 0x0a, 0x52, 0xef},
                  {0xf6, 0x9f, 0x24, 0x45, 0xdf, 0x4f, 0x9b, 0x17, 0xad, 0x2b, 0x41, 0x7b, 0xe6, 0x6c, 0x37, 0x10}};

static void byte_to_string(uint8_t byte, uint8_t* str)
{
    str[0] = (byte >> 4);
    if (str[0] <= 9) str[0] += 0x30;
    else str[0] += 0x57;
    str[1] = byte & 0x0f;
    if (str[1] <= 9) str[1] += 0x30;
    else str[1] += 0x57;
}

void aes_task(void *arg, const size_t arg_sz)
{
    uint8_t str[2];
    uint8_t dout[16];

    // Loop through all key/vector pairs
    int i, j, k;
    for (i = 0; i < 3; ++i)
    {
        aes_driver_ctx_t aes_enc;
        aes_driver_ctx_t aes_dec;

        aes_enc.enc = 1;
        aes_enc.key_size = i;
        aes_enc.key = key[i];
        aes_enc.data_len = 16;
        aes_enc.dout = dout;

        aes_dec.enc = 0;
        aes_dec.key_size = i;
        aes_dec.key = key[i];
        aes_dec.data_len = 16;
        aes_dec.dout = dout;
        aes_dec.din = dout;

        for (j = 0; j < 4; ++j)
        {
            aes_enc.din = din[j];
            msel_svc(MSEL_SVC_AES, &aes_enc);
            for (k = 0; k < 16; ++k)
            {
                byte_to_string(dout[k], str);
                uart_write(str, 2);
            }
            uart_print("\n");
            msel_svc(MSEL_SVC_AES, &aes_dec);
            for (k = 0; k < 16; ++k)
            {
                byte_to_string(dout[k], str);
                uart_write(str, 2);
            }
            uart_print("\n");
            if (msel_memcmp(dout, din[j], 16) != 0)
                uart_print("ERROR!\n");
        }
    }
}

/** @brief Runs immediately after reset and gcc init. initializes system and never returns

    @return Never returns
*/
int main() {
    msel_status ret;

    /* let msel initialize itself */
    msel_init();


    if((ret = msel_task_create(aes_task,NULL,0,NULL)) != MSEL_OK)
        goto err;

    /* give control over to msel */
    msel_start();
    
err:
    while(1);

    /* never reached */
    return 0;
}


