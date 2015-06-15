/*
 *
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
#ifndef OPENRISC_MMIO
#define OPENRISC_MMIO
/*
 * MMIO device support for the Open Reference Platform
 */

#include "exec/address-spaces.h"

#define TRNG_ADDR 0x92000000
#define AES_ADDR 0x93000000
#define SHA_ADDR 0x94000000
#define ECC_ADDR 0x95000000
#define FFS_ADDR 0x98000000

#define TRNG_WIDTH 0x1
#define AES_WIDTH 0x44
#define SHA_WIDTH 0x64
#define ECC_WIDTH 0x104
#define FFS_WIDTH 0x1024

#define IN_RANGE(pos, LOC) ((pos >= LOC) && pos < LOC + LOC ## _SZ)

//#define MMIO_DEBUG

void orp_init_aes_mmio(void* opaque, MemoryRegion* address_space);
void orp_init_sha_mmio(void* opaque, MemoryRegion* address_space);
void orp_init_ecc_mmio(void* opaque, MemoryRegion* address_space);
void orp_init_ffs_mmio(void* opaque, MemoryRegion* address_space);
void openrisc_orp_sim_init_mmio(void *opaque, MemoryRegion *address_space);

#endif // OPENRISC_MMIO
