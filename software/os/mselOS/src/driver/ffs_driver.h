/** @file ffs_driver.h
 *
 *  Declarations for the Faux Filesystem packet interface.
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

#ifndef _MSEL_FFS_DRIVER_H_
#define _MSEL_FFS_DRIVER_H_

#include <stdlib.h>
#include <stdint.h>
#include <msel.h>
#include <msel/ffs.h>

/** @addtogroup driver
 *  @{
 */

/** @defgroup ffs_driver Faux Filesystem Driver
 *  @{
 */

/** @brief Read data from WFILE that has been written by the Android device.
 *  This function should be called from an interrupt context
 *  
 *  @param[out] pkt Packet data that was written to WFILE
 *  @return MSEL status value:
 *    - MSEL_OK for successful operation
 */
msel_status msel_ffs_wfile_read(ffs_packet_t *pkt);

/** @brief Acknowledge receipt of the last packet written to WFILE
 *
 *  @param status The status (success or failure) of the packet transmission
 *  @param nonce  The message ID of the packet in question
 */
void msel_ffs_wfile_set_status(uint8_t status, uint8_t nonce);

/** @brief Return the last-set acknowledgement status for WFILE 
 *
 *  @return the acknowledge status for WFILE
 */
uint8_t msel_ffs_wfile_get_status();

/** @brief Write data to RFILE for the Android device to read
 *  
 *  @param pkt Packet data to write to RFILE
 *  @return MSEL status value:
 *    - MSEL_OK for successful operation
 *    - MSEL_EAGAIN if the Android device isn't ready
 */
msel_status msel_ffs_rfile_write(ffs_packet_t *pkt);

/** @brief Clear data written to the RFILE buffer */
void msel_ffs_rfile_clear();

/** @brief Get the acknowledgement status for RFILE sent from the Android device 
 *
 *  @return the acknowledge status for RFILE
 */
uint8_t msel_ffs_rfile_get_status();

/** @} */

/** @} */

#endif
