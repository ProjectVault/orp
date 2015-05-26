#include <msel.h>
#include <msel/tasks.h>
#include <msel/stdc.h>
#include <msel/malloc.h>
#include <msel/debug.h>

void get_task(const uint8_t **endpoint, void (**task_fn)(void *arg, const size_t arg_sz),
        uint16_t *port, const uint8_t* data)
{
    *endpoint = NULL;
    *task_fn = NULL;
}

#define HEAP_SPACE 2048 /* should always have this much per task */

#define NUM_PTRS 64

void task1(void *arg, const size_t arg_sz)
{
    int heap_sizes[] = {1,4,8,16,32,64,256,0};
    size_t idx,idx2;

    void *ptrlist[NUM_PTRS];
    

    while(1)
    {
        uart_print("TASK1 TEST STARTING\r\n");

        for(idx=0;idx<NUM_PTRS;idx++)
        {
            ptrlist[idx] = NULL;
        }

        /* Allocate up to HEAP_SPACE bytes foreach chunksize */
        for(idx=0; heap_sizes[idx] != 0; ++idx)
        {
            size_t num = HEAP_SPACE / heap_sizes[idx];

            num = (num > NUM_PTRS) ? NUM_PTRS : num;
            
            for(idx2=0;idx2<num;idx2++)
            {
                ptrlist[idx2] = msel_malloc(heap_sizes[idx]);
                if(!ptrlist[idx2])
                    uart_print("MALLOC ERROR\r\n");
            }

            for(idx2=0;idx2<num;idx2++)
            {
                msel_free(ptrlist[idx2]);
            }
        }

        uart_print("TASK1 TEST COMPLETE\r\n");

    }    
}


void task2(void *arg, const size_t arg_sz)
{
    while(1)
    {
        void *coolio = msel_malloc(1024);
        if(!coolio)
            uart_print("TASK2 MALLOC FAIL");
        msel_free(coolio);
        uart_print("TASK2 OK");
    }
}

void task3(void *arg, const size_t arg_sz)
{
    while(1)
    {
        void *coolio = msel_malloc(4);
        if(!coolio)
            uart_print("TASK3 MALLOC FAIL");
        msel_free(coolio);
        uart_print("TASK3 OK");
    }
}

int main()
{
    msel_init();

    msel_task_create(task1, NULL, 0, NULL);
    msel_task_create(task2, NULL, 0, NULL);
    msel_task_create(task3, NULL, 0, NULL);

    msel_start();

    /* never reached */
    while(1);
}
