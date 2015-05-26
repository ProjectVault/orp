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

void uart_test(void *arg, const size_t arg_sz) {

    size_t i;
    
    for(i=0;i<5;i++)
        uart_print("HELLO\r\n");

    uart_print("DONE\r\n");
}

int main() {
    msel_status ret;

    /* let msel initialize itself */
    msel_init();

    /* create threads here */
    if((ret = msel_task_create(uart_test,NULL,0,NULL)) != MSEL_OK)
        goto err;

    /* give control over to msel */
    msel_start();
    
err:
    while(1);

    /* never reached */
    return 0;
}



