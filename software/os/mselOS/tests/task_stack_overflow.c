/** @file main.c

    The reset vector for all the Libero-generated code
*/

#include <stdlib.h>
#include <msel.h>
#include <msel/tasks.h>
#include <msel/debug.h>

void get_task(const uint8_t **endpoint, void (**task_fn)(void *arg, const size_t arg_sz),
        uint16_t *port, const uint8_t* data)
{
    *endpoint = NULL;
    *task_fn = NULL;
}

void do_stack_overflow()
{
    do_stack_overflow();
}

/** @brief Task that overflows the stack to test memory protections */
void overflow_task(void *arg, const size_t arg_sz) {
    uart_print("STARTING STACK OVERFLOW TEST");

    do_stack_overflow();
}


int main() {
    msel_status ret;

    /* let msel initialize itself */
    msel_init();

    /* create threads here */
    if((ret = msel_task_create(overflow_task,NULL,0,NULL)) != MSEL_OK)
        goto err;

    /* give control over to msel */
    msel_start();
    
err:
    while(1);

    /* never reached */
    return 0;
}


