/** @file mmio.h

 * defines address regions for all the supported MMIO devices
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

#ifndef _ARCH_OPENRISC_MMIO_H_
#define _ARCH_OPENRISC_MMIO_H_

/** @defgroup mmio Memory-Mapped I/O devices
 * @{
 */

/** @defgroup mmio_addr MMIO Device Addresses
 *  Starting addresses for each MMIO region for the ORP device
 *  @{
 */

/** @brief Start of UART MMIO region */
#define UART_ADDR 0x90000000

/** @brief Start of TRNG MMIO region */
#define TRNG_ADDR 0x92000000

/** @brief Start of AES MMIO region */
#define AES_ADDR  0x93000000

/** @brief Start of SHA-256 MMIO region */
#define SHA_ADDR  0x94000000

/** @brief Start of EC core MMIO region */
#define ECC_ADDR  0x95000000

/** @brief Start of faux filesystem MMIO region */
#define FFS_ADDR  0x98000000

/** @} */


/** @defgroup aes_addr AES Core Locations 
 *  @{
 */
/** @brief AES key location */
#define AES_KEY_ADDR  (uint8_t*)0x93000000

/** @brief AES input data location */
#define AES_DIN_ADDR  (uint8_t*)0x93000020

/** @brief AES output data location */
#define AES_DOUT_ADDR (uint8_t*)0x93000030

/** @brief AES control register location */
#define AES_CTRL_ADDR (uint32_t*)0x93000040
/** @} */

/** @defgroup sha_addr SHA-256 Core Locations
 *  @{
 */
/** @brief SHA-256 initialization vector/output hash location */
#define SHA_IV_ADDR   (uint8_t*)0x94000000

/** @brief SHA-256 data input location */
#define SHA_DIN_ADDR  (uint8_t*)0x94000020

/** @brief SHA-256 control register location */
#define SHA_CTRL_ADDR (uint32_t*)0x94000060
/** @} */

/** @defgroup ecc_addr ECC Core Locations
 *  @{
 */
/** @brief Scalar value input location for point-scalar multiply */
#define ECC_SCALAR_ADDR (uint8_t*)0x95000000

/** @brief Point input/output location for point-scalar multiply */
#define ECC_POINT_ADDR  (uint8_t*)0x95000080

/** @brief ECC control register location */
#define ECC_CTRL_ADDR   (uint32_t*)0x95000100
/** @} */

/** @defgroup ffs_addr Faux Filesystem Locations
 *  @{
 */
/** @brief WFILE data location */
#define FFS_RECV_DATA_ADDR (uint8_t*)0x98000000

/** @brief RFILE data location */
#define FFS_SEND_DATA_ADDR (uint8_t*)0x98000800

/** @brief WFILE acknowledge location */
#define FFS_RECV_ACK_ADDR  (uint8_t*)0x98001000

/** @brief RFILE acknowledge location */
#define FFS_SEND_ACK_ADDR  (uint8_t*)0x98001010

/** @brief Faux filesystem control register location */
#define FFS_CTRL_ADDR      (uint32_t*)0x98001020

/** @} */

/** @} */

#endif
