/** @file encryptor.c
 *
 *  This file gives the state machine for the ORP demo encryption application
 */
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



#include <msel/ffs.h>
#include <msel/syscalls.h>
#include <msel/malloc.h>
#include <msel/stdc.h>
#include <crypto/kdf.h>
#include <crypto/aes_gcm.h>
#include <encProto.h>

/** @brief encryptor task application endpoint 
 *   Purpose: Bulk encryptor demo application
 *   Endpoint "enc\n\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
 */
void encryptor_task(void *arg, const size_t arg_sz);
const uint8_t encryptor_endpoint[32] =
  { 0x65, 0x6e, 0x63, 0xa, 0x0, 0x0, 0x0, 0x0,
    0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,
    0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,
    0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0
  };

/** @brief Possible application states */
typedef enum {
  ENC_WAITING,
  ENC_RUNNING,
  ENC_SHUTDOWN
} encryptor_state_t;

/** @brief Stateful application information */
typedef struct {
  encryptor_state_t state;
  aes_gcm_ctx_t gcm;
  uint8_t* key; 
  uint8_t iv[SHA256_OUTPUT_LEN];
  enum EncryptorMode mode;
  enum EncryptorAlgo algo;
} encryptor_ctx_t;

static const uint8_t enc_master[] = "Encryptor master key";
static const uint8_t enc_1[] = "Encryptor 1";
static const uint8_t enc_2[] = "Encryptor 2";

static const int TIDL_ERROR = -1;
static const int UNSUPPORTED_ERROR = -2;

/** @brief Initialize the encryptor application
 *
 *  @param ctx A pointer to the encryptor context to be filled in
 *  @param pkt A pointer to the input message
 */
void encryptor_init(encryptor_ctx_t *ctx, ffs_packet_t *pkt)
{
    // Process the incoming command: structure is [mode|algo|key]
    // algo is AES_GCM_[128|196|256], 
    // mode is EC_ENCRYPT/EC_DECRYPT/EC_SHUTDOWN
    uint32_t pos = 0; int err;
    err = EncryptorMode_deserialize(pkt->data, FFS_DATA_SIZE, &pos, &ctx->mode);
    if (err) goto cleanup;
    // If the user said to kill the task, then change state and return
    if (ctx->mode == EC_SHUTDOWN)
    {
        ctx->state = ENC_SHUTDOWN;
        goto cleanup;
    }

    err = EncryptorAlgo_deserialize(pkt->data, FFS_DATA_SIZE, &pos, &ctx->algo);
    if (err) goto cleanup;

    // Otherwise, initialize the keys and change state to 'running'
	msel_memset(&ctx->gcm, 0, sizeof(aes_gcm_ctx_t));
	switch (ctx->algo)
	{
		case AES_GCM_128: ctx->key = msel_malloc(AES_GCM_128_KEY_SIZE); break;
		case AES_GCM_192: err = UNSUPPORTED_ERROR; goto cleanup;
		case AES_GCM_256: err = UNSUPPORTED_ERROR; goto cleanup;
	}
	kdf_getkey(enc_master, sizeof(enc_master), enc_1, sizeof(enc_1), pkt->data + pos, ctx->key);
	kdf_getkey(enc_master, sizeof(enc_master), enc_2, sizeof(enc_2), pkt->data + pos, ctx->iv);
	aes_gcm_setkey(&ctx->gcm, ctx->algo, ctx->key, ctx->iv, SHA256_OUTPUT_LEN);
	ctx->state = ENC_RUNNING;
	goto cleanup;

cleanup:
    // Prepare the response packet
    pos = 0; msel_memset(pkt->data, 0, FFS_DATA_SIZE);
    if (err == TIDL_ERROR) 
		EncryptorResponse_serialize(pkt->data, FFS_DATA_SIZE, &pos, ER_ERROR);
	else if (err == UNSUPPORTED_ERROR)
		EncryptorResponse_serialize(pkt->data, FFS_DATA_SIZE, &pos, ER_UNSUPPORTED);
    else EncryptorResponse_serialize(pkt->data, FFS_DATA_SIZE, &pos, ER_OK);

    while (msel_svc(MSEL_SVC_FFS_SESSION_SEND, pkt) != MSEL_OK) 
        msel_svc(MSEL_SVC_YIELD, NULL);
}

/** @brief Process incoming data
 *
 *  @param ctx A pointer to the encryptor context to be filled in
 *  @param pkt A pointer to the input message
 */
void encryptor_exec(encryptor_ctx_t *ctx, ffs_packet_t *pkt)
{
    // Parse the next incoming packet
    enum EncryptorCommand cmd = EC_DONE; uint32_t pos = 0;
    int err = EncryptorCommand_deserialize(pkt->data, FFS_DATA_SIZE, &pos, &cmd);
    uint32_t data_sz = FFS_DATA_SIZE - pos;

    if (err) goto cleanup;

    // Encrypt/decrypt or clean up
    switch (cmd)
    {
        // When encrypting/decrypting, leave space at the beginning for the status
        case EC_DATA:
            if (ctx->mode == EC_ENCRYPT)
                aes_gcm_encrypt(&ctx->gcm, pkt->data + pos, data_sz, pkt->data + pos);
            else if (ctx->mode == EC_DECRYPT)
                aes_gcm_decrypt(&ctx->gcm, pkt->data + pos, data_sz, pkt->data + pos);
            goto cleanup;
        case EC_DONE:
			msel_free(ctx->key);
            msel_memset(ctx, 0, sizeof(*ctx));
            msel_memset(pkt, 0, sizeof(*pkt));
            ctx->state = ENC_WAITING;
            goto cleanup;
    }

cleanup:

    // Set the status of the response, but don't overwrite the encrypted/decrypted data
    pos = 0;
    if (err) EncryptorResponse_serialize(pkt->data, FFS_DATA_SIZE, &pos, ER_ERROR);
    else EncryptorResponse_serialize(pkt->data, FFS_DATA_SIZE, &pos, ER_OK);

    while (msel_svc(MSEL_SVC_FFS_SESSION_SEND, pkt) != MSEL_OK) 
        msel_svc(MSEL_SVC_YIELD, NULL);
}

/** @brief bulk encryptor task main
 *
 *  @param arg unused
 *  @param arg_sz unused
 */
void encryptor_task(void *arg, const size_t arg_sz) {

    // initialize our data structures
    encryptor_ctx_t *ctx = msel_malloc(sizeof(encryptor_ctx_t));
    ffs_packet_t *pkt = msel_malloc(sizeof(ffs_packet_t));

    // Task main loop.  There are three states that the task can be in:
    //   1) ENC_WAITING: the user has not supplied a key to initialize with
    //   2) ENC_RUNNING: doing encryption/decryption
    //   3) ENC_SHUTDOWN: clean everything up and end the task
    //
    // Proper usage: 
    //   1) provide a user-supplied key and set the algo parameters
    //   2) stream data to be en-/decrypted
    //   3) send EC_DONE
    //   4) If more data to process, goto 1
    //   5) send EC_SHUTDOWN
    msel_memset(ctx, 0, sizeof(*ctx));
    while (1)
    {
        msel_memset(pkt, 0, sizeof(*pkt));
        msel_svc(MSEL_SVC_FFS_SESSION_RECV, pkt);
        if (pkt->session != 0)
        {
            switch (ctx->state)
            {
                case ENC_WAITING: 
                    encryptor_init(ctx, pkt);
                    break;
                case ENC_RUNNING: 
                    encryptor_exec(ctx, pkt);
                    break;
                case ENC_SHUTDOWN:
                    goto shutdown;
            }
        }
        msel_svc(MSEL_SVC_YIELD, NULL);
    }

shutdown:
    msel_free(ctx);
    msel_free(pkt);
}
