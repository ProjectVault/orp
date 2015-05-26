/* Copyright 2015, Google Inc.

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


// orpio.c

#include "orpio.h"
#include "tidl.h"
#include "xtsProto.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>

#define CHANNEL_OK 0
#define CHANNEL_ERROR 1
#define CHANNEL_RETRY 2
#define CHANNEL_WAIT 3
#define CHANNEL_FLUSH_CACHE 4

static int rfile_fd;
static int wfile_fd;

static void* databuf;
static uint8_t wnonce = 0xc1;
static uint8_t rnonce = 0x00;
static uint8_t empty_hdr[ORP_HDR_LEN] = {0};

static const struct timespec wait_time = { .tv_sec = 0, .tv_nsec = 250000000L, };

static const struct timespec cache_flush_time = { .tv_sec = 0, .tv_nsec = 500000000L, };

// Check the status of wfile
uint8_t orp_dev_wstatus()
{
    ssize_t nbytes = pread(wfile_fd, databuf, ORP_PKT_LEN, 0);
    if (nbytes < 0) { perror("dev_wstatus error"); return CHANNEL_ERROR; }

    // Format is [status|nonce|ignored|cache bit]
    uint8_t status = ((uint8_t*)databuf)[0];
    uint8_t snonce = ((uint8_t*)databuf)[1];

    if (status == 0x10 && wnonce == 0x00)
    {
        fprintf(stderr, "Board in initial state: writes are OK\n");
        return CHANNEL_OK;
    }

    // Make sure the nonces match
    if (snonce == wnonce)
    {
        switch (status)
        {
            case 0x10: 
                fprintf(stderr, "Still processing write: try again\n");
                return CHANNEL_WAIT;
            case 0x12: 
                fprintf(stderr, "received OK\n");
                return CHANNEL_OK;
            case 0x11:
            case 0x14: 
                fprintf(stderr, "received ERROR\n");
                return CHANNEL_ERROR;
            case 0x13: 
                fprintf(stderr, "received RETRY\n");
                return CHANNEL_RETRY;
            default:
                fprintf(stderr, "Unknown status code recieved: 0x%x\n", status);
                return CHANNEL_ERROR;

        }
    }

    // If the nonces don't match, a message may have gotten dropped, or it's still processing
    else 
    {
        fprintf(stderr, "nonces don't match, waiting %x %x\n", snonce, wnonce);
        return CHANNEL_WAIT;
    }

    fprintf(stderr, "unknown error occurred");
    return CHANNEL_ERROR;
}

// Write a message to the device
int orp_dev_write(uint16_t session_id, const uint8_t* data)
{
    ++wnonce; if (wnonce == 0) wnonce = 0x01;
    fprintf(stderr, "writing message %x\n", wnonce);

    // Serialize a packet and write it to WFILE
    uint32_t pos = 0;
    uint16_serialize(databuf, ORP_PKT_LEN, &pos, session_id);
    uint8_serialize(databuf, ORP_PKT_LEN, &pos, wnonce);
    uint8_serialize(databuf, ORP_PKT_LEN, &pos, 0x0);  // Cache bit 
    memcpy(databuf + pos, data, ORP_DATA_LEN);

    ssize_t nbytes = pwrite(wfile_fd, databuf, ORP_PKT_LEN, 0); 
    if (nbytes < 0) { perror("dev_write failed"); return -1; }
    syncfs(wfile_fd); // Sync to ensure write happens

    // Wait for an acknowledgement from the device
    int waitTimes = 0;
    uint8_t status;
    while (1)
    {
        // Check the status
        status = orp_dev_wstatus();

        // If we've gotten the "WAIT" message too many times,
        // something may have gotten dropped; try sending again
        if (waitTimes >= 8) status = CHANNEL_RETRY;
        switch (status)
        {
            case CHANNEL_OK: 
                return 0;
            case CHANNEL_ERROR: 
                return -1;
            case CHANNEL_RETRY: 
                fprintf(stderr, "retransmitting message\n");
                nanosleep(&wait_time, NULL);
                return orp_dev_write(session_id, data);
            case CHANNEL_WAIT:
                nanosleep(&wait_time, NULL);
                ++waitTimes;
                break;
        }
    }
}

// Acknowledge receipt of the last message from the device
int orp_dev_ack(uint8_t status)
{
    memset(databuf, 0, ORP_PKT_LEN);
    switch (status)
    {
        case CHANNEL_OK: ((uint8_t*)databuf)[0] = 0x10; break;
        default:         ((uint8_t*)databuf)[0] = 0x11; break;
    }
    ssize_t nbytes = pwrite(rfile_fd, databuf, ORP_PKT_LEN, 0);
    if (nbytes < 0) { perror("dev_rstatus failed"); return -1; }
    syncfs(rfile_fd);   // Make sure the write really happened

    // We need to sleep here to give the device time to drop back
    // into user mode, where it will accept interrupts again
    //
    // NOTE: we don't need this sleep after orp_dev_write because we don't
    // move on until the message has been acknowledged, at which point the
    // device will be in user mode
    nanosleep(&wait_time, NULL);
    return 0;
}

int orp_dev_read(uint16_t* session_id, uint8_t* data)
{
    // Read the packet
    while (1)
    {
        ssize_t nbytes = pread(rfile_fd, databuf, ORP_PKT_LEN, 0);
        if (nbytes < 0) { perror("dev_read failed"); return -1; }
        if (memcmp(databuf, empty_hdr, ORP_HDR_LEN) == 0)
        {
            fprintf(stderr, "waiting for response\n");
            nanosleep(&wait_time, NULL);
        }
        else break;
    } 

    // Parse the response
    uint32_t pos = 0; uint8_t nonce;
    uint16_deserialize(databuf, ORP_PKT_LEN, &pos, session_id);
    uint8_deserialize(databuf, ORP_PKT_LEN, &pos, &nonce);

    // If rnonce == nonce, then an acknowledgement got dropped somehwere
    // and we need to retry
    if (rnonce == nonce)
    {
        fprintf(stderr, "retrying read ack\n");
        if (orp_dev_ack(CHANNEL_OK) != 0) return -1;
        nanosleep(&wait_time, NULL);
        return orp_dev_read(session_id, data);
    }

    rnonce = nonce;
    uint8_t dummy; uint8_deserialize(databuf, ORP_PKT_LEN, &pos, &dummy);
    memcpy(data, databuf + pos, ORP_DATA_LEN);

    fprintf(stderr, "sending read ack\n");
    if (orp_dev_ack(CHANNEL_OK) != 0) return -1;
    else return 0;
}

int orp_dev_connect(const char* dirpath, const char* key)
{
    posix_memalign(&databuf, sysconf(_SC_PAGESIZE), ORP_PKT_LEN);

    // Set up the paths to RFILE and WFILE
    int len = strlen(dirpath);
    char* path = malloc(len + 6);
    strcpy(path, dirpath);
    strcpy(path + len, "/RFILE");

    rfile_fd = open(path, O_RDWR | O_SYNC | O_DIRECT);
    if (rfile_fd == -1)
    {
        perror("error opening rfile");
        return -1;
    }
    path[len+1] = 'W';
    wfile_fd = open(path, O_RDWR |  O_SYNC | O_DIRECT);
    if (wfile_fd == -1)
    {
        perror("error opening wfile");
        return -1;
    }

    free(path);

    uint8_t buf[ORP_DATA_LEN];
    memset(buf, 0, ORP_DATA_LEN);
    
    buf[0] = 0x1;   // Start a new session
    buf[1] = 0x78;  // Endpoint: "xts\n"
    buf[2] = 0x74;
    buf[3] = 0x73;
    buf[4] = 0xa;

    orp_dev_write(0, buf);
    uint16_t session_id;

    // Get the response from the task manager with the session ID of the new task
    orp_dev_read(&session_id, buf);
    uint16_t new_session_id;
    uint32_t pos = 0;
    uint16_deserialize(buf, ORP_DATA_LEN, &pos, &new_session_id);
    printf("session id: %d\n", new_session_id);

    // Initialize the new task with our algorithm and key
    memset(buf, 0, ORP_DATA_LEN); pos = 0;
    XtsAlgo_serialize(buf, ORP_DATA_LEN, &pos, XTS_128);
    uint64_serialize(buf, ORP_DATA_LEN, &pos, ORP_BLOCK_SIZE);
    string_serialize(buf, ORP_DATA_LEN, &pos, key);

    // Get the response
    orp_dev_write(new_session_id, buf);
    orp_dev_read(&session_id, buf);

    pos = 0;
    enum XtsResponse response;
    XtsResponse_deserialize(buf, ORP_DATA_LEN, &pos, &response);

    if (response != XTS_OK)
        return -1;

    // Return the session ID of the encryptor task
    return new_session_id;
}

