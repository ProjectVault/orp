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


void get_task(const uint8_t **endpoint, void (**task_fn)(void *arg, const size_t arg_sz),
        uint16_t *port, const uint8_t* data)
{
    *endpoint = NULL;
    *task_fn = NULL;
}

/* Some simplistic example/debugging tasks. These should all be
 * removed prior to any release */
void syscall_task(void *arg, const size_t arg_sz);
void overflow_task(void *arg, const size_t arg_sz);
void memfault_task(void *arg, const size_t arg_sz);
void counting_task(void *arg, const size_t arg_sz);
void yielding_task(void *arg, const size_t arg_sz);
void session_task_1(void *arg, const size_t arg_sz);
void session_task_2(void *arg, const size_t arg_sz);
void malloc_task(void *arg, const size_t arg_sz);
void echo_task_2(void *arg, const size_t arg_sz);
void echo_task_1(void *arg, const size_t arg_sz);

void monotonic_task();
void prng_task();

/** @brief Runs immediately after reset and gcc init. initializes system and never returns

    @return Never returns
*/
int main() {
    msel_status ret;

    /* let msel initialize itself */
    msel_init();

    /* create threads here */

    /*
    if((ret = msel_task_create(syscall_task,1024,4096,NULL)) != MSEL_OK)
	goto err;
    if((ret = msel_task_create(overflow_task,1024,4096,NULL)) != MSEL_OK)
	goto err;
    if((ret = msel_task_create(counting_task,1024,0,NULL)) != MSEL_OK)
	goto err;
    if((ret = msel_task_create(yielding_task,1024,0,NULL)) != MSEL_OK)
        goto err;
    */

/*    if ((ret = msel_task_create(ffs_task,NULL,0,NULL)) != MSEL_OK)
        goto err;*/

/*    if((ret = msel_task_create(memfault_task,1024,0,NULL)) != MSEL_OK)
        goto err;*/
    if((ret = msel_task_create(yielding_task,NULL,0,NULL)) != MSEL_OK)
        goto err;


    /*

    if((ret = msel_task_create(monotonic_task,1024,4096,NULL)) != MSEL_OK)
	goto err;
    if((ret = msel_task_create(prng_task,1024,4096,NULL)) != MSEL_OK)
	goto err;

    */

    /* give control over to msel */
    msel_start();
    
err:
    while(1);

    /* never reached */
    return 0;
}


/** @brief just an example task that makes a syscall */
void syscall_task(void *arg, const size_t arg_sz) {
    while(1)
	msel_svc(MSEL_SVC_DEBUG,NULL);
}

/** @brief Task that overflows the stack to test memory protections */
void overflow_task(void *arg, const size_t arg_sz) {
    overflow_task(arg,arg_sz);
}

/** @brief task that attempts to write to code memory  */
void memfault_task(void *arg, const size_t arg_sz) {
    /* write to some address forbidden by MMU */
    while(1)
        *((uint32_t*)0x00100000) = 1;
}

/** @brief task that just increments a counter in a loop */
void counting_task(void *arg, const size_t arg_sz) {
    uint32_t ctr;

    while((ctr=1)) {
        while(ctr < 50000);
        ctr++;
    }
}

/** @brief task that counts and yields the cpu time manually */
void yielding_task(void *arg, const size_t arg_sz) {
    uint32_t ctr;

    while((ctr=1)) {

        uart_print("CHECK\r\n");

        while(ctr++ < 50000);
        msel_svc(MSEL_SVC_YIELD, NULL);
    }
}

void malloc_task(void *arg, const size_t arg_sz) 
{
    int* one;
    char *two;

    while(1)
    {
	one = (int *)msel_malloc(sizeof(int));
	*one = 1;

	two = (char *)msel_malloc(msel_strlen("Fun fun fun"));
	msel_strcpy(two, "Fun fun fun");

	msel_free(one);
	one = (int *)msel_malloc(sizeof(int));
    
	msel_free(two);
	msel_free(one);
    }
}
