/** @file ffs_session.h
 *
 *  This file contains declarations for the faux filesystem session management functions
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

#ifndef _FFS_SESSION_H
#define _FFS_SESSION_H

#include <msel/ffs.h>
#include <stdint.h>

/** @defgroup ffs_session Faux Filesystem Session Management

    Any data sent to and from sessions (applications) in mselOS get routed through the FFS
    session manager.  The session manager maintains several circular buffers for incoming
    and outgoing data.  All outgoing data enters one single circular buffer; whereas incoming
    data is routed to a circular buffer corresponding to the particular session indicated
    in the packet header.

    Applications are assigned session IDs from 1..MAX_NUM_SESSIONS, and each one has a
    circular buffer queue of MAX_QUEUE_SIZE messages.  Session ID 0 corresponds to the 
    session manager, which is in charge of starting new sessions and killing existing 
    sessions.

    (Note: Session 0, the session manager, has a special outgoing queue for messages.  
    Outbound messages from S0 pre-empt any other outbound messages)

 *  @{
 */


/** @anchor ffs_status
 *  @name Faux Filesystem Return Values
 *  @{
 */

/** @brief The faux filesystem channel is ready for data */
#define FFS_CHANNEL_READY       0x10    

/** @brief The last faux filesystem request failed due to lack
 *  of resources or other OS failure
 */
#define FFS_CHANNEL_LAST_FAIL   0x11    

/** @brief The last faux filesystem request succeeded */
#define FFS_CHANNEL_LAST_SUCC   0x12    

/** @brief The last faux filesystem request should be retried 
 *  due to a full message queue
 */
#define FFS_CHANNEL_LAST_RETRY  0x13    

/** @brief The last faux filesystem packet was malformed */
#define FFS_CHANNEL_LAST_EINPUT 0x14    
/** @} */


/** @brief Send a message from an application to the Android device.
 *  Call this function with the MSEL_SVC_FFS_SESSION_SEND syscall.
 *
 *  @param pkt A pointer to the data packet to be transmitted
 *  @return MSEL status value:
 *    - MSEL_OK on successful operation
 *    - MSEL_ERESOURCE if there's not enough space in the RFILE queue
 */
msel_status msel_ffs_session_send(ffs_packet_t *pkt);


/** @brief Read an incoming message from the Android device to an application.
 *  Call this function with the MSEL_SVC_FFS_SESSION_RECV syscall.
 *
 *  @param pkt A pointer to the data to be given to the application
 *  @return MSEL status value:
 *    - MSEL_OK on successful operation
 *    - MSEL_ERESOURCE if the session ID is invalid and there's data to read 
 */
msel_status msel_ffs_session_recv(ffs_packet_t *pkt);

/** @brief Advance the RFILE message queue
 *
 *  This is called when the Android device signals that it's ready for another
 *  packet; this function should only be called from an interrupt context
 */
void msel_rfile_step_queue();

/** @brief Read the next message from the Android device
 *
 *  This function serves two purposes: writes to session 0 call the session manager,
 *  and either start a new task or kill the old task.  Writes to any other session
 *  just transfer the data to the application's message queue.
 *
 *  This function should only be called from an interrupt context
 */
void msel_wfile_get_packet();

/** @brief Initialize all of the message queues */
void msel_init_ffs_queues(void);

/** @} */

#endif // _FFS_SESSION_H
