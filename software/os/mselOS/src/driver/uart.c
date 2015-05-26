/** @file uart.c

    This driver provides a debug output API via a standart 16550 UART
    interface. If present, the UART will bne used to implement write
    only communication for any tasks that wish to dump information out
    via this channel.

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

#include <msel/stdc.h>

#include "os/syscall.h"
#include "uart.h"
#include "arch.h"

uart_queue_entry uart_queue[MSEL_TASKS_MAX]; 

int uart_enabled = 0;

void msel_init_uart()
{
    msel_memset(uart_queue, 0, sizeof(uart_queue));
    uart_enabled = arch_init_uart();
}


int uart_write_now(const char* data, size_t len)
{
    size_t n;
    for(n=0;n<len;n++)
    {
        /* block until available */
        while(!arch_uart_is_writable());
        /* write */
        arch_uart_putc(data[n]);
    }
    return n;
}

int uart_print_now(const char* data)
{
    return uart_write_now(data, msel_strlen(data));
}

int uart_send_next_byte()
{
    /* make starting task num static to round robin */
    static size_t task=0;

    size_t modidx;

    for(modidx=0; modidx<MSEL_TASKS_MAX; modidx++)
    {
        size_t n = (task + modidx) % MSEL_TASKS_MAX;
        
        if(uart_queue[n].args.buf)
        {
            if(uart_queue[n].offset < uart_queue[n].args.len)
            {
                uint8_t* buf = uart_queue[n].args.buf;
                size_t*  off = &(uart_queue[n].offset);
                arch_uart_putc(buf[(*(off))++]);
                task = n; /* save place for faster search next time */
                return 1;
            }
        }
    }
    return 0;
}

/* Send as much data off of the queue as is currently possible */
void uart_flush_queue()
{
    while(arch_uart_is_writable() && uart_send_next_byte());
}

msel_status msel_uart_write(msel_uart_write_args *wr_args)
{
    msel_status ret = MSEL_EUNKNOWN;

    /* Sanity check user supplied arguments */
    if(!wr_args->buf || !wr_args->len)
    {
        ret = MSEL_EINVAL;
        goto cleanup;
    }
    
    /* Save in queue if this is a new reqest */
    if(!uart_queue[msel_active_task_num].args.buf)
    {
        msel_memcpy(&(uart_queue[msel_active_task_num].args), wr_args, sizeof(*wr_args));
        uart_queue[msel_active_task_num].offset = 0;
    }

    /* else assume this is a resume... a task suspended in another
     * write can not send again, after all... */

    /* Try to flush it all out now, if possible */
retry:
    uart_flush_queue();

    if(uart_queue[msel_active_task_num].offset < uart_queue[msel_active_task_num].args.len)
    {
        /* Main can never block, spin until empty */
        if(msel_active_task_num == MSEL_TASK_MAIN)
            goto retry;

        /* Else suspend the syscall for later resume */
        msel_active_task->wait_op = MSEL_TASK_WAIT_UART;
        msel_task_schedule();
        ret = MSEL_ESUSP;
        goto cleanup;
    }

    /* Else all bytes for this call have been sent already, return success */
    uart_queue[msel_active_task_num].args.buf = NULL;
    uart_queue[msel_active_task_num].args.len = 0;
    uart_queue[msel_active_task_num].offset   = 0;
    
    ret = MSEL_OK;
cleanup:
    return ret;
}

msel_status uart_write(uint8_t *buf, size_t len)
{
    msel_uart_write_args wr_args;

    if(!uart_enabled)
        return MSEL_ERESOURCE;

    wr_args.buf = buf;
    wr_args.len = len;
    return msel_svc(MSEL_SVC_UART_WRITE, &wr_args);
}

msel_status uart_print(char* msg)
{
    msel_uart_write_args wr_args;

    if(!uart_enabled)
        return MSEL_ERESOURCE;

    wr_args.buf = (uint8_t*)msg;
    wr_args.len = msel_strlen(msg);
    return msel_svc(MSEL_SVC_UART_WRITE, &wr_args);

}
