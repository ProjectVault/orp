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
#include "hw/openrisc/ecc.h"

#define SCALAR 0x000
#define POINT 0x080
#define CTRL 0x100

#define SCALAR_SZ 128
#define POINT_SZ 128
#define CTRL_SZ 4

#define GO    (ctrl[3] & 0x01)
#define RESET (ctrl[2] & 0x01)
#define BUSY  (ctrl[1] & 0x01)

#define SET_BUSY(val) (val ? ctrl[2] |= 1 : (ctrl[2] &= 0xfe))
#define SET_GO(val)   (val ? ctrl[0] |= 1 : (ctrl[0] &= 0xfe))

static uint8_t scalar[SCALAR_SZ] = {0};
static uint8_t point[POINT_SZ] = {0};
static uint8_t ctrl[CTRL_SZ] = {0};

static const uint8_t ctrl_mask_r[4] = {0x0, 0x1, 0x0, 0x0};
static const uint8_t ctrl_mask_w[4] = {0x0, 0x0, 0x01, 0x01};

static const ec_group_t* e521;

#ifdef MMIO_DEBUG
static void ecc_print_state(void)
{
    int i;
    printf("scalar: "); for (i = 0; i < SCALAR_SZ; ++i) printf("%02x ", (int)scalar[i]); printf("\n");
    printf("point: "); for (i = 0; i < POINT_SZ; ++i) printf("%02x ", (int)point[i]); printf("\n");
    printf("ctrl: "); for (i = 0; i < CTRL_SZ; ++i) printf("%02x ", (int)ctrl[i]); printf("\n");
}
#endif

// Need to convert from the MMIO region to the ECC library.  The MMIO region takes a point in
// big-endian format.  The ECC library expects 32-bit integers ordered little-endian.  So each group
// of four bytes from the MMIO region needs to be preserved in order, but the ordering of the
// "groups-of-four" needs to be flip-flopped.
static void make_ul(ul val, uint8_t* value, unsigned len)
{
    unsigned i;
    for (i = 0; i < 17; ++i)
    {
        val->x[i] = 0x0;
        val->x[i] |= (value[len - 4*i - 4] << 24);
        val->x[i] |= (value[len - 4*i - 3] << 16);
        val->x[i] |= (value[len - 4*i - 2] <<  8);
        val->x[i] |= (value[len - 4*i - 1]);
    }
}

static void from_ul(ul val, uint8_t* value, unsigned len)
{
    unsigned i;
    for (i = 0; i < 17; ++i)
    {
        value[len - 4*i - 4] = (val->x[i] >> 24);
        value[len - 4*i - 3] = (val->x[i] >> 16);
        value[len - 4*i - 2] = (val->x[i] >>  8);
        value[len - 4*i - 1] = (val->x[i]);
    }
}

/* Reset clears all of the data structure arrays */
static void ecc_reset(void)
{
    memset(scalar, 0, SCALAR_SZ);
    memset(point, 0, POINT_SZ);
    memset(ctrl, 0, CTRL_SZ);
}

/* Read out data; we can only read from POINT and the BUSY bit of the ctrl reg */
static uint64_t ecc_read(void* opaque, hwaddr addr, unsigned size)
{
    uint64_t data = 0;
    unsigned i;
    for (i = 0; i < size; ++i)
    {
        unsigned pos = addr + i;
        uint8_t byte;

        if (IN_RANGE(pos, SCALAR)) return 0;
        else if (IN_RANGE(pos, POINT)) return point[pos - POINT];
        else if (IN_RANGE(pos, CTRL))
            byte = ctrl[pos - CTRL] & ctrl_mask_r[pos - CTRL];

        // Need to return the bytes in the right order; first byte read is left-most byte of data
        data |= (byte << ((size - i - 1) * 8));
    }
    return data;
}

/* Write data; can load point, scalar, GO/RESET of ctrl */
static void ecc_write(void* opaque, hwaddr addr, uint64_t data, unsigned size)
{
    // Don't allow writes if the algorithm is currently doing a multiply 
    if (BUSY) return;

    // Read the bytes of data in reverse order, so we can just bitshift at the end
    int i;
    for (i = size - 1; i >= 0; --i)
    {
        hwaddr pos = addr + i;

        if (IN_RANGE(pos, SCALAR)) scalar[pos - SCALAR] = data;
        else if (IN_RANGE(pos, POINT)) point[pos - POINT] = data;
        else if (IN_RANGE(pos, CTRL))
            ctrl[pos - CTRL] = data & ctrl_mask_w[pos - CTRL];

        data >>= 8;
    }

    // If the RESET bit is toggled, don't do anything else
    if (RESET) ecc_reset();

    // Otherwise, run encryption if the GO bit is set
    else if (GO)
    {
        // We're busy encrypting -- just call the ECC lib
        SET_BUSY(1);
#ifdef MMIO_DEBUG
        ecc_print_state();
#endif

        // Get the scalar value
        ul n; make_ul(n, scalar, SCALAR_SZ);

        // Get the (compressed) point value
        ul cpt; make_ul(cpt, point, POINT_SZ);

        // Convert to montgomery form
        ul_to_montgomery(cpt, cpt, e521->p);

        // The sign of y is the 7th bit of the first byte
        uint8_t y_sign = point[0] & 0x01;

        // Uncompress the point value
        edwards_point_t pt; 
        uncompress_point(pt, cpt, y_sign, e521);

        // Do the multiply
        edwards_point_t out;
        edwards_montgomery_ladder(out, n, pt, e521);

        // Compress the point -- this removes from mont. form
        compress_point(cpt, &y_sign, out, e521);
        point[0] &= 0xfe; point[0] |= y_sign;

        // Load the new point into the buffer
        from_ul(cpt, point, POINT_SZ);

#ifdef MMIO_DEBUG
        ecc_print_state();
#endif
        // When we're done, toggle the GO and BUSY bits
        SET_GO(0); SET_BUSY(0);
    }
}

// Initialize the MMIO region
static MemoryRegion ecc_mmio_region;
static const MemoryRegionOps ecc_ops= { .read = &ecc_read, .write = &ecc_write, };

void orp_init_ecc_mmio(void* opaque, MemoryRegion* address_space)
{
    memory_region_init_io(&ecc_mmio_region, NULL, &ecc_ops, opaque, "ecc", ECC_WIDTH);
    memory_region_add_subregion(address_space, ECC_ADDR, &ecc_mmio_region);
    ecc_reset();
    e521 = ec_curve_lookup(ECC_CURVE_E521);
}
