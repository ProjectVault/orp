/** @file ffs_driver.c

    This file contains syscalls for accessing the faux filesystem (FFS) interface
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

#include <msel.h>

#include "ffs_session.h"
#include "ffs_driver.h"
#include "arch.h"


msel_status msel_ffs_wfile_read(ffs_packet_t *pkt)
{
    return arch_ffs_wfile_read(pkt);
}

void msel_ffs_wfile_set_status(uint8_t status, uint8_t nonce)
{
    return arch_ffs_wfile_set_status(status, nonce);
}

uint8_t msel_ffs_wfile_get_status()
{
    return arch_ffs_wfile_get_status();
}

msel_status msel_ffs_rfile_write(ffs_packet_t *pkt)
{
    return arch_ffs_rfile_write(pkt);
}

void msel_ffs_rfile_clear()
{
    arch_ffs_rfile_clear();
}

uint8_t msel_ffs_rfile_get_status()
{
    return arch_ffs_rfile_get_status();
}
