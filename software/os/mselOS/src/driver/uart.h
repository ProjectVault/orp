/** @file uart.h

    Contains routines for using UART as part of session multiplexing

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

#ifndef _MSEL_UART_H_
#define _MSEL_UART_H_

#include <stdlib.h>
#include "msel.h"
#include "msel/debug.h" /* Task-callable definitions here */

typedef struct {
    uint8_t* buf;
    size_t  len;
} msel_uart_write_args;

typedef struct
{
    msel_uart_write_args args; /* orig dataptr & len */
    size_t offset;             /* current write progress */
} uart_queue_entry;

void msel_init_uart();
//msel_status msel_uart_read(void *dst, size_t len, size_t *rdlen);
//msel_status msel_uart_write(const void const* src, const size_t len);
//int msel_uart_is_write_complete();

msel_status msel_uart_write(msel_uart_write_args *wr_args);

/* These are *BLOCKING* and only to be called in an interrupt
 * context... and probably only from msel_panic when everything else
 * is hosed or at boot before tasking starts */

int         uart_write_now(const char* data, size_t len);
int         uart_print_now(const char* data);

#endif
