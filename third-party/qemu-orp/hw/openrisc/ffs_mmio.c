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
#include "hw/openrisc/ffs_sockets.h"
#include "qemu/timer.h"
#include "sysemu/sysemu.h"
#include "qemu/option.h"

#include <sys/socket.h>

#define WFILE_DATA 0x0
#define RFILE_DATA 0x0800
#define WFILE_ACK 0x1000
#define RFILE_ACK 0x1010
#define CTRL 0x1020

#define WFILE_DATA_SZ 2048
#define RFILE_DATA_SZ 2048
#define WFILE_ACK_SZ 16
#define RFILE_ACK_SZ 16
#define CTRL_SZ 4

static uint8_t wfile_data[WFILE_DATA_SZ] = {0};
static uint8_t rfile_data[RFILE_DATA_SZ] = {0};
static uint8_t wfile_ack[RFILE_ACK_SZ] = {0};
static uint8_t rfile_ack[RFILE_ACK_SZ] = {0};
static uint8_t ctrl[CTRL_SZ] = {0};

// TODO
static const uint8_t ctrl_mask_r[4] = {0x0, 0x0, 0x0, 0x0};
static const uint8_t ctrl_mask_w[4] = {0x0, 0x0, 0x0, 0x0};

// Timer for polling the network interface
static QEMUTimer* ffs_socket_timer;
static int rfile_sock_fd, wfile_sock_fd;
static int curr_wfile_fd = -1;
static int curr_rfile_fd = -1;

// Packet values
const uint8_t DATA_REQUEST =   0x01;
const uint8_t STATUS_REQUEST = 0x02;

/* Reset clears all of the data structure arrays */
static void ffs_reset(void)
{
    memset(wfile_data, 0, WFILE_DATA_SZ);
    memset(rfile_data, 0, RFILE_DATA_SZ);
    memset(wfile_ack, 0, WFILE_ACK_SZ);
    memset(rfile_ack, 0, RFILE_ACK_SZ);
    memset(ctrl, 0, CTRL_SZ);
}

/* 
 * Note that the only part of the OS should be able to read is the wfile buffer (that is, the data
 * that has been written to the OS by the peripheral device)
 */
static uint64_t ffs_read(void* opaque, hwaddr addr, unsigned size)
{
    uint64_t data = 0;

    unsigned i;
    for (i = 0; i < size; ++i)
    {
        unsigned pos = addr + i;
        uint8_t byte;

        // Read what the peripheral wrote to wfile
        if (IN_RANGE(pos, WFILE_DATA)) 
            byte = wfile_data[pos]; 
        
        // Can't read rfile
        else if (IN_RANGE(pos, RFILE_DATA)) 
            byte = 0;

        // Check the OS acknowledgement status for wfile
        else if (IN_RANGE(pos, WFILE_ACK)) 
            byte = wfile_ack[pos - WFILE_ACK];   

        // Check Android's acknowledgement of data
        else if (IN_RANGE(pos, RFILE_ACK))
            byte = rfile_ack[pos - RFILE_ACK];
        
        // Check values in the CTRL register
        else if (IN_RANGE(pos, CTRL))
            byte = ctrl[pos - CTRL] & ctrl_mask_r[pos - CTRL];

        else byte = 0;

        // Need to return the bytes in the right order; first byte read is left-most byte of data
        data |= (byte << ((size - i - 1) * 8));
    } 
    return data;
}

/* 
 * Things get confusing here when figuring out which things are readable and writeable by the OS.
 * We use the naming convention to be consistent with the peripheral's view of the device.
 * Therefore, wfile is writeable BY THE PERIPHERAL, but shouldn't ever be overwritten by the OS.
 * On the other hand, rfile is not writeable BY THE PERIPHERAL, but is writeable by the OS to
 * transmit data to the peripheral device.
 */
static void ffs_write(void* opaque, hwaddr addr, uint64_t data, unsigned size)
{
    // Read the bytes of data in reverse order, so we can just bitshift at the end
    int i;
    for (i = size - 1; i >= 0; --i)
    {
        hwaddr pos = addr + i;

        // The OS doesn't write to wfile
        if (IN_RANGE(pos, WFILE_DATA))
            continue;

        // The OS writes to rfile, and clears the rfile_ack buffer
        else if (IN_RANGE(pos, RFILE_DATA))
            rfile_data[pos - RFILE_DATA] = data;

        // The OS acknowledges receipt of data from the peripheral through wfile_ack
        else if (IN_RANGE(pos, WFILE_ACK))
            wfile_ack[pos - WFILE_ACK] = data;

        // The OS can't write to the rfile acknowledgement
        else if (IN_RANGE(pos, RFILE_ACK))
            continue;

        // Ctrl reg--don't need to do anything here
        else if (IN_RANGE(pos, CTRL))
            ctrl[pos - CTRL] = data & ctrl_mask_w[pos - CTRL];

        data >>= 8;
    }
}

// Initialize the MMIO region
static MemoryRegion ffs_mmio_region;
static const MemoryRegionOps ffs_ops= { .read = &ffs_read, .write = &ffs_write, };

static int waiting_for_data = 0;
static ssize_t recv_len = 0;

// This function is called periodically to poll the network interfaces for incoming data
static void ffs_socket_timer_cb(void* opaque)
{
    OpenRISCCPU* cpu = opaque;
    CPUState* cs = CPU(cpu);

    uint64_t next;

    check_incoming_connection(rfile_sock_fd, &curr_rfile_fd);
    check_incoming_connection(wfile_sock_fd, &curr_wfile_fd);

    // From the perspective of Qemu, the peripheral device is writing to wfile, so
    // we call recv to get the data
    if (curr_wfile_fd != -1)
    {
        uint8_t wfile_cmd = 0;

		// There are two states we can be in at this point:
		//   1. Waiting for command (i.e., !waiting_for_data)
		//   2. Waiting for data
        if (!waiting_for_data) 
		{
			// Waiting for command (i.e., !waiting_for_data): in this state, we want to
			// read a single byte from the socket to tell us what to do next.  If we recv
			// a DATA_REQUEST, then we know that the next 2048 bytes will (should) be data
			// to be passed to the operating system.  Alternately, the command could be a
			// STATUS_REQUEST, in which case the filesystem just returns the contents of 
			// the status buffer.
			recv_len = safe_recv(&curr_wfile_fd, &wfile_cmd, 1, 0);
			if (recv_len > 0)
			{
                if (wfile_cmd == DATA_REQUEST) 
                    waiting_for_data = 1;
                else if (wfile_cmd == STATUS_REQUEST)
                    send(curr_wfile_fd, wfile_ack, WFILE_ACK_SZ, 0);
				else fprintf(stderr, "ERROR in wfile_cmd!\n");
				recv_len = 0;
			}
		}
        else 
		{
			// Waiting for data: in this state, we are in the process of getting the 2048
			// bytes to pass to the OS.  recv caps out at around 1400 bytes on my machine,
			// so we need to ensure we've recv'd all the data before actually sending the 
			// interrupt request.  This may mean multiple calls to recv.
			ssize_t bytes = safe_recv(&curr_wfile_fd, wfile_data + recv_len, WFILE_DATA_SZ - recv_len, 0);
			if (bytes > 0) recv_len += bytes;

			if (recv_len == WFILE_DATA_SZ)
			{
                // Trigger the interrupt to tell the CPU someone's talking to it via FFS
                cs->interrupt_request |= CPU_INTERRUPT_FFS_WRITE;
                waiting_for_data = 0;
				recv_len = 0;
			}
		}
    }

    // From the perspective of Qemu, the peripheral device is reading from rfile.  The peripheral
    // will still send packets on the rfile socket, however, to both do a read request and
    // acknowledge that data has been received.  The behavior of the rfile socket is as follows:
    //  1) Any time a "read request" packet is recv'd, immediately copy the contents of rfile_data
    //     onto the socket.
    //  2) When an "acknowledgement" packet is recv'd, clear the contents of rfile_data, and put the
    //     acknowledgement packet in rfile_ack.  There are two kinds of acknowledgements, success
    //     and failure, but these are handled at the OS level, not the hardware level.  When an ack
    //     packet is received, trigger an interrupt for the OS to handle it.
    if (curr_rfile_fd != -1)
    {
        uint8_t rfile_request;
        ssize_t bytes = safe_recv(&curr_rfile_fd, &rfile_request, 1, 0);
        if (bytes > 0) 
        {
            if (rfile_request == DATA_REQUEST)
            {
                bytes = send(curr_rfile_fd, rfile_data, RFILE_DATA_SZ, 0);
                if (bytes == -1) perror("rfile send");
            }
            else 
            {
                // If we're acknowledging a packet, need to recv the remaining bytes into
                // the ack buffer.  
                rfile_ack[0] = rfile_request;
                safe_recv(&curr_rfile_fd, rfile_ack + 1, RFILE_ACK_SZ - 1, 0);
                cs->interrupt_request |= CPU_INTERRUPT_FFS_ACK;
            }
        }   
    }

    // Qemu's timer are one-shot timers, so we need to rearm the timer after
    // every time it fires.  The timer needs to be sufficiently large so that the
    // main CPU threads have a chance to run.  Qemu will complain if it's too
    // small, and empirically a step size of around 100000 clock cycles seems to
    // be about right
    next = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) + 100000;
    timer_mod(ffs_socket_timer, next);
}

void orp_init_ffs_mmio(void* opaque, MemoryRegion* address_space)
{
    // Set up the memory region
    memory_region_init_io(&ffs_mmio_region, NULL, &ffs_ops, opaque, "ffs", FFS_WIDTH);
    memory_region_add_subregion(address_space, FFS_ADDR, &ffs_mmio_region);
    ffs_reset();

    // Set up the network connections
    ffs_init_sockets(&rfile_sock_fd, &wfile_sock_fd);
    ffs_socket_timer = timer_new_ns(QEMU_CLOCK_VIRTUAL, &ffs_socket_timer_cb, (OpenRISCCPU*)opaque); 
    uint64_t next = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) + 1000;
    timer_mod(ffs_socket_timer, next);
}
