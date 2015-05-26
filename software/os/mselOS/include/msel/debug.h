/** @file debug.h

    UART debug output messaging features for mselOS

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

#ifndef _MSEL_INC_DEBUG_H_
#define _MSEL_INC_DEBUG_H_

#include <stdint.h>
#include <msel.h>

/** @brief 

    Write a given length of characters to the system debug UART. A
    valid UART driver must exist for the platform, and it must have
    been initialized successfully at boot or else this call with be
    skipped over.

    @param buf Pointer to character data to write

    @param len Length of buf

    @return Number of bytes written

*/
msel_status uart_write(uint8_t *buf, size_t len);

/** @brief

    "uart_print" is a wrapper of "uart_write" that writes a C-style
    NULL terminated string instead of an arbitrary length-bounded
    array.

    @param msg NULL terminated string

    @return Number of bytes written
*/
msel_status uart_print(char* msg);

#endif
