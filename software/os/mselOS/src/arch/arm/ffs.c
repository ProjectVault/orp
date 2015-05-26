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
#include "arch.h"

// Write a packet to wfile
msel_status arch_ffs_wfile_read(ffs_packet_t *pkt)
{
    return MSEL_ENOTIMPL;
}

void arch_ffs_wfile_set_status(uint8_t status, uint8_t nonce)
{}

uint8_t arch_ffs_wfile_get_status()
{
    return 0;
}

msel_status arch_ffs_rfile_write(ffs_packet_t *pkt)
{
    return MSEL_ENOTIMPL;
}

void arch_ffs_rfile_clear()
{}

uint8_t arch_ffs_rfile_get_status()
{
    return 0;
}
