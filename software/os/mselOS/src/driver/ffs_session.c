/** @file ffs_session.c
 *  
 *  This file contains all of the session management functions for the faux filesystem (FFS).
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

#include <msel/endpoints.h>
#include <msel/syscalls.h>
#include <msel/malloc.h>
#include <msel/stdc.h>

#include "os/task.h"

#include "ffs_session.h"
#include "ffs_driver.h"

/** @addtogroup ffs_session
 *  @{
 */

/** @name Internal Faux Filesystem Parameters
 *  @{
 */

/** @brief The maximum number of messages a session queue can hold */
#define MAX_QUEUE_SIZE 2

/** @brief The maximum number of sessions that can exist at a time */
#define MAX_NUM_SESSIONS 5

/** @brief Instruct the session manager to start a new application/session */
#define CMD_START_SESSION 0x1

/** @brief Instruct the session manager to kill an existing session */
#define CMD_KILL_SESSION  0x2

/** @brief Number of bytes for the session manager commands */
#define S0_CMD_SIZE 1

/** @} */

/** @brief Message queue for a FFS session */
typedef struct ffs_queue_s
{
    ffs_packet_t data[MAX_QUEUE_SIZE];

    // The queue is stored as a circular array
    uint8_t size;
    ffs_packet_t *start;
    ffs_packet_t *end;

    // Data about the task corresponing to this queue
    uint16_t task_id;
    const uint8_t *endpoint;
    uint16_t port;
    
    // Whether the session is currently in use
    uint8_t active;
} ffs_queue_t;

/** @brief Special outgoing queue for session 0 (the session manager) */
struct
{
    uint32_t data[MAX_QUEUE_SIZE];
    uint8_t size;
    uint32_t *start;
    uint32_t *end;
} s0_rq;

/** @} */

#define CBUF_ADD(buf) { \
    ++buf.end; ++buf.size; \
    if (buf.end >= buf.data + MAX_QUEUE_SIZE) \
        buf.end = buf.data; \
}

#define CBUF_REM(buf) { \
    --buf.size; ++buf.start; \
     if (buf.start >= buf.data + MAX_QUEUE_SIZE) \
         buf.start = buf.data; \
}

static int rfile_empty;    // Is there anything in RFILE?
static ffs_queue_t rq;     // The outbound (RFILE) queue
static ffs_queue_t wq[MAX_NUM_SESSIONS];   // The incoming (WFILE) queues 

// 1-indexed array for convenient access without session-ID translation issues
// NEVER access wq_1[0]!!
static ffs_queue_t *wq_1 = wq - 1;

// Data is read into s0 before being routed to the appropriate session
static ffs_packet_t s0;

static uint16_t num_sessions = 0;
static uint16_t retry_session_id = MAX_NUM_SESSIONS + 1;

/** @brief If the last message was "retry", signal ready for a new write */
static void clear_retry_id()
{
    if (msel_ffs_wfile_get_status() == FFS_CHANNEL_LAST_RETRY)
    {
        retry_session_id = MAX_NUM_SESSIONS + 1;
        msel_ffs_wfile_set_status(FFS_CHANNEL_READY, 0x00);
    }
}

/** @brief Set up a new session queue for receiving messages
 
    Only call this function if there is room for a new session
    Otherwise, undefined behavior results

    @param task_id The application ID for the new task in mselOS
    @param endpoint A 32-byte array corresponding to the application endpoing
    @param port The "port" or "socket" to use for communication with the application

    @return The session ID of the newly-created session
 */
static uint16_t init_session(uint16_t task_id, const uint8_t* endpoint, uint16_t port)
{
    // Compute the session number, and initialize the new session
    uint16_t sid;
    for (sid = 1; sid <= MAX_NUM_SESSIONS; ++sid)
        if (!wq_1[sid].active) break;

    wq_1[sid].start = wq_1[sid].data;
    wq_1[sid].end = wq_1[sid].data;
    wq_1[sid].size = 0;
    wq_1[sid].task_id = task_id;
    wq_1[sid].endpoint = endpoint;
    wq_1[sid].port = port;
    wq_1[sid].active = 1;

    num_sessions++;
    return sid;     
}

/** @brief Force-quit a specified session

    @param sid The session ID which should be halted.
      Undefined behavior results if sid does not correspond to a valid session
 */
static void end_session(uint16_t sid)
{
    msel_task_force_kill(wq_1[sid].task_id, "Force quit by Android");
    msel_memset(&(wq_1[sid]), 0, sizeof(ffs_queue_t));
    --num_sessions;

    if (sid == retry_session_id) clear_retry_id();
}

msel_status msel_ffs_session_send(ffs_packet_t *pkt)
{
    // Make sure the task isn't trying to impersonate an invalid session 
    uint8_t task_id = msel_active_task_num;
    for (pkt->session = 1; pkt->session <= MAX_NUM_SESSIONS; ++(pkt->session))
        if (wq_1[pkt->session].active && wq_1[pkt->session].task_id == task_id) break;

    // Copy the message to RFILE
    if (pkt->session > 0 && pkt->session <= MAX_NUM_SESSIONS && rq.size < MAX_QUEUE_SIZE)
    {
        msel_memcpy(rq.end, pkt, sizeof(ffs_packet_t));
        CBUF_ADD(rq);

        // Don't have to acknowledge before receiving first packet
        if (rfile_empty) msel_rfile_step_queue();

        return MSEL_OK;
    }
    else return MSEL_ERESOURCE;
}

msel_status msel_ffs_session_recv(ffs_packet_t *pkt)
{
    // Find out which queue to get the next message from
    uint8_t sid; uint8_t task_id = msel_active_task_num;
    for (sid = 1; sid <= MAX_NUM_SESSIONS; ++sid)
        if (wq_1[sid].active && wq_1[sid].task_id == task_id) break;

    // Copy the message and advance the array index (the data's now in the task's
    // hands, if it throws it away there's no getting it back).
    if (sid > 0 && sid <= MAX_NUM_SESSIONS && wq_1[sid].size > 0)
    {
        msel_memcpy(pkt, wq_1[sid].start, sizeof(ffs_packet_t));

        // If the peripheral was told to retry, now we can say that we're ready
        if (sid == retry_session_id) 
            clear_retry_id();

        CBUF_REM(wq_1[sid]);
        return MSEL_OK;
    }
    else return MSEL_ERESOURCE;
}

void msel_rfile_step_queue()
{
    rfile_empty = 0;

    // Messages from session 0 pre-empt anything in rq
    if (s0_rq.size > 0)
    {
        msel_memset(&s0, 0, sizeof(ffs_packet_t));
        s0.data[0] = (*s0_rq.start >> 8);
        s0.data[1] = (*s0_rq.start);
        if (msel_ffs_rfile_write(&s0) == MSEL_OK)
            CBUF_REM(s0_rq);
    }
    else if (rq.size > 0)
    {
        // Remember, the host writes to rfile and reads from wfile
        //
        // Only advance the queue if the peripheral is ready for the next packet
        if (msel_ffs_rfile_write(rq.start) == MSEL_OK)
            CBUF_REM(rq);
    }
    else
    {
        rfile_empty = 1;
        msel_ffs_rfile_clear();
    }
}

void msel_wfile_get_packet()
{
    // Remember, the host writes to rfile and reads from wfile
    msel_ffs_wfile_read(&s0);
    uint16_t sid = s0.session;
    uint16_t status = FFS_CHANNEL_LAST_SUCC;
    uint8_t in_nonce = s0.nonce;

    msel_ffs_wfile_set_status(FFS_CHANNEL_READY, in_nonce);

    // If the session ID is 0, then pass the packet data to the session manager
    if (sid == 0)
    {
        uint8_t ctrl;

        // Check the ctrl byte for our command -- first byte of the data stream
        ctrl = s0.data[0];

        // Start up a new session
        if (ctrl == CMD_START_SESSION)
        {
            // The protocol requires that we return the session ID on the rfile channel,
            // so if this buffer is full then we need to try again later
            if (s0_rq.size >= MAX_QUEUE_SIZE)
                { status = FFS_CHANNEL_LAST_RETRY; goto cleanup; }

            // Make sure there's an available session
            else if (num_sessions >= MAX_NUM_SESSIONS)
                { status = FFS_CHANNEL_LAST_FAIL; goto cleanup; }

            else // Try to create the session
            {
                uint8_t task_id; uint16_t port; 
                const uint8_t* endpoint = NULL;
                void (*task_fn)(void *arg, const size_t arg_sz) = NULL;

                // Check that the endpoint is valid
                get_task(&endpoint, &task_fn, &port, s0.data + S0_CMD_SIZE);
                if (endpoint == NULL || task_fn == NULL)
                    { status = FFS_CHANNEL_LAST_FAIL; goto cleanup; }

                // Try to create the task
                if (msel_task_create(task_fn, NULL, 0, &task_id) != MSEL_OK)
                    { status = FFS_CHANNEL_LAST_FAIL; goto cleanup; }

                sid = init_session(task_id, endpoint, port);

                // Send a response from session 0 announcing the session ID
                *s0_rq.end = sid;
                CBUF_ADD(s0_rq);
                status = FFS_CHANNEL_LAST_SUCC;

                // Don't have to acknowledge before receiving first packet
                if (rfile_empty) msel_rfile_step_queue();

                goto cleanup;
            }
        }

        // Force-quit a session
        else if (ctrl == CMD_KILL_SESSION)
        {
            uint16_t sid = (s0.data[S0_CMD_SIZE] << 8) | (s0.data[S0_CMD_SIZE + 1]);
            if (sid > 0 && sid <= MAX_NUM_SESSIONS && wq_1[sid].active)
                { end_session(sid); goto cleanup; }
            else { status = FFS_CHANNEL_LAST_FAIL; goto cleanup; }
        }
        
        else { status = FFS_CHANNEL_LAST_EINPUT; goto cleanup; }

        goto cleanup;
    }

    // Otherwise, route the packet to the appropriate session
    else 
    {
        // Invalid session ID
        if (sid == 0 || sid > MAX_NUM_SESSIONS || !wq_1[sid].active)
            { status = FFS_CHANNEL_LAST_FAIL; goto cleanup; }

        // Too much data in the buffer
        else if (wq_1[sid].size >= MAX_QUEUE_SIZE) 
        {
            retry_session_id = sid;
            status = FFS_CHANNEL_LAST_RETRY;
            goto cleanup;
        }

        // Copy the data to the buffer
        else
        {
            msel_memcpy(wq_1[sid].end, &s0, sizeof(ffs_packet_t));

            CBUF_ADD(wq_1[sid]);
            goto cleanup;
        }

        goto cleanup;
    }

cleanup:
    msel_ffs_wfile_set_status(status, in_nonce);
}

int hasSeenReadAck;

void msel_init_ffs_queues()
{
    msel_memset(&rq, 0, sizeof(ffs_queue_t));
    rq.start = rq.data; rq.end = rq.data;
    s0_rq.start = s0_rq.data;
    s0_rq.end = s0_rq.data;
    s0_rq.size = 0;
    rfile_empty = 1;
    hasSeenReadAck = 0;
    
    msel_memset(wq, 0, sizeof(ffs_queue_t) * MAX_NUM_SESSIONS);
    msel_ffs_wfile_set_status(FFS_CHANNEL_READY, 0x00);
    msel_ffs_rfile_clear();
}
