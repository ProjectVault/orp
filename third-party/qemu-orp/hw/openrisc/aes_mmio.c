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
#include "hw/openrisc/mmio.h"
#include "hw/openrisc/aes.h"

#define KEY 0x0
#define DIN 0x20
#define DOUT 0x30
#define CTRL 0x40
#define END 0x44

#define KEY_SZ 32
#define DIN_SZ 16
#define DOUT_SZ 16
#define CTRL_SZ 4

#define GO    (ctrl[3] & 0x01)
#define MODE  ((ctrl[3] >> 1) & 0x01)
#define ALGO  ((ctrl[3] >> 2) & 0x03) 
#define RESET (ctrl[2] & 0x01)
#define BUSY  (ctrl[1] & 0x01)

#define SET_BUSY(val) (val ? ctrl[2] |= 1 : (ctrl[2] &= 0xfe))
#define SET_GO(val)   (val ? ctrl[0] |= 1 : (ctrl[0] &= 0xfe))

static aes_ctx_t ctx;
static uint8_t key[KEY_SZ] = {0};
static uint8_t din[DIN_SZ] = {0};
static uint8_t dout[DOUT_SZ] = {0};
static uint8_t ctrl[CTRL_SZ] = {0};

static const uint8_t ctrl_mask_r[4] = {0x0, 0x1, 0x0, 0x0};
static const uint8_t ctrl_mask_w[4] = {0x00, 0x00, 0x01, 0x0f};

#ifdef MMIO_DEBUG
static void aes_print_state(void)
{
    int i;
    printf("Key: "); for (i = 0; i < KEY_SZ; ++i) printf("%02x ", (int)key[i]); printf("\n");
    printf("din: "); for (i = 0; i < DIN_SZ; ++i) printf("%02x ", (int)din[i]); printf("\n");
    printf("dout: "); for (i = 0; i < DOUT_SZ; ++i) printf("%02x ", (int)dout[i]); printf("\n");
    printf("ctrl: "); for (i = 0; i < CTRL_SZ; ++i) printf("%02x ", (int)ctrl[i]); printf("\n");
}
#endif

/* Reset clears all of the data structure arrays */
static void aes_reset(void)
{
    memset(key, 0, KEY_SZ);
    memset(din, 0, DIN_SZ);
    memset(dout, 0, DOUT_SZ);
    memset(ctrl, 0, CTRL_SZ);
}

/* Read out data; we can only read from DOUT and the BUSY bit of the ctrl reg */
static uint64_t aes_read(void* opaque, hwaddr addr, unsigned size)
{
    uint64_t data = 0;
    unsigned i;
    for (i = 0; i < size; ++i)
    {
        unsigned pos = addr + i;
        uint8_t byte;

        if (IN_RANGE(pos, KEY)) return 0;
        else if (IN_RANGE(pos, DIN)) return 0;
        else if (IN_RANGE(pos, DOUT)) byte = dout[pos - DOUT];
        else if (IN_RANGE(pos, CTRL))     
            byte = ctrl[pos - CTRL] & ctrl_mask_r[pos - CTRL];

        // Need to return the bytes in the right order; first byte read is left-most byte of data
        data |= (byte << ((size - i - 1) * 8));
    }
    return data;
}

/* Write data; can load key, DIN, and GO/ENCDEC/SZ/RESET of ctrl */
static void aes_write(void* opaque, hwaddr addr, uint64_t data, unsigned size)
{
    // Don't allow writes if the algorithm is currently encrypting something
    if (BUSY) return;

    // Read the bytes of data in reverse order, so we can just bitshift at the end
    int i;
    for (i = size - 1; i >= 0; --i)
    {
        hwaddr pos = addr + i;

        if (IN_RANGE(pos, KEY)) key[pos - KEY] = data;
        else if (IN_RANGE(pos, DIN)) din[pos - DIN] = data;
        else if (IN_RANGE(pos, DOUT)) continue;
        else if (IN_RANGE(pos, CTRL))
            ctrl[pos - CTRL] = data & ctrl_mask_w[pos - CTRL];

        data >>= 8;
    }

    // If the RESET bit is toggled, don't do anything else
    if (RESET) aes_reset();

    // Otherwise, run encryption if the GO bit is set
    else if (GO)
    {
        // We're busy encrypting -- just call the AES lib
        SET_BUSY(1);
#ifdef MMIO_DEBUG
        aes_print_state();
#endif

        aes_setkey(&ctx, ALGO, key);
        if (MODE == 0) aes_ecb_encrypt(&ctx, din, dout); 
        else aes_ecb_decrypt(&ctx, din, dout);

#ifdef MMIO_DEBUG
        aes_print_state();
#endif
        // When we're done, toggle the GO and BUSY bits
        SET_GO(0); SET_BUSY(0);
    }
}

// Initialize the MMIO region
static MemoryRegion aes_mmio_region;
static const MemoryRegionOps aes_ops= { .read = &aes_read, .write = &aes_write, };

void orp_init_aes_mmio(void* opaque, MemoryRegion* address_space)
{
    memory_region_init_io(&aes_mmio_region, NULL, &aes_ops, opaque, "aes", AES_WIDTH);
    memory_region_add_subregion(address_space, AES_ADDR, &aes_mmio_region);
    aes_reset();
}
