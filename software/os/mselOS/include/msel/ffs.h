/** @file ffs.h
 *
 *  The Faux Filesystem packet interface.
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

#ifndef _MSEL_FFS_H_
#define _MSEL_FFS_H_

#include <stdlib.h>
#include <stdint.h>
#include "msel.h"

/** @addtogroup ffs_session
 *  @{
 */

/** @name Faux Filesystem Packet Structure
 *  @{
 */

/** @brief Number of bytes for the header of a faux filesystem packet */
#define FFS_HDR_SIZE 4

/** @brief Number of bytes for the contents of a faux filesystem packet */
#define FFS_DATA_SIZE 2044 

/** @} */

/** @brief A faux filesystem message packet */
typedef struct ffs_packet_s
{
    /** @brief Session ID that the packet should be routed to */
    uint16_t session;

    /** @brief Nonce value to prevent packet loss */
    uint8_t nonce;

    /** @brief unused */
    uint8_t reserved;

    /** @brief Packet data */
    uint8_t data[FFS_DATA_SIZE];
} ffs_packet_t;

/** @} */

#endif
