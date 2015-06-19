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


#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <msel.h>
#include <msel/syscalls.h>
#include <msel/malloc.h>
#include <msel/stdc.h>
#include <msel/ffs.h>

#include <crypto/prng.h>
#include <crypto/kdf.h>
#include <crypto/sha2.h>
#include <crypto/aes_gcm.h>
#include <crypto/ecc.h>

#include <bulkencryptionProto.h>


/* 
 * Bulk Encryption task: 
 *   Purpose: supports the Bulk Encryption demo
 *   Endpoint "bulkencryption\n\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
 */
void bulkencryption_task(void *arg, const size_t arg_sz);
const uint8_t bulkencryption_endpoint[32] =
  { 0x62, 0x75, 0x6C, 0x6B, 0x65, 0x6E, 0x63, 0x72,
    0x79, 0x70, 0x74, 0x69, 0x6F, 0x6E, 0xa,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,
    0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0,  0x0
  };



/*
 * Session state
 *
 */

typedef struct ecc_pair_s {
  uint8_t pub[ECC_POINT_LEN];
  uint8_t prv[ECC_SCALAR_LEN];
} ecc_pair_t;

#define SECRET_KEY_LEN    64
typedef struct shared_s {
  uint8_t secret[SECRET_KEY_LEN];
} shared_t;

typedef struct gcm_s {
  aes_gcm_ctx_t ctx;
  enum GCM_Mode mode;
} gcm_t;

#define ECC_PAIRS_LEN     4
#define SHARED_LEN        4
#define GCM_LEN           4
typedef struct state_s {
  ecc_pair_t ecc_pairs[ECC_PAIRS_LEN];
  uint32_t ecc_pairs_count;

  shared_t shared[SHARED_LEN];
  uint32_t shared_count;

  gcm_t gcm[GCM_LEN];
  uint32_t gcm_count;
} state_t;



/*
 * Keying constants
 *
 */

const uint8_t bulkencryption_master[] = "bulkencryption master";
const uint8_t bulkencryption_secret_data[] =
  { 0x7a, 0x5a, 0xfc, 0x2b, 0x03, 0xd4, 0x98, 0x9c,
    0x13, 0xec, 0x35, 0x1d, 0xe6, 0x9a, 0xa4, 0x32,
    0x2a, 0x1f, 0xf3, 0x9d, 0x54, 0xa9, 0xc1, 0xe4,
    0x76, 0xbd, 0x32, 0x76, 0xf8, 0x9c, 0xda, 0xc4
  };
const uint8_t bulkencryption_secret_data_exchange1[] =
  { 0x13, 0x9d, 0xcb, 0xd6, 0x33, 0x33, 0xaa, 0xb6,
    0x87, 0xce, 0xb6, 0xdf, 0x5e, 0x60, 0x86, 0x39,
    0x9b, 0xf3, 0x62, 0xa6, 0xd2, 0x60, 0x11, 0x84,
    0x2e, 0xdc, 0x70, 0xad, 0x9e, 0x5f, 0x5a, 0x4c
  };
const uint8_t bulkencryption_secret_data_exchange2[] =
  { 0x17, 0x67, 0xd7, 0xb5, 0xbb, 0x4b, 0x4b, 0x5e,
    0xec, 0x2b, 0x87, 0x33, 0xd3, 0x80, 0x37, 0x35,
    0xa6, 0xd0, 0x97, 0x8b, 0x6c, 0x5b, 0x99, 0x4b,
    0x7d, 0x92, 0xbc, 0x02, 0x92, 0x0a, 0x30, 0xcb
  };
static const uint8_t bulkencryption_curve_base_point[ECC_POINT_LEN] =
 { 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
 0x75,0x2c,0xb4,0x5c,0x48,0x64,0x8b,0x18,0x9d,0xf9,0x0c,0xb2,0x29,0x6b,0x28,0x78,
 0xa3,0xbf,0xd9,0xf4,0x2f,0xc6,0xc8,0x18,0xec,0x8b,0xf3,0xc9,0xc0,0xc6,0x20,0x39,
 0x13,0xf6,0xec,0xc5,0xcc,0xc7,0x24,0x34,0xb1,0xae,0x94,0x9d,0x56,0x8f,0xc9,0x9c,
 0x60,0x59,0xd0,0xfb,0x13,0x36,0x48,0x38,0xaa,0x30,0x2a,0x94,0x0a,0x2f,0x19,0xba,0x6c };



/*
 * EC Key generation
 *
 */
void bulkencryption_ecc_genkey(uint8_t *seed, uint8_t *prv, uint8_t *pub) {
  prng_ctx_t prng;
  uint8_t tmp[SHA256_OUTPUT_LEN];

  kdf_getkey(bulkencryption_master, sizeof(bulkencryption_master),
             bulkencryption_secret_data, sizeof(bulkencryption_secret_data),
             seed,
             tmp);

  prng_init(&prng, AES_128, tmp, ((uint64_t*)tmp)[2], ((uint64_t*)tmp)[3]);

  for (int i = 0; i < ECC_SCALAR_LEN; i += AES_BLOCK_SIZE)
    prng_output(&prng, &prv[i]);

  /* Generate the public key*/
  msel_memcpy(pub, bulkencryption_curve_base_point, ECC_POINT_LEN);
  ecc_ctx_t ecc_ctx;
  ecc_ctx.scalar = prv;
  ecc_ctx.point = pub;
  msel_svc(MSEL_SVC_ECC, &ecc_ctx);

  msel_memset(tmp, 0, SHA256_OUTPUT_LEN);
  msel_memset(&prng, 0, sizeof(prng));
}



/*
 * The bulkencryption task and its packet processing routines
 *
 */
void processPacket(state_t *state, ffs_packet_t *pkt);

void bulkencryption_task(void *arg, const size_t arg_sz) {
  ffs_packet_t *pkt = NULL;
  state_t *state = NULL;

  pkt = msel_malloc(sizeof(ffs_packet_t));
  if (NULL == pkt)
    goto cleanup;
  state = msel_malloc(sizeof(state_t));
  if (NULL == state)
    goto cleanup;
  msel_memset(state, 0, sizeof(*state));

  int keepRunning = 1;

  while (keepRunning) {
    msel_memset(pkt, 0, sizeof(*pkt));
    msel_svc(MSEL_SVC_FFS_SESSION_RECV, pkt);

    if (0 != pkt->session)
      processPacket(state, pkt);

    /* yield, it's polite */
    msel_svc(MSEL_SVC_YIELD, NULL);
  }

  cleanup:

  if (NULL != pkt)
    msel_free(pkt);
  if (NULL != state)
    msel_free(state);
}

int processPacket_Logout(state_t *state, ffs_packet_t *pkt);
int processPacket_GenDHPair(state_t *state, ffs_packet_t *pkt, struct GenDHPair_Cmd *cmd_data);
int processPacket_DHExchange(state_t *state, ffs_packet_t *pkt, struct DHExchange_Cmd *cmd_data);
int processPacket_GCMInit(state_t *state, ffs_packet_t *pkt, struct GcmInit_Cmd *cmd_data);
int processPacket_GCMDoBlock(state_t *state, ffs_packet_t *pkt, struct GcmDoBlock_Cmd *cmd_data);

void processPacket(state_t *state, ffs_packet_t *pkt) {
  int err = 0;
  uint32_t pos = 0;

  /* Parse the command */
  enum BulkEncryptionCommand cmd = BEC_LOGOUT;
  err = BulkEncryptionCommand_deserialize(pkt->data, FFS_DATA_SIZE, &pos, &cmd);
  if (err) goto cleanup;

  switch (cmd) {
    case BEC_LOGOUT:
      {
        err = processPacket_Logout(state, pkt);
        break;
      }
    case BEC_GEN_DHPAIR:
      {
        struct GenDHPair_Cmd cmd_data;
        GenDHPair_Cmd_init(&cmd_data);
        err = GenDHPair_Cmd_deserialize(pkt->data, FFS_DATA_SIZE, &pos, &cmd_data);
        if (!err)
          err = processPacket_GenDHPair(state, pkt, &cmd_data);
        GenDHPair_Cmd_cleanup(&cmd_data);
        break;
      }
    case BEC_DH_EXCHANGE:
      {
        struct DHExchange_Cmd cmd_data;
        DHExchange_Cmd_init(&cmd_data);
        err = DHExchange_Cmd_deserialize(pkt->data, FFS_DATA_SIZE, &pos, &cmd_data);
        if (!err)
          err = processPacket_DHExchange(state, pkt, &cmd_data);
        DHExchange_Cmd_cleanup(&cmd_data);
        break;
      }
    case BEC_GCM_INIT:
      {
        struct GcmInit_Cmd cmd_data;
        GcmInit_Cmd_init(&cmd_data);
        err = GcmInit_Cmd_deserialize(pkt->data, FFS_DATA_SIZE, &pos, &cmd_data);
        if (!err)
          err = processPacket_GCMInit(state, pkt, &cmd_data);
        GcmInit_Cmd_cleanup(&cmd_data);
        break;
      }
    case BEC_GCM_DOBLOCK:
      {
        struct GcmDoBlock_Cmd cmd_data;
        GcmDoBlock_Cmd_init(&cmd_data);
        err = GcmDoBlock_Cmd_deserialize(pkt->data, FFS_DATA_SIZE, &pos, &cmd_data);
        if (!err)
          err = processPacket_GCMDoBlock(state, pkt, &cmd_data);
        GcmDoBlock_Cmd_cleanup(&cmd_data);
        break;
      }
  }

  cleanup:
  if (err) {
    uint32_t err_pos = 0;
    msel_memset(pkt->data, 0, FFS_DATA_SIZE);
    int err_err = BulkEncryptionResponse_serialize(pkt->data, FFS_DATA_SIZE, &err_pos, BER_ERROR);
    if (!err_err) {
      while (msel_svc(MSEL_SVC_FFS_SESSION_SEND, pkt) != MSEL_OK) 
        msel_svc(MSEL_SVC_YIELD, NULL);
    }
  }
  else {
    while (msel_svc(MSEL_SVC_FFS_SESSION_SEND, pkt) != MSEL_OK) 
      msel_svc(MSEL_SVC_YIELD, NULL);
  }
}

int processPacket_Logout(state_t *state, ffs_packet_t *pkt) {
  int err = 0;
  uint32_t pos = 0;

  msel_memset(state, 0, sizeof(*state));

  msel_memset(pkt->data, 0, FFS_DATA_SIZE);
  err = BulkEncryptionResponse_serialize(pkt->data, FFS_DATA_SIZE, &pos, BER_GOODBYE);
  return err;
}

int processPacket_GenDHPair(state_t *state, ffs_packet_t *pkt, struct GenDHPair_Cmd *cmd_data) {
  int err = 0;
  uint32_t pos = 0;

  struct GenDHPair_Res res_data;
  GenDHPair_Res_init(&res_data);

  if (state->ecc_pairs_count >= ECC_PAIRS_LEN) {
    err = -1;
    goto cleanup;
  }
  uint32_t idx = state->ecc_pairs_count;
  ecc_pair_t *pair = &state->ecc_pairs[idx];
  state->ecc_pairs_count++;

  bulkencryption_ecc_genkey(cmd_data->seed, pair->prv, pair->pub);
  msel_memcpy(res_data.pubkey, pair->pub, GENDHPAIR_RES_PUBKEY_LENGTH);
  res_data.privkey_idx = idx;

  msel_memset(pkt->data, 0, FFS_DATA_SIZE);
  err = BulkEncryptionResponse_serialize(pkt->data, FFS_DATA_SIZE, &pos, BER_DHPAIR);
  if (err)
    err = GenDHPair_Res_serialize(pkt->data, FFS_DATA_SIZE, &pos, &res_data);

  cleanup:
  GenDHPair_Res_cleanup(&res_data);
  return err;
}

int processPacket_DHExchange(state_t *state, ffs_packet_t *pkt, struct DHExchange_Cmd *cmd_data) {
  int err = 0;
  uint32_t pos = 0;

  struct DHExchange_Res res_data;
  DHExchange_Res_init(&res_data);

  if (state->shared_count >= SHARED_LEN) {
    err = -1;
    goto cleanup;
  }
  if (cmd_data->local_privkey_idx >= ECC_PAIRS_LEN) {
    err = -1;
    goto cleanup;
  }
  uint8_t *local_prv = state->ecc_pairs[cmd_data->local_privkey_idx].prv;
  uint8_t *remote_pub = cmd_data->remote_pubkey;

  uint32_t idx = state->shared_count;
  shared_t *shared = &state->shared[idx];
  state->shared_count++;

  /* perform the exchange */
  uint8_t exchanged[ECC_POINT_LEN];
  msel_memcpy(exchanged, remote_pub, ECC_POINT_LEN);
  ecc_ctx_t ecc_ctx;
  ecc_ctx.scalar = local_prv;
  ecc_ctx.point = exchanged;
  msel_svc(MSEL_SVC_ECC, &ecc_ctx);

  /* compute the hash of the shared point */
  uint8_t seed[SHA256_OUTPUT_LEN];
  sha256_hash(exchanged, ECC_POINT_LEN, seed);

  /* use the kdf to compute the shared secret */
  kdf_getkey(bulkencryption_master, sizeof(bulkencryption_master),
             bulkencryption_secret_data_exchange1, sizeof(bulkencryption_secret_data_exchange1),
             seed,
             &shared->secret[0]);
  kdf_getkey(bulkencryption_master, sizeof(bulkencryption_master),
             bulkencryption_secret_data_exchange2, sizeof(bulkencryption_secret_data_exchange2),
             seed,
             &shared->secret[32]);


  res_data.shared_idx = idx;

  msel_memset(pkt->data, 0, FFS_DATA_SIZE);
  err = BulkEncryptionResponse_serialize(pkt->data, FFS_DATA_SIZE, &pos, BER_DH_EXCHANGED);
  if (!err)
    err = DHExchange_Res_serialize(pkt->data, FFS_DATA_SIZE, &pos, &res_data);

  cleanup:
  DHExchange_Res_cleanup(&res_data);
  return err;
}

int processPacket_GCMInit(state_t *state, ffs_packet_t *pkt, struct GcmInit_Cmd *cmd_data) {
  int err = 0;
  uint32_t pos = 0;

  struct GcmInit_Res res_data;
  GcmInit_Res_init(&res_data);

  if (state->gcm_count >= GCM_LEN) {
    err = -1;
    goto cleanup;
  }
  if (cmd_data->shared_idx >= SHARED_LEN) {
    err = -1;
    goto cleanup;
  }
  shared_t *shared = &state->shared[cmd_data->shared_idx];

  uint32_t idx = state->gcm_count;
  gcm_t *gcm = &state->gcm[idx];
  state->gcm_count++;

  res_data.gcm_idx = idx;

  gcm->mode = cmd_data->mode;
  aes_gcm_setkey(&gcm->ctx, AES_GCM_128, &shared->secret[0], &shared->secret[AES_GCM_128_KEY_SIZE], SECRET_KEY_LEN - AES_GCM_128_KEY_SIZE);

  msel_memset(pkt->data, 0, FFS_DATA_SIZE);
  err = BulkEncryptionResponse_serialize(pkt->data, FFS_DATA_SIZE, &pos, BER_GCM_READY);
  if (!err)
    err = GcmInit_Res_serialize(pkt->data, FFS_DATA_SIZE, &pos, &res_data);

  cleanup:
  GcmInit_Res_cleanup(&res_data);
  return err;
}

int processPacket_GCMDoBlock(state_t *state, ffs_packet_t *pkt, struct GcmDoBlock_Cmd *cmd_data) {
  int err = 0;
  uint32_t pos = 0;

  struct GcmDoBlock_Res res_data;
  GcmDoBlock_Res_init(&res_data);

  if (cmd_data->gcm_idx >= GCM_LEN) {
    err = -1;
    goto cleanup;
  }
  gcm_t *gcm = &state->gcm[cmd_data->gcm_idx];
  switch (gcm->mode) {
    case GCM128_ENCRYPT:
      aes_gcm_encrypt(&gcm->ctx, cmd_data->block, GCMDOBLOCK_CMD_BLOCK_LENGTH, res_data.block);
      break;
    case GCM128_DECRYPT:
      aes_gcm_decrypt(&gcm->ctx, cmd_data->block, GCMDOBLOCK_CMD_BLOCK_LENGTH, res_data.block);
      break;
  }

  msel_memset(pkt->data, 0, FFS_DATA_SIZE);
  err = BulkEncryptionResponse_serialize(pkt->data, FFS_DATA_SIZE, &pos, BER_GCM_BLOCKS);
  if (!err)
    err = GcmDoBlock_Res_serialize(pkt->data, FFS_DATA_SIZE, &pos, &res_data);

  cleanup:
  GcmDoBlock_Res_cleanup(&res_data);
  return err;
}

