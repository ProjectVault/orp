/** @file xts_encryptor.c
 *
 *  This file encodes the state machine for the ORP XTS filesystem encryption demo
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
#include <crypto/aes_xts.h>
#include <xtsProto.h>
#include <tidl.h>

/** @brief XTS encryptor task endpoint
 *   Purpose: XTS encryption demo application
 *   Endpoint "xts\n\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
 */
void xts_encryptor_task(void *arg, const size_t arg_sz);
const uint8_t xts_encryptor_endpoint[32] =
  { 0x78, 0x74, 0x73, 0xa, 0x0, 0x0, 0x0, 0x0,
    0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,
    0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0,
    0x0,  0x0,  0x0,  0x0,  0x0, 0x0, 0x0, 0x0
  };

/** @brief Possible application states */
typedef enum {
  XTS_APP_WAITING,
  XTS_APP_RUNNING,
  XTS_APP_SHUTDOWN
} xts_encryptor_state_t;

/** @brief Stateful application information */
typedef struct {
  xts_encryptor_state_t state;
  aes_xts_ctx_t xts;
  uint8_t* key;
  uint64_t block_size;
  uint32_t block_count;
  enum XtsAlgo algo;
  uint8_t* output;
} xts_encryptor_ctx_t;

static const uint8_t xts_master[] = "XTS Encryptor master key";
static const uint8_t xts_1[] = "XTS Encryptor 1";
static const uint8_t xts_2[] = "XTS Encryptor 2";

static const int TIDL_ERROR = -1;
static const int UNSUPPORTED_ERROR = -2;

/** @brief Initialize the XTS encryptor application
 *
 *  @param ctx A pointer to the encryptor context to be filled in
 *  @param pkt A pointer to the input message
 */
void xts_encryptor_init(xts_encryptor_ctx_t *ctx, ffs_packet_t *pkt)
{
    // Process the incoming command: structure is [algo|block-size|key]
    // algo is XTS_[128|256] (only supports 128)
    uint32_t pos = 0; int err;

	// Get the selected algorithm
    err = XtsAlgo_deserialize(pkt->data, FFS_DATA_SIZE, &pos, &ctx->algo);
    if (err) goto cleanup;

	// Get the block-size and compute the block-count:
	// block_size is the size of the incoming data block
	// block_count is the number of AES blocks that make up the incoming data block
	err = uint64_deserialize(pkt->data, FFS_DATA_SIZE, &pos, &ctx->block_size);
	if (err) goto cleanup;
	ctx->block_count = ctx->block_size / AES_BLOCK_SIZE;
	ctx->output = msel_malloc(ctx->block_size);

    // Initialize the keys and change state to 'running'
	msel_memset(&ctx->xts, 0, sizeof(aes_xts_ctx_t));
	switch (ctx->algo)
	{
		case XTS_128: 
			ctx->key = msel_malloc(AES_XTS_128_KEY_SIZE); 
			break;
		case XTS_256: 
			err = UNSUPPORTED_ERROR;
			goto cleanup;
			break;
	}

	kdf_getkey(xts_master, sizeof(xts_master), xts_1, sizeof(xts_1), pkt->data + pos, ctx->key);
	aes_xts_setkey(&ctx->xts, ctx->algo, ctx->key);
	ctx->state = XTS_APP_RUNNING;
	goto cleanup;

cleanup:
    // Prepare the response packet
    pos = 0; msel_memset(pkt->data, 0, FFS_DATA_SIZE);
    if (err == TIDL_ERROR) 
		XtsResponse_serialize(pkt->data, FFS_DATA_SIZE, &pos, XTS_ERROR);
	else if (err == UNSUPPORTED_ERROR)
		XtsResponse_serialize(pkt->data, FFS_DATA_SIZE, &pos, XTS_UNSUPPORTED);
    else XtsResponse_serialize(pkt->data, FFS_DATA_SIZE, &pos, XTS_OK);

    while (msel_svc(MSEL_SVC_SESSION_SEND, pkt) != MSEL_OK) 
        msel_svc(MSEL_SVC_YIELD, NULL);
}

/** @brief Encrypt/decrypt incoming data
 *
 *  @param ctx A pointer to the encryptor context to be filled in
 *  @param pkt A pointer to the input message
 */
void xts_encryptor_exec(xts_encryptor_ctx_t *ctx, ffs_packet_t *pkt)
{
    // Parse the next incoming packet
    enum XtsCommand cmd = XTS_SHUTDOWN; uint32_t pos = 0;
    int err = XtsCommand_deserialize(pkt->data, FFS_DATA_SIZE, &pos, &cmd);
    if (err) goto cleanup;

	// Get the sequence ID of the block to encrypt/decrypt
	uint64_t sequence;
	err = uint64_deserialize(pkt->data, FFS_DATA_SIZE, &pos, &sequence);
	if (err) goto cleanup;

    // Encrypt/decrypt or clean up
    switch (cmd)
    {
		// Don't want to overwrite the incoming data since the packets have different forms,
		// so just copy to a new buffer for now
        case XTS_ENCRYPT:
            aes_xts_encrypt(&ctx->xts, pkt->data + pos, ctx->block_count, sequence, ctx->output);
            goto cleanup;
        case XTS_DECRYPT:
            aes_xts_decrypt(&ctx->xts, pkt->data + pos, ctx->block_count, sequence, ctx->output);
            goto cleanup;
        case XTS_SHUTDOWN:
			msel_free(ctx->key);
			msel_free(ctx->output);
            msel_memset(ctx, 0, sizeof(*ctx));
            msel_memset(pkt, 0, sizeof(*pkt));
            ctx->state = XTS_APP_WAITING;
            goto cleanup;
    }

cleanup:

    pos = 0; msel_memset(pkt->data, 0, FFS_DATA_SIZE);
    if (err) XtsResponse_serialize(pkt->data, FFS_DATA_SIZE, &pos, XTS_ERROR);
    else XtsResponse_serialize(pkt->data, FFS_DATA_SIZE, &pos, XTS_OK);

	// Now we've formed the header for the outgoing packet, so copy in the data
	msel_memcpy(pkt->data + pos, ctx->output, ctx->block_size);

    while (msel_svc(MSEL_SVC_SESSION_SEND, pkt) != MSEL_OK) 
        msel_svc(MSEL_SVC_YIELD, NULL);
}

/** @brief xts encryptor task main
 *
 *  @param arg unused
 *  @param arg_sz unused
 */
void xts_encryptor_task(void *arg, const size_t arg_sz) {

    // initialize our data structures
    xts_encryptor_ctx_t *ctx = msel_malloc(sizeof(xts_encryptor_ctx_t));
    ffs_packet_t *pkt = msel_malloc(sizeof(ffs_packet_t));

    // Task main loop.  There are three states that the task can be in:
    //   1) XTS_WAITING: the user has not supplied a key to initialize with
    //   2) XTS_RUNNING: doing encryption/decryption
    //   3) XTS_SHUTDOWN: clean everything up and end the task
    //
    // Proper usage: 
    //   1) provide a user-supplied key and set the algo parameters
    //   2) stream data to be en-/decrypted
    //   3) send XTS_DONE
    //   4) If more data to process, goto 1
    //   5) send XTS_SHUTDOWN
    msel_memset(ctx, 0, sizeof(*ctx));
    while (1)
    {
        msel_memset(pkt, 0, sizeof(*pkt));
        msel_svc(MSEL_SVC_SESSION_RECV, pkt);
        if (pkt->session != 0)
        {
            switch (ctx->state)
            {
                case XTS_APP_WAITING: 
                    xts_encryptor_init(ctx, pkt);
                    break;
                case XTS_APP_RUNNING: 
                    xts_encryptor_exec(ctx, pkt);
                    break;
                case XTS_APP_SHUTDOWN:
                    goto shutdown;
            }
        }
        msel_svc(MSEL_SVC_YIELD, NULL);
    }

shutdown:
    msel_free(ctx);
    msel_free(pkt);
}
