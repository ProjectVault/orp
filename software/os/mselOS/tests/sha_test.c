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

#include <crypto/sha2.h>

void get_task(const uint8_t **endpoint, void (**task_fn)(void *arg, const size_t arg_sz),
        uint16_t *port, const uint8_t* data)
{
    *endpoint = NULL;
    *task_fn = NULL;
}

static void byte_to_string(uint8_t byte, uint8_t* str)
{
    str[0] = (byte >> 4);
    if (str[0] <= 9) str[0] += 0x30;
    else str[0] += 0x57;
    str[1] = byte & 0x0f;
    if (str[1] <= 9) str[1] += 0x30;
    else str[1] += 0x57;
}

static uint8_t din1[3] = { 'a', 'b', 'c' };
static uint8_t din2[56] = { 'a', 'b', 'c', 'd', 'b', 'c', 'd', 'e', 'c', 'd', 'e', 'f', 'd', 'e', 'f', 'g', 'e', 'f', 'g', 'h', 'f', 'g', 'h', 'i', 'g', 'h', 'i', 'j', 'h', 'i', 'j', 'k', 'i', 'j', 'k', 'l', 'j', 'k', 'l', 'm', 'k', 'l', 'm', 'n', 'l', 'm', 'n', 'o', 'm', 'n', 'o', 'p', 'n', 'o', 'p', 'q' };
static uint8_t din3[112] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u' };

/** @brief task that calls the SHA-256 transform function */
void sha_task(void *arg, const size_t arg_sz) {

    uint8_t str[2];
    uint8_t dout[32];
    unsigned i;
    
    sha256_hash(din1, 3, dout); 
    for (i = 0; i < 32; ++i)
    {
        byte_to_string(dout[i], str);
        uart_write(str, 2);
    }
    uart_print("\n");
    sha256_hash(din1, 0, dout); 
    for (i = 0; i < 32; ++i)
    {
        byte_to_string(dout[i], str);
        uart_write(str, 2);
    }
    uart_print("\n");
    sha256_hash(din2, 56, dout); 
    for (i = 0; i < 32; ++i)
    {
        byte_to_string(dout[i], str);
        uart_write(str, 2);
    }
    uart_print("\n");
    sha256_hash(din3, 112, dout); 
    for (i = 0; i < 32; ++i)
    {
        byte_to_string(dout[i], str);
        uart_write(str, 2);
    }
    uart_print("\n");
}

/** @brief Runs immediately after reset and gcc init. initializes system and never returns

    @return Never returns
*/
int main() {
    msel_status ret;

    /* let msel initialize itself */
    msel_init();

    if((ret = msel_task_create(sha_task,NULL,0,NULL)) != MSEL_OK)
        goto err;

    /* give control over to msel */
    msel_start();
    
err:
    while(1);

    /* never reached */
    return 0;
}

