/** @file syscall.c

    This file contains wrappers for all system/service calls, as well as the
    implementation of the system call framework itself
*/
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


#include <msel/master_key.h>
#include <msel/provision.h>

#include "system.h"
#include "syscall.h"
#include "task.h"

#include "driver/aes_driver.h"
#include "driver/ecc_driver.h"
#include "driver/sha_driver.h"
#include "driver/trng_driver.h"
#include "driver/ffs_session.h"
#include "driver/mtc.h"
#include "driver/pol_int.h"
#include "driver/uart.h"

#include "arch.h"

/* @brief handle a service call. This is called directly from the SVC
   interrupt handler and should never be called directly from a thread
   
   @param svcnum : The requested svc number
   
   @param arg : and arbitrary pointer for passing data down to a svc
   handler. This will usually be either ignored or a structure
   containing relavent IN/OUT data.
   
   @return : On success, returns MSEL_OK. For all errors other than
   invalid SVC number, the return value is defined by the exact SVC
   handler. Otherwise the error will be MSEL_EINVSVC

*/
msel_status msel_svc_handler(msel_svc_number svcnum, void *arg) {
    msel_status retval = MSEL_EUNKNOWN;

    switch(svcnum) {
    case MSEL_SVC_DEBUG:
        /* Just exec a breakpoint. Note that the environment may just
         * ignore this depending on how it is setup. Do not use a
         * breakpoint in leiu of a proper error handler. */
        ARCH_EMIT_BREAKPOINT();
        retval = MSEL_OK;
        goto end;
    case MSEL_SVC_RESET:
        retval = MSEL_ENOTIMPL;
        goto end;
    case MSEL_SVC_HALT:
        retval = MSEL_ENOTIMPL;
        goto end;
    case MSEL_SVC_WORKER:
        retval = msel_svc_worker();
        goto end;
    case MSEL_SVC_YIELD:
        /* Yield just invokes the default scheduler */
        retval = msel_task_schedule();
        goto end;
    case MSEL_SVC_RESTART:
        retval = msel_svc_restart((msel_svc_restart_args*)arg);
        goto end;
    case MSEL_SVC_EXIT:
        if(msel_active_task_num == 0)
            msel_panic("Task 0 exited.");
        msel_task_force_kill(msel_active_task_num, "exited");
        retval = MSEL_OK;
        goto end;

    /* MMIO syscalls */
    case MSEL_SVC_TRNG:
        retval = msel_trng_read((uint8_t*)arg);
        goto end;
    case MSEL_SVC_AES:
        retval = msel_do_aes((aes_driver_ctx_t*)arg);
        goto end;
    case MSEL_SVC_SHA:
        retval = msel_do_sha((sha_data_t*)arg);
        goto end;
    case MSEL_SVC_ECC:
        retval = msel_ecc_mul((ecc_ctx_t*)arg);
        goto end;
    case MSEL_SVC_FFS_SESSION_SEND:
        retval = msel_ffs_session_send((ffs_packet_t*)arg);
        goto end;
    case MSEL_SVC_FFS_SESSION_RECV:
        retval = msel_ffs_session_recv((ffs_packet_t*)arg);
        goto end;

    case MSEL_SVC_UART_WRITE:
        retval = msel_uart_write((msel_uart_write_args*)arg);
        goto end;

    case MSEL_SVC_MTC_READ_INC:
        retval = msel_mtc_read_increment((mtc_t*)arg);
        goto end;

    case MSEL_SVC_PROVISION:
        retval = msel_provision((struct provision_args*)arg);
        goto end;

    case MSEL_SVC_POL:
        retval = msel_proof_of_life((pol_t*)arg);
        goto end;
    default:
        retval = MSEL_EINVSVC;
        break;
    }
    
end:
    
    return retval;
}

/** @brief implements system call resume by refreshing arguments,
 * switching back to the saved task number and re-calling the original
 * svc function */
msel_status msel_svc_restart(msel_svc_restart_args* rs_args)
{
    msel_status retval = MSEL_EUNKNOWN;

    /* only msel_main may restart a system call */
    if(msel_active_task_num != 0) 
    {
	retval = MSEL_EPERM;
	goto end;
    }

    /* Never restart a restart call, inf. recursion would result */
    if(rs_args->svcnum == MSEL_SVC_RESTART)
    {
	retval = MSEL_EINVAL;
	goto end;
    }

    /* Fake like the orig task is calling again... main will
     * just resume again later like normal.. but the retval of
     * this svc call should be ignored by main */
    msel_active_task_num = rs_args->tasknum;
    msel_active_task = &(msel_task_list[rs_args->tasknum]);
    msel_task_resume(msel_active_task);
    retval = msel_svc_handler(rs_args->svcnum,rs_args->arg);

end:
    return retval;
}

/** @brief make a system call from a thread context. This is intended
    to be called directly from non-priviledged threads. See the
    individual svc implementation routines for further explanations of
    parameters and behaviors.
    
    @param svcnum : The requested svc number
    
    @param arg : and arbitrary pointer for passing data down to a svc
    handler. This will usually be either ignored or a structure
    containing relavent IN/OUT data.
    
    @return : On success, returns MSEL_OK. For all errors other than
    invalid SVC number, the return value is defined by the exact SVC
    handler. Otherwise the error will be MSEL_EINVSVC
*/
inline msel_status msel_svc(msel_svc_number svcnum, void *arg) {
    volatile register msel_status res;
    ARCH_DO_SYSCALL(res);
    return res;
}

/** @brief Performs periodic work tasks that need to be done in an ISR
 * context, called periodically from main. */
msel_status msel_svc_worker()
{
    msel_status retval = MSEL_EUNKNOWN;
    if(msel_active_task_num != 0)
    {
	retval = MSEL_EPERM;
	goto cleanup;
    }

    {
	/* Handle any background tasks here */   
//	msel_session_worker();
//	msel_pktbuf_worker();
    }

    retval = MSEL_OK;

cleanup:
    return retval;

}


