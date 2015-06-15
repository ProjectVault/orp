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

#include "exec/address-spaces.h"
#include "hw/openrisc/mmio.h"

// Memory regions for the various hardware devices
static MemoryRegion trng_mmio_region;

// Read/write callbacks for the mmio regions
static uint64_t trng_read(void* opaque, hwaddr addr, unsigned size);
static void trng_write(void* opaque, hwaddr addr, uint64_t data, unsigned size);

// Define our callback functions
static const MemoryRegionOps trng_ops= { .read = &trng_read, .write = &trng_write, };

// TRNG callback definitions
static uint64_t trng_read(void* opaque, hwaddr addr, unsigned size)
{
    // Just use the system drand48 function
    uint8_t rnd = drand48() * 0xff;
    return rnd;
}

static void trng_write(void* opaque, hwaddr addr, uint64_t data, unsigned size)
{
    // Writes to TRNG are ignored
    return;
}

// Initialize the memory-mapped IO region and add it to the address space
void openrisc_orp_sim_init_mmio(void *opaque, MemoryRegion *address_space)
{
    // TRNG 
    long seed = time(NULL);
    //long seed = 1423786924;
    printf("SEED = %ld\n", seed);
    srand48(seed);
    memory_region_init_io(&trng_mmio_region, NULL, &trng_ops, opaque, "trng", TRNG_WIDTH);
    memory_region_add_subregion(address_space, TRNG_ADDR, &trng_mmio_region);

    // AES, SHA, ECC, FFS
    orp_init_aes_mmio(opaque, address_space);
    orp_init_sha_mmio(opaque, address_space);
    orp_init_ecc_mmio(opaque, address_space);
    orp_init_ffs_mmio(opaque, address_space);
}   
