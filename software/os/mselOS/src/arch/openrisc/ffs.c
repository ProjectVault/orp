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
#include <msel/stdc.h>

#include "driver/ffs_session.h"

#include "arch.h"
#include "mmio.h"

static uint8_t curr_wfile_status = FFS_CHANNEL_READY;

// Write a packet to wfile
msel_status arch_ffs_wfile_read(ffs_packet_t *pkt)
{
    msel_memcpy(&(pkt->session), FFS_RECV_DATA_ADDR, 2);
    msel_memcpy(&(pkt->nonce), FFS_RECV_DATA_ADDR + 2, 1);
    msel_memcpy(pkt->data, FFS_RECV_DATA_ADDR + FFS_HDR_SIZE, FFS_DATA_SIZE);
    return MSEL_OK;
}

void arch_ffs_wfile_set_status(uint8_t status, uint8_t nonce)
{
    uint8_t *ack = FFS_RECV_ACK_ADDR;
    ack[1] = nonce;
    ack[0] = status;
    curr_wfile_status = status;
}

uint8_t arch_ffs_wfile_get_status()
{
    return curr_wfile_status;
}

msel_status arch_ffs_rfile_write(ffs_packet_t *pkt)
{
    static uint8_t nonce = 0x01;

    // Check to see if the peripheral has read the last packet
    if (msel_ffs_rfile_get_status() != FFS_CHANNEL_LAST_SUCC &&
        msel_ffs_rfile_get_status() != FFS_CHANNEL_READY) 
        return MSEL_EAGAIN;

    nonce++;
    if (nonce == 0x00)
        nonce = 0x01;
    uint32_t hdr = 0;

	// Set the header/nonce to 0 while the data write is occuring; the android
	// application should not read the data until the header has been filled in
	// and the nonce matches
	msel_memcpy(FFS_SEND_DATA_ADDR, &hdr, FFS_HDR_SIZE);

	// Copy the data over
    msel_memcpy(FFS_SEND_DATA_ADDR + FFS_HDR_SIZE, pkt->data, FFS_DATA_SIZE);

	// Now set the header
    hdr = (pkt->session << 16) | ((0xff & (uint32_t)nonce) << 8);
    msel_memcpy(FFS_SEND_DATA_ADDR, &hdr, FFS_HDR_SIZE);

    return MSEL_OK;
}


void arch_ffs_rfile_clear()
{
    msel_memset(FFS_SEND_DATA_ADDR, 0, FFS_HDR_SIZE + FFS_DATA_SIZE);
}

extern int hasSeenReadAck;


uint8_t arch_ffs_rfile_get_status()
{
    uint8_t ret = hasSeenReadAck ? (*FFS_SEND_ACK_ADDR) : FFS_CHANNEL_READY;
    return ret;
}
