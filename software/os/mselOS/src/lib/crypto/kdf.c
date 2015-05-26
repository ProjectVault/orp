/** @file kdf.c
 *
 *  This file contains the key deriviation function for mselOS
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

#include <stdint.h>
#include <msel/stdc.h>
#include <msel/malloc.h>
#include <crypto/kdf.h>
#include <crypto/sha2.h>

/** @brief Key Derivation function for mselOS
 *
 *  @param master The device master key
 *  @param l_master The length of the device master key
 *  @param protocol The "protocol" string/key
 *  @param l_protocol The length of the protocol string
 *  @param nonce A nonce or session key
 *  @param out The resulting key after mixing
 */
void kdf_getkey( const uint8_t *master, uint32_t l_master
               , const uint8_t *protocol, uint32_t l_protocol
               , const uint8_t *nonce
               , uint8_t *out
               )
{
  /*
  assert(NULL != master);
  assert(NULL != protocol);
  assert(NULL != nonce);
  assert(l_master > 0);
  assert(l_protocol > 0);
  assert(NULL != out);
  */

  uint8_t *table = msel_malloc(KDF_NUM_ITER * SHA256_OUTPUT_LEN);
  uint8_t curr[SHA256_OUTPUT_LEN];
  uint32_t i = 0;

  /* mix the 3 input keys */
  sha256_hash(master, l_master, &table[0 * SHA256_OUTPUT_LEN]);
  sha256_hash(protocol, l_protocol, &table[1 * SHA256_OUTPUT_LEN]);
  sha256_hash(nonce, SHA256_OUTPUT_LEN, &table[2 * SHA256_OUTPUT_LEN]);

  for (i = 0; i < SHA256_OUTPUT_LEN; i++)
    table[0*SHA256_OUTPUT_LEN+i] ^= table[1*SHA256_OUTPUT_LEN+i] ^ table[2*SHA256_OUTPUT_LEN+i];
  /* table[0] is now initialized with the mixed input key */

  /* generate the mixing table, noting that table[0] is already initialized */
  for (i = 1; i < KDF_NUM_ITER; i++)
    sha256_hash(&table[(i-1)*SHA256_OUTPUT_LEN], SHA256_OUTPUT_LEN, &table[i*SHA256_OUTPUT_LEN]);

  /* grab the output-key seed from the table */
  msel_memcpy(curr, &table[(KDF_NUM_ITER-1)*SHA256_OUTPUT_LEN], SHA256_OUTPUT_LEN);

  /* generate the output-key from the seed and table */
  for (i = 0; i < KDF_NUM_ITER; i++) {
    uint32_t j = 0;
    uint8_t idx = 0;
    sha256_hash(curr, SHA256_OUTPUT_LEN, curr);
    idx = (curr[1] | curr[0]) & (KDF_NUM_ITER - 1);
    for (j = 0; j < SHA256_OUTPUT_LEN; j++)
      curr[j] ^= table[idx*SHA256_OUTPUT_LEN+j];
  }

  /* return the generated key */
  msel_memcpy(out, curr, SHA256_OUTPUT_LEN);

  msel_memset(table, 0, KDF_NUM_ITER * SHA256_OUTPUT_LEN);
  msel_memset(curr, 0, SHA256_OUTPUT_LEN);
  msel_free(table);
}

