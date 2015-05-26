#include <msel.h>
#include <msel/tasks.h>
#include <msel/mtc.h>
#include <msel/debug.h>

void get_task(const uint8_t **endpoint, void (**task_fn)(void *arg, const size_t arg_sz),
        uint16_t *port, const uint8_t* data)
{
    *endpoint = NULL;
    *task_fn = NULL;
}

/* Start up 2 of these tasks doing the same thing */
void mtc_reader(void *arg, const size_t arg_sz)
{
    msel_status ret;
    mtc_t val;
    mtc_t prev=0xff00ff00ff00ff00;

    while(1)
    {
        ret = mtc_read_increment(&val);

        if(ret != MSEL_OK)
            uart_print("MTC ERROR");
        
        if(prev==0xff00ff00ff00ff00)
            prev=val;
        
        /* Check to see that the other thread is also incrementing the mtc */
        if(val > prev+1)
            uart_print("MTC MULTITHREAD OK");

        if(val <= prev)
            uart_print("MTC NOT INC ERROR!");
        else
            uart_print("MTC INC OK");

        prev = val;
    }
}

int main()
{
    msel_init();

    msel_task_create(mtc_reader, NULL, 0, NULL);
    msel_task_create(mtc_reader, NULL, 0, NULL);
    
    msel_start();

    /* never reached */
    while(1);
}
