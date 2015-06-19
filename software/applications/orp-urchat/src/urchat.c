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

#include <tidl.h>
#include <chatProto.h>

/* 
 * Chat task: 
 *   Purpose: supports the UrChat demo
 *   Endpoint "chat\n\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
 */
void urchat_task(void *arg, const size_t arg_sz);
const uint8_t urchat_endpoint[32] =
  { 0x63, 0x68, 0x61, 0x74, 0xa, 0x0, 0x0, 0x0,
    0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,
    0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,
    0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0
  };


static const uint8_t curve_base_point[ECC_POINT_LEN] =
 { 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
 0x75,0x2c,0xb4,0x5c,0x48,0x64,0x8b,0x18,0x9d,0xf9,0x0c,0xb2,0x29,0x6b,0x28,0x78,
 0xa3,0xbf,0xd9,0xf4,0x2f,0xc6,0xc8,0x18,0xec,0x8b,0xf3,0xc9,0xc0,0xc6,0x20,0x39,
 0x13,0xf6,0xec,0xc5,0xcc,0xc7,0x24,0x34,0xb1,0xae,0x94,0x9d,0x56,0x8f,0xc9,0x9c,
 0x60,0x59,0xd0,0xfb,0x13,0x36,0x48,0x38,0xaa,0x30,0x2a,0x94,0x0a,0x2f,0x19,0xba,0x6c };

static const uint8_t chat_secret_data[SHA256_OUTPUT_LEN] =
  { 0xcf, 0xfa, 0x04, 0xe3, 0xef, 0xfe, 0xfa, 0x8d
  , 0xb6, 0x21, 0x5b, 0x69, 0xc7, 0xf6, 0x8b, 0x6a
  , 0xa1, 0xd9, 0x51, 0x4e, 0xe5, 0x14, 0xc4, 0xe1
  , 0x1b, 0x42, 0x0b, 0xb5, 0xe5, 0x7b, 0xcc, 0x58
  };

static const uint8_t chat_master[] = "Chat master key";
static const uint8_t chat_login[] = "Chat login action";
static const uint8_t chat_seek1[] = "Chat seek1";
static const uint8_t chat_seek2[] = "Chat seek2";
static const uint8_t chat_xchg1[] = "Chat exchange 1";
static const uint8_t chat_xchg2[] = "Chat exchange 2";
static const uint8_t chat_key[] = "Chat key";
static const uint8_t chat_iv[] = "Chat IV";

#define CHAT_PRVKEY_LEN   ((uint32_t) 60) // 65)
#define CHAT_PUBKEY_LEN   ((uint32_t) 70)


typedef enum {
  CSS_CLOSED = 0,     /* This channel is not in use */
  CSS_LISTENING,      /* This channel is waiting for an exchange */
  CSS_OPEN,           /* This channel is open and chat-able */
} chat_session_state_t;

typedef struct {
  chat_session_state_t state;
  uint8_t remote_signing_key[ECC_POINT_LEN];
  uint8_t local_key_prv[ECC_SCALAR_LEN];
  uint8_t local_key_pub[ECC_POINT_LEN];
  uint8_t local_key_pub_hash[SHA256_OUTPUT_LEN];
  uint8_t xchg_key[AES_GCM_128_KEY_SIZE];
  uint8_t xchg_iv[SHA256_OUTPUT_LEN];
  uint8_t send_key[AES_GCM_128_KEY_SIZE];
  uint8_t send_iv[SHA256_OUTPUT_LEN];
  uint8_t recv_key[AES_GCM_128_KEY_SIZE];
  uint8_t recv_iv[SHA256_OUTPUT_LEN];
} chat_session_t;

#define SESSION_BUFFER_SIZE ((uint32_t) (512))     // TODO: make bigger
#define MESSAGE_SIZE ((uint32_t) (SESSION_BUFFER_SIZE-128))
#define SESSIONS_SIZE ((uint32_t) 4)
#define SEEKS_SIZE ((uint32_t) 4)

typedef struct {
  uint8_t key[AES_GCM_128_KEY_SIZE];
  uint8_t iv[SHA256_OUTPUT_LEN];
} seeking_t;

typedef enum {
  CS_AWAITING_LOGIN,
  CS_AWAITING_COMMANDS,
  CS_SHOULD_EXIT
} chat_state_t;

typedef struct {
  uint8_t session_buffer[SESSION_BUFFER_SIZE];
  chat_state_t state;
  uint8_t key_prv[ECC_SCALAR_LEN];
  uint8_t key_pub[ECC_POINT_LEN];
  uint8_t key_pub_hash[SHA256_OUTPUT_LEN];
  prng_ctx_t prng;
  seeking_t seeking[SEEKS_SIZE];
  uint32_t seeking_active_count;
  chat_session_t sessions[SESSIONS_SIZE];
} chat_context_t;

void clean_session_buffer(chat_context_t *ctx) {
  msel_memset(ctx->session_buffer, 0, SESSION_BUFFER_SIZE);
  for (int i = 0; i < SESSION_BUFFER_SIZE; i += AES_BLOCK_SIZE)
    prng_output(&ctx->prng, &ctx->session_buffer[i]);
  for (int i = 0; i < SESSION_BUFFER_SIZE; i += SHA256_OUTPUT_LEN)
    sha256_hash(&ctx->session_buffer[i], SHA256_OUTPUT_LEN, &ctx->session_buffer[i]);
}

/** @brief Generates an ECC public/private pair.
 *
 * @param seed (Input) Non-null pointer to array of size SHA256_OUTPUT_LEN.
 * @param prv (Output) Non-null pointer to an array of size ECC_SCALAR_LEN.
 * @param pub (Output) Non-null pointer to an array of size ECC_POINT_LEN.
 */

void chat_ecc_genkey(uint8_t *seed, uint8_t *prv, uint8_t *pub) {
  /* Generate the private key */
  uint8_t tmp[SHA256_OUTPUT_LEN];
  kdf_getkey(chat_master, sizeof(chat_master), chat_secret_data, SHA256_OUTPUT_LEN, seed, tmp);
  prng_ctx_t prng;
  prng_init(&prng, AES_128, tmp, ((uint64_t*)tmp)[2], ((uint64_t*)tmp)[3]);
  msel_memset(tmp, 0, SHA256_OUTPUT_LEN);

  for (int i = 0; i < ECC_SCALAR_LEN; i += AES_BLOCK_SIZE)
    prng_output(&prng, &prv[i]);
  for (int i = 0; i < ECC_SCALAR_LEN - CHAT_PRVKEY_LEN; i++)
    prv[i] = 0;

  /* Generate the public key*/
  msel_memcpy(pub, curve_base_point, ECC_POINT_LEN);
  ecc_ctx_t ecc_ctx;
  ecc_ctx.scalar = prv;
  ecc_ctx.point = pub;
  // msel_svc(MSEL_SVC_ECC, &ecc_ctx);
  for (int i = 0; i < ECC_POINT_LEN; i++)
    ecc_ctx.point[i] ^= ecc_ctx.scalar[i];

  msel_memset(&prng, 0, sizeof(prng));
}

/** @brief Process a login packet.
 *
 * A login packet consists of a 32-byte uninterpreted value.
 * [SHA256_OUTPUT_LEN-byte uninterpreted array]
 *
 * On success, the reponse packet is
 * [CR_PUBKEY][(uint16) CHAT_PUBKEY_LEN][byte array of length CHAT_PUBKEY_LEN]
 *
 * @param ctx Non-null. The chat context.
 * @param pkt Non-null. The incoming login packet. On success, this
 *            pointer re-used to format the outgoing reply message.
 * @return 0 on success.
 */
int chat_process_login(chat_context_t *ctx, ffs_packet_t *pkt) {
  int err = 0;

  /* Generate the ECC key and compute its hash */
  {
    chat_ecc_genkey(pkt->data, ctx->key_prv, ctx->key_pub);
    sha256_hash(ctx->key_pub, ECC_POINT_LEN, ctx->key_pub_hash);
  }

  /* Initialize the prng */
  {
    uint8_t seed[SHA256_OUTPUT_LEN];
    sha256_hash(pkt->data, SHA256_OUTPUT_LEN, seed);
    uint8_t r[SHA256_OUTPUT_LEN];
    for (int i = 0; i < SHA256_OUTPUT_LEN; i++)
      msel_svc(MSEL_SVC_TRNG, &r[i]);
    sha256_hash(r, SHA256_OUTPUT_LEN, r);
    for (int i = 0; i < SHA256_OUTPUT_LEN; i++)
      seed[i] ^= r[i];
    sha256_hash(seed, SHA256_OUTPUT_LEN, seed);
    prng_init(&ctx->prng, AES_128, seed, ((uint64_t*)seed)[2], ((uint64_t*)seed)[3]);
  }

  /* Format the response */
  {
    uint32_t length = 0;
    msel_memset(pkt->data, 0, FFS_DATA_SIZE);
    /* [CR_PUBKEY] */
    err = ChatResponse_serialize(pkt->data, FFS_DATA_SIZE, &length, CR_PUBKEY);
    if (err) goto cleanup;
    /* [(uint16) CHAT_PUBKEY_LEN] */
    err = uint16_serialize(pkt->data, FFS_DATA_SIZE, &length, CHAT_PUBKEY_LEN);
    if (err) goto cleanup;
    /* [CHAT_PUBKEY_LEN-byte ecc point] */
    err = uint8_serialize(pkt->data, FFS_DATA_SIZE, &length, ctx->key_pub[0]);
    if (err) goto cleanup;
    msel_memcpy(&pkt->data[length], &ctx->key_pub[ECC_POINT_LEN - (CHAT_PUBKEY_LEN - 1)], (CHAT_PUBKEY_LEN - 1));
    length += CHAT_PUBKEY_LEN - 1;
  }

  /* Update our status */
  ctx->state = CS_AWAITING_COMMANDS;

  /* Send the response */
  while (msel_svc(MSEL_SVC_FFS_SESSION_SEND, pkt) != MSEL_OK) 
      msel_svc(MSEL_SVC_YIELD, NULL);

  cleanup:
  return err;
}

/** @brief Process a logout packet.
 *
 * A logout packet carries no data and always succeeds. It responds with the packet
 * [CR_GOODBYE]
 *
 * @param ctx Non-null. The chat context.
 * @param pkt Non-null. The incoming login packet. On success, this
 *            pointer re-used to format the outgoing reply message.
 * @return 0 on success.
 */
int chat_process_logout(chat_context_t *ctx, ffs_packet_t *pkt, uint32_t *pos) {
  int err = 0;

  /* Format the response */
  {
    uint32_t length = 0;
    msel_memset(pkt->data, 0, FFS_DATA_SIZE);
    /* [CR_GOODBYE] */
    err = ChatResponse_serialize(pkt->data, FFS_DATA_SIZE, &length, CR_GOODBYE);
    if (err) goto cleanup;
  }

  /* Update our state */
  msel_memset(ctx, 0, sizeof(*ctx));
  ctx->state = CS_AWAITING_LOGIN;

  /* Send the response */
  while (msel_svc(MSEL_SVC_FFS_SESSION_SEND, pkt) != MSEL_OK) 
      msel_svc(MSEL_SVC_YIELD, NULL);

  cleanup:
  return err;
}

/** @brief Process a set-seek packet.
 *
 * A set-seek packet consists of a uint32 L, followed by
 * L * SHA256_OUTPUT_LEN uninterpreted bytes. To be well-formed,
 * we require L <= SEEKS_SIZE.
 * [uint32 L][L * SHA256_OUTPUT_LEN bytes]
 *
 * On success, the response is simply
 * [CR_SEEKUPDATED]
 *
 * @param ctx Non-null. The chat context.
 * @param pkt Non-null. The incoming login packet. On success, this
 *            pointer re-used to format the outgoing reply message.
 * @return 0 on success.
 */
int chat_process_setseek(chat_context_t *ctx, ffs_packet_t *pkt, uint32_t *pos) {
  int err = 0;

  uint32_t L = 0;
  err = uint32_deserialize(pkt->data, FFS_DATA_SIZE, pos, &L);
  if (err) goto cleanup;
  if (L > SEEKS_SIZE) { err = -1; goto cleanup; }

  /* Compute the seek phrases */
  uint8_t *phrases = &pkt->data[*pos];
  seeking_t new_seeking[SEEKS_SIZE];
  msel_memset(new_seeking, 0, sizeof(new_seeking));
  for (int i = 0; i < L; i++) {
      kdf_getkey(chat_master, sizeof(chat_master), chat_seek1, sizeof(chat_seek1), &phrases[i * SHA256_OUTPUT_LEN], new_seeking[i].key);
      kdf_getkey(chat_master, sizeof(chat_master), chat_seek2, sizeof(chat_seek2), &phrases[i * SHA256_OUTPUT_LEN], new_seeking[i].iv);
  }

  /* Format our response */
  {
    uint32_t length = 0;
    msel_memset(pkt->data, 0, FFS_DATA_SIZE);
    /* [CR_SEEKUPDATED] */
    err = ChatResponse_serialize(pkt->data, FFS_DATA_SIZE, &length, CR_SEEKUPDATED);
    if (err) goto cleanup;
  }

  /* Update our state */
  ctx->seeking_active_count = L;
  msel_memcpy(ctx->seeking, new_seeking, sizeof(ctx->seeking));
  msel_memset(new_seeking, 0, sizeof(new_seeking));

  /* Send the response */
  while (msel_svc(MSEL_SVC_FFS_SESSION_SEND, pkt) != MSEL_OK) 
      msel_svc(MSEL_SVC_YIELD, NULL);

  cleanup:
  return err;
}

/** @brief Process a get-seek packet.
 *
 * A get-seek packet consists of a uint32 J. To be well-formed,
 * we require J < SEEKS_SIZE.
 * [uint32 J]
 *
 * On success, the response is simply
 * [CR_SEEK][SESSION_BUFFER_SIZE bytes of uninterpreted data]
 *
 * @param ctx Non-null. The chat context.
 * @param pkt Non-null. The incoming login packet. On success, this
 *            pointer re-used to format the outgoing reply message.
 * @return 0 on success.
 */
int chat_process_getseek(chat_context_t *ctx, ffs_packet_t *pkt, uint32_t *pos) {
  int err = 0;

  uint32_t J = 0;
  err = uint32_deserialize(pkt->data, FFS_DATA_SIZE, pos, &J);
  if (err) goto cleanup;
  if (J > SEEKS_SIZE) { err = -1; goto cleanup; }

  /* Format our response. */
  clean_session_buffer(ctx);
  uint32_t out_pos = 0;
  {
    /* [SHA256_OUTPUT_LEN nonce] */
    uint8_t nonce[SHA256_OUTPUT_LEN];
    for (int i = 0; i < SHA256_OUTPUT_LEN; i += AES_BLOCK_SIZE)
      prng_output(&ctx->prng, &nonce[i]);
    sha256_hash(nonce, SHA256_OUTPUT_LEN, nonce);
    msel_memcpy(&ctx->session_buffer[out_pos], nonce, SHA256_OUTPUT_LEN);
    out_pos += SHA256_OUTPUT_LEN;
    msel_memset(nonce, 0, SHA256_OUTPUT_LEN);
  }
  {
    /* [CR_SEEK] */
    err = ChatResponse_serialize(ctx->session_buffer, SESSION_BUFFER_SIZE, &out_pos, CR_SEEK);
    if (err) goto cleanup;
  }
  {
    /* [SHA256_OUTPUT_LEN The pubkey hash] */
    msel_memcpy(&ctx->session_buffer[out_pos], ctx->key_pub_hash, SHA256_OUTPUT_LEN);
    out_pos += SHA256_OUTPUT_LEN;
  }
  {
    /* [ECC_POINT_LEN The pubkey itself] */
    msel_memcpy(&ctx->session_buffer[out_pos], ctx->key_pub, ECC_POINT_LEN);
    out_pos += ECC_POINT_LEN;
  }
  {
    /* TODO: hmac */
  }
  {
    /* Encrypt seek string */
    aes_gcm_ctx_t gcm;
    msel_memset(&gcm, 0, sizeof(gcm));
    aes_gcm_setkey(&gcm, AES_GCM_128, ctx->seeking[J].key, ctx->seeking[J].iv, SHA256_OUTPUT_LEN);
    aes_gcm_aad(&gcm, NULL, 0);
    aes_gcm_encrypt(&gcm, ctx->session_buffer, SESSION_BUFFER_SIZE, ctx->session_buffer);
    msel_memset(&gcm, 0, sizeof(gcm));
  }
  {
    /* Format the packet */
    uint32_t length = 0;
    msel_memset(pkt->data, 0, FFS_DATA_SIZE);
    /* [CR_SEEK] */
    err = ChatResponse_serialize(pkt->data, FFS_DATA_SIZE, &length, CR_SEEK);
    if (err) goto cleanup;
    /* SESSION_BUFFER_SIZE bytes of uninterpreted data */
    msel_memcpy(&pkt->data[length], ctx->session_buffer, SESSION_BUFFER_SIZE);
    length += SESSION_BUFFER_SIZE;
  }

  /* Send the response */
  while (msel_svc(MSEL_SVC_FFS_SESSION_SEND, pkt) != MSEL_OK) 
      msel_svc(MSEL_SVC_YIELD, NULL);

  cleanup:
  return err;
}

/** @brief Process a get-channel packet.
 *
 * A get-channel packet consists of a public key.
 * [(uint16) CHAT_PUBKEY_LEN][byte array of length CHAT_PUBKEY_LEN]
 *
 * On success, the response is simply
 * [CR_CHANNEL][uninterpreted uint32_t]
 *
 * @param ctx Non-null. The chat context.
 * @param pkt Non-null. The incoming login packet. On success, this
 *            pointer re-used to format the outgoing reply message.
 * @return 0 on success.
 */
int chat_process_getchannel(chat_context_t *ctx, ffs_packet_t *pkt, uint32_t *pos) {
  int err = 0;

  /* First, do we even have room for another channel? */
  uint32_t channel = 0;
  {
    while ((channel < SESSIONS_SIZE) && (CSS_CLOSED != ctx->sessions[channel].state))
      channel++;
    if (channel >= SESSIONS_SIZE) { err = -1; goto cleanup; }
  }

  uint16_t incoming_len;
  err = uint16_deserialize(pkt->data, FFS_DATA_SIZE, pos, &incoming_len);
  if (err) goto cleanup;
  if (CHAT_PUBKEY_LEN != incoming_len) { err = -1; goto cleanup; }

  uint8_t remote_signing_key[ECC_POINT_LEN];
  msel_memset(remote_signing_key, 0, ECC_POINT_LEN);
  err = uint8_deserialize(pkt->data, FFS_DATA_SIZE, pos, &remote_signing_key[0]);
  if (err) goto cleanup;
  msel_memcpy(&remote_signing_key[ECC_POINT_LEN - (CHAT_PUBKEY_LEN - 1)], &pkt->data[*pos], (CHAT_PUBKEY_LEN - 1));

  /* Format our response */
  {
    uint32_t length = 0;
    msel_memset(pkt->data, 0, FFS_DATA_SIZE);
    /* [CR_CHANNEL] */
    err = ChatResponse_serialize(pkt->data, FFS_DATA_SIZE, &length, CR_CHANNEL);
    if (err) goto cleanup;
    /* [uninterpreted uint32_t] */
    err = uint32_serialize(pkt->data, FFS_DATA_SIZE, &length, channel);
    if (err) goto cleanup;
  }

  /* Update our state */
  {
    ctx->sessions[channel].state = CSS_LISTENING;
    msel_memcpy(ctx->sessions[channel].remote_signing_key, remote_signing_key, ECC_POINT_LEN);

    /* Get the hash of the shared ECC point */
    uint8_t common_pt_hash[SHA256_OUTPUT_LEN];
    {
      uint8_t common_pt[ECC_POINT_LEN];
      msel_memcpy(common_pt, remote_signing_key, ECC_POINT_LEN);
      ecc_ctx_t ecc_ctx;
      ecc_ctx.scalar = ctx->key_prv;
      ecc_ctx.point = common_pt;
      // msel_svc(MSEL_SVC_ECC, &ecc_ctx);
      for (int i = 0; i < ECC_POINT_LEN; i++)
        ecc_ctx.point[i] ^= ecc_ctx.scalar[i];
      sha256_hash(common_pt, ECC_POINT_LEN, common_pt_hash);
    }

    /* Compute the exchange key */
    {
      kdf_getkey(chat_master, sizeof(chat_master), chat_xchg1, sizeof(chat_xchg1), common_pt_hash, ctx->sessions[channel].xchg_key);
      kdf_getkey(chat_master, sizeof(chat_master), chat_xchg2, sizeof(chat_xchg2), common_pt_hash, ctx->sessions[channel].xchg_iv);
    }

    /* Compute an ephemeral public/private pair */
    {
      uint8_t nonce[SHA256_OUTPUT_LEN];
      for (int i = 0; i < SHA256_OUTPUT_LEN; i += AES_BLOCK_SIZE)
        prng_output(&ctx->prng, &nonce[i]);
      sha256_hash(nonce, SHA256_OUTPUT_LEN, nonce);

      chat_ecc_genkey(nonce, ctx->sessions[channel].local_key_prv, ctx->sessions[channel].local_key_pub);
      sha256_hash(ctx->sessions[channel].local_key_pub, ECC_POINT_LEN, ctx->sessions[channel].local_key_pub_hash);

      msel_memset(nonce, 0, SHA256_OUTPUT_LEN);
    }
  }
  msel_memset(remote_signing_key, 0, ECC_POINT_LEN);

  /* Send the response */
  while (msel_svc(MSEL_SVC_FFS_SESSION_SEND, pkt) != MSEL_OK) 
      msel_svc(MSEL_SVC_YIELD, NULL);

  cleanup:
  return err;
}

/** @brief Process a close-channel packet.
 *
 * A close-channel packet consists of a channel identifier.
 * [uninterpreted uint32]
 *
 * On success, the response is simply
 * [CR_CHANNEL_CLOSED]
 *
 * @param ctx Non-null. The chat context.
 * @param pkt Non-null. The incoming login packet. On success, this
 *            pointer re-used to format the outgoing reply message.
 * @return 0 on success.
 */
int chat_process_closechannel(chat_context_t *ctx, ffs_packet_t *pkt, uint32_t *pos) {
  int err = 0;

  uint32_t channel = 0;
  err = uint32_deserialize(pkt->data, FFS_DATA_SIZE, pos, &channel);
  if (err) goto cleanup;
  if (channel >= SESSIONS_SIZE) { err = -1; goto cleanup; }

  /* Format our response */
  {
    uint32_t length = 0;
    msel_memset(pkt->data, 0, FFS_DATA_SIZE);
    /* [CR_CHANNEL_CLOSED] */
    err = ChatResponse_serialize(pkt->data, FFS_DATA_SIZE, &length, CR_CHANNEL_CLOSED);
    if (err) goto cleanup;
  }

  /* Update our state */
  {
    msel_memset(&ctx->sessions[channel], 0, sizeof(ctx->sessions[channel]));
    ctx->sessions[channel].state = CSS_CLOSED;
  }

  /* Send the response */
  while (msel_svc(MSEL_SVC_FFS_SESSION_SEND, pkt) != MSEL_OK) 
      msel_svc(MSEL_SVC_YIELD, NULL);

  cleanup:
  return err;
}

/** @brief Process a get-exchange packet.
 *
 * A get-exchange packet consists of a uint32 J. To be well-formed,
 * J must identify a channel in the CSS_LISTENING or CSS_OPEN state.
 * [uint32 J]
 *
 * On success, the response is simply
 * [CR_EXCHANGE][SESSION_BUFFER_SIZE bytes of uninterpreted data]
 *
 * @param ctx Non-null. The chat context.
 * @param pkt Non-null. The incoming login packet. On success, this
 *            pointer re-used to format the outgoing reply message.
 * @return 0 on success.
 */
int chat_process_getexchange(chat_context_t *ctx, ffs_packet_t *pkt, uint32_t *pos) {
  int err = 0;

  uint32_t channel = 0;
  err = uint32_deserialize(pkt->data, FFS_DATA_SIZE, pos, &channel);
  if (err) goto cleanup;
  if (channel >= SESSIONS_SIZE) { err = -1; goto cleanup; }
  if ((CSS_LISTENING != ctx->sessions[channel].state) &&
      (CSS_OPEN != ctx->sessions[channel].state))
  { err = -1; goto cleanup; }

  /* Format our response. */
  clean_session_buffer(ctx);
  uint32_t out_pos = 0;
  {
    /* [SHA256_OUTPUT_LEN nonce] */
    uint8_t nonce[SHA256_OUTPUT_LEN];
    for (int i = 0; i < SHA256_OUTPUT_LEN; i += AES_BLOCK_SIZE)
      prng_output(&ctx->prng, &nonce[i]);
    sha256_hash(nonce, SHA256_OUTPUT_LEN, nonce);
    msel_memcpy(&ctx->session_buffer[out_pos], nonce, SHA256_OUTPUT_LEN);
    out_pos += SHA256_OUTPUT_LEN;
    msel_memset(nonce, 0, SHA256_OUTPUT_LEN);
  }
  {
    /* [CR_EXCHANGE] */
    err = ChatResponse_serialize(ctx->session_buffer, SESSION_BUFFER_SIZE, &out_pos, CR_EXCHANGE);
    if (err) goto cleanup;
  }
  {
    /* [SHA256_OUTPUT_LEN The pubkey hash] */
    msel_memcpy(&ctx->session_buffer[out_pos], ctx->sessions[channel].local_key_pub_hash, SHA256_OUTPUT_LEN);
    out_pos += SHA256_OUTPUT_LEN;
  }
  {
    /* [ECC_POINT_LEN The pubkey itself] */
    msel_memcpy(&ctx->session_buffer[out_pos], ctx->sessions[channel].local_key_pub, ECC_POINT_LEN);
    out_pos += ECC_POINT_LEN;
  }
  {
    /* TODO: ECDSA signature of the pubkey hash */
  }
  {
    /* TODO: hmac */
  }
  {
    /* Encrypt exchange string */
    aes_gcm_ctx_t gcm;
    msel_memset(&gcm, 0, sizeof(gcm));
    aes_gcm_setkey(&gcm, AES_GCM_128, ctx->sessions[channel].xchg_key, ctx->sessions[channel].xchg_iv, SHA256_OUTPUT_LEN);
    aes_gcm_aad(&gcm, NULL, 0);
    aes_gcm_encrypt(&gcm, ctx->session_buffer, SESSION_BUFFER_SIZE, ctx->session_buffer);
    msel_memset(&gcm, 0, sizeof(gcm));
  }
  {
    /* Format the packet */
    uint32_t length = 0;
    msel_memset(pkt->data, 0, FFS_DATA_SIZE);
    /* [CR_EXCHANGE] */
    err = ChatResponse_serialize(pkt->data, FFS_DATA_SIZE, &length, CR_EXCHANGE);
    if (err) goto cleanup;
    /* SESSION_BUFFER_SIZE bytes of uninterpreted data */
    msel_memcpy(&pkt->data[length], ctx->session_buffer, SESSION_BUFFER_SIZE);
    length += SESSION_BUFFER_SIZE;
  }

  /* Send the response */
  while (msel_svc(MSEL_SVC_FFS_SESSION_SEND, pkt) != MSEL_OK) 
      msel_svc(MSEL_SVC_YIELD, NULL);


  cleanup:
  return err;
}

/** @brief Process an encrypt packet.
 *
 * An encrypt packet consists of a uint32 J and an uninterpreted array
 * of MESSAGE_SIZE bytes. To be well-formed,
 * J must identify a channel in the CSS_OPEN state.
 * [uint32 J][MESSAGE_SIZE bytes of uninterpreted data]
 *
 * On success, the response is simply
 * [CR_CIPHERTEXT][SESSION_BUFFER_SIZE bytes of uninterpreted data]
 *
 * @param ctx Non-null. The chat context.
 * @param pkt Non-null. The incoming login packet. On success, this
 *            pointer re-used to format the outgoing reply message.
 * @return 0 on success.
 */
int chat_process_encrypt(chat_context_t *ctx, ffs_packet_t *pkt, uint32_t *pos) {
  int err = 0;

  uint32_t channel = 0;
  err = uint32_deserialize(pkt->data, FFS_DATA_SIZE, pos, &channel);
  if (err) goto cleanup;
  if (channel >= SESSIONS_SIZE) { err = -1; goto cleanup; }
  if (CSS_OPEN != ctx->sessions[channel].state)
  { err = -1; goto cleanup; }

  /* Format our response. */
  clean_session_buffer(ctx);
  uint32_t out_pos = 0;
  {
    /* [SHA256_OUTPUT_LEN nonce] */
    uint8_t nonce[SHA256_OUTPUT_LEN];
    for (int i = 0; i < SHA256_OUTPUT_LEN; i += AES_BLOCK_SIZE)
      prng_output(&ctx->prng, &nonce[i]);
    sha256_hash(nonce, SHA256_OUTPUT_LEN, nonce);
    msel_memcpy(&ctx->session_buffer[out_pos], nonce, SHA256_OUTPUT_LEN);
    out_pos += SHA256_OUTPUT_LEN;
    msel_memset(nonce, 0, SHA256_OUTPUT_LEN);
  }
  {
    /* [CR_CIPHERTEXT] */
    err = ChatResponse_serialize(ctx->session_buffer, SESSION_BUFFER_SIZE, &out_pos, CR_CIPHERTEXT);
    if (err) goto cleanup;
  }
  {
    /* [MESSAGE_SIZE The message] */
    msel_memcpy(&ctx->session_buffer[out_pos], &pkt->data[*pos], MESSAGE_SIZE);
    out_pos += MESSAGE_SIZE;
  }
  {
    /* Encrypt seek string */
    aes_gcm_ctx_t gcm;
    msel_memset(&gcm, 0, sizeof(gcm));
    aes_gcm_setkey(&gcm, AES_GCM_128, ctx->sessions[channel].send_key, ctx->sessions[channel].send_iv, SHA256_OUTPUT_LEN);
    aes_gcm_aad(&gcm, NULL, 0);
    aes_gcm_encrypt(&gcm, ctx->session_buffer, SESSION_BUFFER_SIZE, ctx->session_buffer);
    msel_memset(&gcm, 0, sizeof(gcm));
  }
  {
    /* Format the packet */
    uint32_t length = 0;
    msel_memset(pkt->data, 0, FFS_DATA_SIZE);
    /* [CR_CIPHERTEXT] */
    err = ChatResponse_serialize(pkt->data, FFS_DATA_SIZE, &length, CR_CIPHERTEXT);
    if (err) goto cleanup;
    /* SESSION_BUFFER_SIZE bytes of uninterpreted data */
    msel_memcpy(&pkt->data[length], ctx->session_buffer, SESSION_BUFFER_SIZE);
    length += SESSION_BUFFER_SIZE;
  }

  /* Advance the key and IV */
  sha256_hash(ctx->sessions[channel].send_key, SHA256_OUTPUT_LEN, ctx->sessions[channel].send_key);
  sha256_hash(ctx->sessions[channel].send_iv, SHA256_OUTPUT_LEN, ctx->sessions[channel].send_iv);

  /* Send the response */
  while (msel_svc(MSEL_SVC_FFS_SESSION_SEND, pkt) != MSEL_OK) 
      msel_svc(MSEL_SVC_YIELD, NULL);

  cleanup:
  return err;
}

/** @brief Process an incoming packet.
 *
 * An incoming packet consists an uninterpreted array of SESSION_BUFFER_SIZE bytes.
 * [SESSION_BUFFER_SIZE bytes of uninterpreted data]
 *
 * On success, the response is one of
 * * [CR_FOUND][uint32 seek position][(uint16) CHAT_PUBKEY_LEN][byte array of length CHAT_PUBKEY_LEN]
 * * [CR_EXCHANGED][uint32 channel identifier]
 * * [CR_PLAINTEXT][uint32 channel identifier][MESSAGE_SIZE bytes of uninterpreted data]
 * * [CR_NEVERMIND]
 *
 * @param ctx Non-null. The chat context.
 * @param pkt Non-null. The incoming login packet. On success, this
 *            pointer re-used to format the outgoing reply message.
 * @return 0 on success.
 */
int chat_process_incoming(chat_context_t *ctx, ffs_packet_t *pkt, uint32_t *pos) {
  int err = 0;

  /* indicates how we are returning */
  enum ChatResponse ret = CR_ERROR;

  /* pubkey is used for seek and exchange packets */
  uint8_t pubkey[ECC_POINT_LEN];
  msel_memset(pubkey, 0, ECC_POINT_LEN);

  /* channel is used for exchange and ciphertext packets */
  uint32_t channel = SESSIONS_SIZE;

  /* Is this a seek packet? */
  for (uint32_t i = 0; i < ctx->seeking_active_count; i++) {
    msel_memset(ctx->session_buffer, 0, SESSION_BUFFER_SIZE);
    {
      aes_gcm_ctx_t gcm;
      msel_memset(&gcm, 0, sizeof(gcm));
      aes_gcm_setkey(&gcm, AES_GCM_128, ctx->seeking[i].key, ctx->seeking[i].iv, SHA256_OUTPUT_LEN);
      aes_gcm_decrypt(&gcm, &pkt->data[*pos], SESSION_BUFFER_SIZE, ctx->session_buffer);
      msel_memset(&gcm, 0, sizeof(gcm));
    }
    uint32_t decode_pos = 0;
    /* [SHA256_OUTPUT_LEN nonce] */
    decode_pos += SHA256_OUTPUT_LEN;
    /* [CR_SEEK] */
    enum ChatResponse magic = CR_ERROR;
    ChatResponse_deserialize(ctx->session_buffer, SESSION_BUFFER_SIZE, &decode_pos, &magic);
    if (CR_SEEK == magic) {
      /* [SHA256_OUTPUT_LEN The pubkey hash] */
      if (!msel_memcmp(&ctx->session_buffer[decode_pos], ctx->key_pub_hash, SHA256_OUTPUT_LEN))
        goto not_for_us;
      decode_pos += SHA256_OUTPUT_LEN;
      /* [ECC_POINT_LEN The pubkey itself] */
      msel_memcpy(pubkey, &ctx->session_buffer[decode_pos], ECC_POINT_LEN);
      decode_pos += ECC_POINT_LEN;
      /* Format the response */
      uint32_t length = 0;
      msel_memset(pkt->data, 0, FFS_DATA_SIZE);
      /* [CR_FOUND] */
      err = ChatResponse_serialize(pkt->data, FFS_DATA_SIZE, &length, CR_FOUND);
      if (err) goto cleanup;
      /* [uint32 seek position] */
      err = uint32_serialize(pkt->data, FFS_DATA_SIZE, &length, i);
      if (err) goto cleanup;
      /* [(uint16) CHAT_PUBKEY_LEN] */
      err = uint16_serialize(pkt->data, FFS_DATA_SIZE, &length, CHAT_PUBKEY_LEN);
      if (err) goto cleanup;
      /* [CHAT_PUBKEY_LEN-byte ecc point] */
      err = uint8_serialize(pkt->data, FFS_DATA_SIZE, &length, pubkey[0]);
      if (err) goto cleanup;
      msel_memcpy(&pkt->data[length], &pubkey[ECC_POINT_LEN - (CHAT_PUBKEY_LEN - 1)], (CHAT_PUBKEY_LEN - 1));
      length += CHAT_PUBKEY_LEN - 1;
      ret = CR_FOUND;
      goto respond;
    }
  }

  /* Is this an exchange packet? */
  for (int i = 0; i < SESSIONS_SIZE; i++) {
    if (CSS_LISTENING != ctx->sessions[i].state)
      continue;
    msel_memset(ctx->session_buffer, 0, SESSION_BUFFER_SIZE);
    {
      aes_gcm_ctx_t gcm;
      msel_memset(&gcm, 0, sizeof(gcm));
      aes_gcm_setkey(&gcm, AES_GCM_128, ctx->sessions[i].xchg_key, ctx->sessions[i].xchg_iv, SHA256_OUTPUT_LEN);
      aes_gcm_decrypt(&gcm, &pkt->data[*pos], SESSION_BUFFER_SIZE, ctx->session_buffer);
      msel_memset(&gcm, 0, sizeof(gcm));
    }
    uint32_t decode_pos = 0;
    /* [SHA256_OUTPUT_LEN nonce] */
    decode_pos += SHA256_OUTPUT_LEN;
    /* [CR_EXCHANGE] */
    enum ChatResponse magic = CR_ERROR;
    ChatResponse_deserialize(ctx->session_buffer, SESSION_BUFFER_SIZE, &decode_pos, &magic);
    if (CR_EXCHANGE == magic) {
      /* [SHA256_OUTPUT_LEN The pubkey hash] */
      if (!msel_memcmp(&ctx->session_buffer[decode_pos], ctx->sessions[i].local_key_pub_hash, SHA256_OUTPUT_LEN))
        goto not_for_us;
      decode_pos += SHA256_OUTPUT_LEN;
      /* [ECC_POINT_LEN The pubkey itself] */
      msel_memset(pubkey, 0, ECC_POINT_LEN);
      msel_memcpy(pubkey, &ctx->session_buffer[decode_pos], ECC_POINT_LEN);
      decode_pos += ECC_POINT_LEN;

      channel = i;

      /* Format the response */
      uint32_t length = 0;
      msel_memset(pkt->data, 0, FFS_DATA_SIZE);
      /* [CR_EXCHANGED] */
      err = ChatResponse_serialize(pkt->data, FFS_DATA_SIZE, &length, CR_EXCHANGED);
      if (err) goto cleanup;
      /* [uint32 channel identifier] */
      err = uint32_serialize(pkt->data, FFS_DATA_SIZE, &length, channel);
      if (err) goto cleanup;

      ret = CR_EXCHANGED;
      goto respond;
    }
  }

  /* Is this a ciphertext packet? */
  for (int i = 0; i < SESSIONS_SIZE; i++) {
    if (CSS_OPEN != ctx->sessions[i].state)
      continue;
    msel_memset(ctx->session_buffer, 0, SESSION_BUFFER_SIZE);
    {
      aes_gcm_ctx_t gcm;
      msel_memset(&gcm, 0, sizeof(gcm));
      aes_gcm_setkey(&gcm, AES_GCM_128, ctx->sessions[i].recv_key, ctx->sessions[i].recv_iv, SHA256_OUTPUT_LEN);
      aes_gcm_decrypt(&gcm, &pkt->data[*pos], SESSION_BUFFER_SIZE, ctx->session_buffer);
      msel_memset(&gcm, 0, sizeof(gcm));
    }
    uint32_t decode_pos = 0;
    /* [SHA256_OUTPUT_LEN nonce] */
    decode_pos += SHA256_OUTPUT_LEN;
    /* [CR_CIPHERTEXT] */
    enum ChatResponse magic = CR_ERROR;
    ChatResponse_deserialize(ctx->session_buffer, SESSION_BUFFER_SIZE, &decode_pos, &magic);
    if (!err && CR_CIPHERTEXT == magic) {
      /* [MESSAGE_SIZE The message] */
      uint8_t message[MESSAGE_SIZE];
      msel_memcpy(message, &ctx->session_buffer[decode_pos], MESSAGE_SIZE);
      decode_pos += MESSAGE_SIZE;

      channel = i;

      /* Format the response */
      uint32_t length = 0;
      msel_memset(pkt->data, 0, FFS_DATA_SIZE);
      /* [CR_PLAINTEXT] */
      err = ChatResponse_serialize(pkt->data, FFS_DATA_SIZE, &length, CR_PLAINTEXT);
      if (err) goto cleanup;
      /* [uint32 channel identifier] */
      err = uint32_serialize(pkt->data, FFS_DATA_SIZE, &length, channel);
      if (err) goto cleanup;
      /* [MESSAGE_SIZE bytes of uninterpreted data] */
      msel_memcpy(&pkt->data[length], message, MESSAGE_SIZE);
      length += MESSAGE_SIZE;

      ret = CR_PLAINTEXT;
      goto respond;
    }
  }

  /* Apparently it is not for us */
  not_for_us:
  {
      /* Format the response */
      uint32_t length = 0;
      msel_memset(pkt->data, 0, FFS_DATA_SIZE);
      /* [CR_NEVERMIND] */
      err = ChatResponse_serialize(pkt->data, FFS_DATA_SIZE, &length, CR_NEVERMIND);
      if (err) goto cleanup;
  }

  respond:
  /* Update our state */
  if (CR_EXCHANGED == ret) {
    /* Get the hash of the shared ECC point */
    uint8_t common_hash[SHA256_OUTPUT_LEN];
    for (int i = 0; i < 2; i++)
    {
      uint8_t common_pt[ECC_POINT_LEN];
      msel_memcpy(common_pt, pubkey, ECC_POINT_LEN);
      ecc_ctx_t ecc_ctx;
      ecc_ctx.scalar = ctx->sessions[channel].local_key_prv;
      ecc_ctx.point = common_pt;
      // msel_svc(MSEL_SVC_ECC, &ecc_ctx);
      for (int i = 0; i < ECC_POINT_LEN; i++)
        ecc_ctx.point[i] ^= ecc_ctx.scalar[i];
      sha256_hash(common_pt, ECC_POINT_LEN, common_hash);
    }

    kdf_getkey(chat_key, sizeof(chat_key), pubkey, ECC_POINT_LEN, common_hash, ctx->sessions[channel].send_key);
    kdf_getkey(chat_iv, sizeof(chat_iv), pubkey, ECC_POINT_LEN, common_hash, ctx->sessions[channel].send_iv);
    kdf_getkey(chat_key, sizeof(chat_key), ctx->sessions[channel].local_key_pub, ECC_POINT_LEN, common_hash, ctx->sessions[channel].recv_key);
    kdf_getkey(chat_iv, sizeof(chat_iv), ctx->sessions[channel].local_key_pub, ECC_POINT_LEN, common_hash, ctx->sessions[channel].recv_iv);

    /* Update the channel's state */
    ctx->sessions[channel].state = CSS_OPEN;

    msel_memset(pubkey, 0, ECC_POINT_LEN);
    msel_memset(common_hash, 0, SHA256_OUTPUT_LEN);
  }
  if (CR_PLAINTEXT == ret) {
    /* Advance the key and IV */
    sha256_hash(ctx->sessions[channel].recv_key, SHA256_OUTPUT_LEN, ctx->sessions[channel].recv_key);
    sha256_hash(ctx->sessions[channel].recv_iv, SHA256_OUTPUT_LEN, ctx->sessions[channel].recv_iv);
  }
  /* Send the response */
  while (msel_svc(MSEL_SVC_FFS_SESSION_SEND, pkt) != MSEL_OK) 
      msel_svc(MSEL_SVC_YIELD, NULL);

  cleanup:
  return err;
}

void chat_process_command(chat_context_t *ctx, ffs_packet_t *pkt) {
  int err = 0;

  /* Parse the packet */
  enum ChatCommand cmd = CC_LOGOUT;
  uint32_t pos = 0;
  err = ChatCommand_deserialize(pkt->data, FFS_DATA_SIZE, &pos, &cmd);
  if (err) goto cleanup;

  switch (cmd) {
    case CC_LOGOUT:
      err = chat_process_logout(ctx, pkt, &pos);
      break;
    case CC_SETSEEK:
      err = chat_process_setseek(ctx, pkt, &pos);
      break;
    case CC_GETSEEK:
      err = chat_process_getseek(ctx, pkt, &pos);
      break;
    case CC_GET_CHANNEL:
      err = chat_process_getchannel(ctx, pkt, &pos);
      break;
    case CC_CLOSE_CHANNEL:
      err = chat_process_closechannel(ctx, pkt, &pos);
      break;
    case CC_GET_EXCHANGE:
      err = chat_process_getexchange(ctx, pkt, &pos);
      break;
    case CC_ENCRYPT:
      err = chat_process_encrypt(ctx, pkt, &pos);
      break;
    case CC_INCOMING:
      err = chat_process_incoming(ctx, pkt, &pos);
      break;
  }

  cleanup:
  if (err) {
    uint32_t length = 0;
    msel_memset(pkt->data, 0, FFS_DATA_SIZE);
    if (!ChatResponse_serialize(pkt->data, FFS_DATA_SIZE, &length, CR_ERROR)) {
      while (msel_svc(MSEL_SVC_FFS_SESSION_SEND, pkt) != MSEL_OK) 
          msel_svc(MSEL_SVC_YIELD, NULL);
    }
  }
}

chat_context_t *chat_context_create() {
  chat_context_t *ret = msel_malloc(sizeof(chat_context_t));
  if (NULL == ret)
    goto cleanup;
  msel_memset(ret, 0, sizeof(*ret));
  ret->state = CS_AWAITING_LOGIN;

  cleanup:
  return ret;
}

void chat_context_destroy(chat_context_t **p) {
  if (NULL != *p) {
    msel_free(*p);
    *p = NULL;
  }
}

void urchat_task(void *arg, const size_t arg_sz) {
  chat_context_t *ctx = NULL;
  ffs_packet_t *pkt = NULL;
  ctx = chat_context_create();
  pkt = msel_malloc(sizeof(ffs_packet_t));
  if (NULL == ctx || NULL == pkt)
    goto cleanup;

  while (CS_SHOULD_EXIT != ctx->state) {
    msel_memset(pkt, 0, sizeof(*pkt));
    msel_svc(MSEL_SVC_FFS_SESSION_RECV, pkt);
    if (0 != pkt->session) {
      switch (ctx->state) {
        case CS_AWAITING_LOGIN:
          chat_process_login(ctx, pkt);
          break;
        case CS_AWAITING_COMMANDS:
          chat_process_command(ctx, pkt);
          break;
        case CS_SHOULD_EXIT:
          goto escape;
          break;
      }
    }
    else {
      /* yield, it's polite */
      msel_svc(MSEL_SVC_YIELD, NULL);
    }
  }

  escape:

  cleanup:
  if (NULL != ctx)
    chat_context_destroy(&ctx);
  if (NULL != pkt)
    msel_free(pkt);
}

