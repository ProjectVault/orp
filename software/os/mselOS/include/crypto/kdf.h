/** @file kdf.h
 *
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

#ifndef _KDF_H_
#define _KDF_H_

#include <stdint.h>
#include <crypto/sha2.h>

/** @ingroup crypto
 *  @{
 */

/** @defgroup kdf Key derivation routines
 *  @{
 */


#define KDF_NUM_ITER ((uint32_t) 16)

/** @brief Generate a 32-byte key, suitable for use with AES.
 *
 * @param master Master input key. Assumed non-null.
 * @param l_master Length of the master input key. Assumed
 *        `l_master > 0`.
 * @param protocol Protocol input key. Assumed non-null.
 * @param l_protocol Length of the protocol input key. Assumed
 *        `l_protocol > 0`.
 * @param nonce Session nonce. Assumed non-null with length
 *        SHA256_OUTPUT_LEN.
 * @param out Pointer to output buffer for generated key.
 *        Assumed non-null with length SHA256_OUTPUT_LEN.
 */
void kdf_getkey( const uint8_t *master, uint32_t l_master
               , const uint8_t *protocol, uint32_t l_protocol
               , const uint8_t *nonce
               , uint8_t *out
               );

/** @} */

/** @} */

#endif /* _KDF_H_ */
