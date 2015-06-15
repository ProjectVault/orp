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

#include "hw/openrisc/mmio.h"
#include "hw/openrisc/sha2.h"

#define IV 0x0
#define DIN 0x20
#define CTRL 0x60

#define IV_SZ 32
#define DIN_SZ 64
#define CTRL_SZ 4

#define GO    (ctrl[3] & 0x01)
#define RESET (ctrl[2] & 0x01)
#define BUSY  (ctrl[1] & 0x01)

#define SET_BUSY(val) (val ? ctrl[2] |= 1 : (ctrl[2] &= 0xfe))
#define SET_GO(val)   (val ? ctrl[0] |= 1 : (ctrl[0] &= 0xfe))

static uint8_t iv[IV_SZ] = {0};
static uint8_t din[DIN_SZ] = {0};
static uint8_t ctrl[CTRL_SZ] = {0};

static const uint8_t ctrl_mask_r[4] = {0x0, 0x1, 0x0, 0x0};
static const uint8_t ctrl_mask_w[4] = {0x0, 0x0, 0x01, 0x01};

#ifdef MMIO_DEBUG
static void sha_print_state(void)
{
    int i;
    printf("iv: "); for (i = 0; i < IV_SZ; ++i) printf("%02x ", (int)iv[i]); printf("\n");
    printf("din: "); for (i = 0; i < DIN_SZ; ++i) printf("%02x ", (int)din[i]); printf("\n");
    printf("ctrl: "); for (i = 0; i < CTRL_SZ; ++i) printf("%02x ", (int)ctrl[i]); printf("\n");
}
#endif

/* Reset clears all of the data structure arrays */
static void sha_reset(void)
{
    memset(iv, 0, IV_SZ);
    memset(din, 0, DIN_SZ);
    memset(ctrl, 0, CTRL_SZ);
}

// Convert between 8- and 32-bit integers
static void c8to32(uint8_t* iv8, uint32_t* iv32)
{
    unsigned i;
    for (i = 0; i < 32; i += 4)
        iv32[i / 4] = (iv8[i] << 24) | (iv8[i+1] << 16) | (iv8[i+2]) << 8 | iv8[i+3];
}

static void c32to8(uint32_t* iv32, uint8_t* iv8)
{
    unsigned i;
    for (i = 0; i < 32; i += 4)
    {
        iv8[i]     = iv32[i / 4] >> 24;
        iv8[i + 1] = iv32[i / 4] >> 16;
        iv8[i + 2] = iv32[i / 4] >> 8;
        iv8[i + 3] = iv32[i / 4];
    }
}

/* Read out data; we can only read from IV and the BUSY bit of the ctrl reg */
static uint64_t sha_read(void* opaque, hwaddr addr, unsigned size)
{
    uint64_t data = 0;
    unsigned i;
    for (i = 0; i < size; ++i)
    {
        unsigned pos = addr + i;
        uint8_t byte;

        if (IN_RANGE(pos, IV)) byte = iv[pos]; 
        else if (IN_RANGE(pos, DIN)) return 0;
        else if (IN_RANGE(pos, CTRL)) 
            byte = ctrl[pos - CTRL] & ctrl_mask_r[pos - CTRL];

        // Need to return the bytes in the right order; first byte read is left-most byte of data
        data |= (byte << ((size - i - 1) * 8));
    }
    return data;
}

/* Write data; can load IV, DIN, and GO/RESET bits of ctrl */
static void sha_write(void* opaque, hwaddr addr, uint64_t data, unsigned size)
{
    // Don't allow writes if the algorithm is currently encrypting something
    if (BUSY) return;

    // Read the bytes of data in reverse order, so we can just bitshift at the end
    int i;
    for (i = size - 1; i >= 0; --i)
    {
        hwaddr pos = addr + i;

        if (IN_RANGE(pos, IV))  iv[pos - IV] = data;
        else if (IN_RANGE(pos, DIN)) din[pos - DIN] = data;
        else if (IN_RANGE(pos, CTRL)) 
            ctrl[pos - CTRL] = data & ctrl_mask_w[pos - CTRL];

        data >>= 8;
    }

    // If the RESET bit is toggled, don't do anything else
    if (RESET) sha_reset();

    // Otherwise, run encryption if the GO bit is set
    else if (GO)
    {
        // We're busy encrypting -- just call the SHA-256 transform function
        SET_BUSY(1);
#ifdef MMIO_DEBUG
        sha_print_state();
#endif

        uint32_t iv32[8];
        c8to32(iv, iv32);
        sha256_transform(iv32, din);
        c32to8(iv32, iv);

#ifdef MMIO_DEBUG
        sha_print_state();
#endif
        // When we're done, toggle the GO and BUSY bits
        SET_GO(0); SET_BUSY(0);
    }
}

// Initialize the MMIO region
static MemoryRegion sha_mmio_region;
static const MemoryRegionOps sha_ops= { .read = &sha_read, .write = &sha_write, };

void orp_init_sha_mmio(void* opaque, MemoryRegion* address_space)
{
    memory_region_init_io(&sha_mmio_region, NULL, &sha_ops, opaque, "sha", SHA_WIDTH);
    memory_region_add_subregion(address_space, SHA_ADDR, &sha_mmio_region);
    sha_reset();
}
