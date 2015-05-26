/** @file main.c

    The reset vector for all the Libero-generated code
*/

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <msel.h>
#include <msel/syscalls.h>
#include <msel/tasks.h>
#include <msel/ffs.h>
#include <msel/malloc.h>
#include <msel/uuid.h>
#include <msel/stdc.h>
#include <msel/debug.h>
#include <task.h>

/* 
 * List the provisioned tasks, along with their endpoint hashes here; the endpoint hash 
 * is a 32-byte array that uniquely identifies each of the tasks (applications) that can
 * be started on the operating system.
 *
 * When adding a new task, specify the endpoint here, and modify the get_task function 
 * to ensure that the endpoint is recognized by the OS
 */

/* 
 * Echo task: 
 *   Purpose: immediately echoes any incoming data to the task back out to the Android device
 *   Endpoint "abcd\n\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
 */
void echo_task(void *arg, const size_t arg_sz);
static const uint8_t echo_endpoint[32] =
  { 0x61, 0x62, 0x63, 0x64, 0xa, 0x0, 0x0, 0x0,
    0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,
    0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,
    0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0
  };

void get_task(const uint8_t **endpoint, void (**task_fn)(void *arg, const size_t arg_sz),
        uint16_t *port, const uint8_t* data)
{
    *endpoint = NULL;
    *task_fn = NULL;

    if (msel_memcmp(echo_endpoint, data, ENDPT_HASH_SIZE) == 0)
        { *endpoint = echo_endpoint; *task_fn = echo_task; return; }
}


/** @brief task that writes data out to the faux filesystem */
void echo_task(void *arg, const size_t arg_sz) {
    ffs_packet_t *pkt = msel_malloc(sizeof(ffs_packet_t));
    while (1)
    {
        if (msel_svc(MSEL_SVC_FFS_SESSION_RECV, pkt) == MSEL_OK)
        {
            while (msel_svc(MSEL_FFS_SVC_SESSION_SEND, pkt) != MSEL_OK) 
                msel_svc(MSEL_SVC_YIELD, NULL);
        }
        // TODO it would be nice to not have to YIELD in this task
        msel_svc(MSEL_SVC_YIELD, NULL);
    }
    msel_free(pkt);
}

/** @brief Runs immediately after reset and gcc init. initializes system and never returns

    @return Never returns
*/
int main() {
    msel_status ret;

    /* let msel initialize itself */
    msel_init();

    /* give control over to msel */
    msel_start();
    
err:
    while(1);

    /* never reached */
    return 0;
}

