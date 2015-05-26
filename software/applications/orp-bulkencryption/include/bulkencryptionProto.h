/**
 * @file bulkencryptionProto.h
 */

#ifndef __BULKENCRYPTIONPROTO_H__
#define __BULKENCRYPTIONPROTO_H__


#include <tidl.h>


/* Start forward declarations */

enum BulkEncryptionCommand;
enum BulkEncryptionResponse;
struct GenDHPair_Cmd;
struct GenDHPair_Res;
struct DHExchange_Cmd;
struct DHExchange_Res;
enum GCM_Mode;
struct GcmInit_Cmd;
struct GcmInit_Res;
struct GcmDoBlock_Cmd;
struct GcmDoBlock_Res;

/* End forward declarations */


/* Start array length constants */

#define GENDHPAIR_CMD_SEED_LENGTH 32
#define GENDHPAIR_RES_PUBKEY_LENGTH 128
#define DHEXCHANGE_CMD_REMOTE_PUBKEY_LENGTH 128
#define GCMDOBLOCK_CMD_BLOCK_LENGTH 16
#define GCMDOBLOCK_RES_BLOCK_LENGTH 16

/* End array length constants */


/* Start type definitions */

enum BulkEncryptionCommand {
  BEC_LOGOUT = 16,
  BEC_GEN_DHPAIR = 32,
  BEC_DH_EXCHANGE = 48,
  BEC_GCM_INIT = 80,
  BEC_GCM_DOBLOCK = 96,
};

enum BulkEncryptionResponse {
  BER_ERROR = 255,
  BER_GOODBYE = 17,
  BER_DHPAIR = 33,
  BER_DH_EXCHANGED = 49,
  BER_GCM_READY = 81,
  BER_GCM_BLOCKS = 97,
};

struct GenDHPair_Cmd {
  uint8_t seed[GENDHPAIR_CMD_SEED_LENGTH];
};

struct GenDHPair_Res {
  uint8_t pubkey[GENDHPAIR_RES_PUBKEY_LENGTH];
  uint8_t privkey_idx;
};

struct DHExchange_Cmd {
  uint8_t remote_pubkey[DHEXCHANGE_CMD_REMOTE_PUBKEY_LENGTH];
  uint8_t local_privkey_idx;
};

struct DHExchange_Res {
  uint8_t shared_idx;
};

enum GCM_Mode {
  GCM128_ENCRYPT = 16,
  GCM128_DECRYPT = 32,
};

struct GcmInit_Cmd {
  enum GCM_Mode mode;
  uint8_t shared_idx;
};

struct GcmInit_Res {
  uint8_t gcm_idx;
};

struct GcmDoBlock_Cmd {
  uint8_t block[GCMDOBLOCK_CMD_BLOCK_LENGTH];
  uint8_t gcm_idx;
};

struct GcmDoBlock_Res {
  uint8_t block[GCMDOBLOCK_RES_BLOCK_LENGTH];
};


/* End type definitions */


/* Start function prototypes */

int BulkEncryptionCommand_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, enum BulkEncryptionCommand in);
int BulkEncryptionCommand_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, enum BulkEncryptionCommand *out);

int BulkEncryptionResponse_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, enum BulkEncryptionResponse in);
int BulkEncryptionResponse_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, enum BulkEncryptionResponse *out);

void GenDHPair_Cmd_init(struct GenDHPair_Cmd *in);
void GenDHPair_Cmd_cleanup(struct GenDHPair_Cmd *in);
int GenDHPair_Cmd_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, struct GenDHPair_Cmd *in);
int GenDHPair_Cmd_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, struct GenDHPair_Cmd *out);

void GenDHPair_Res_init(struct GenDHPair_Res *in);
void GenDHPair_Res_cleanup(struct GenDHPair_Res *in);
int GenDHPair_Res_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, struct GenDHPair_Res *in);
int GenDHPair_Res_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, struct GenDHPair_Res *out);

void DHExchange_Cmd_init(struct DHExchange_Cmd *in);
void DHExchange_Cmd_cleanup(struct DHExchange_Cmd *in);
int DHExchange_Cmd_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, struct DHExchange_Cmd *in);
int DHExchange_Cmd_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, struct DHExchange_Cmd *out);

void DHExchange_Res_init(struct DHExchange_Res *in);
void DHExchange_Res_cleanup(struct DHExchange_Res *in);
int DHExchange_Res_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, struct DHExchange_Res *in);
int DHExchange_Res_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, struct DHExchange_Res *out);

int GCM_Mode_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, enum GCM_Mode in);
int GCM_Mode_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, enum GCM_Mode *out);

void GcmInit_Cmd_init(struct GcmInit_Cmd *in);
void GcmInit_Cmd_cleanup(struct GcmInit_Cmd *in);
int GcmInit_Cmd_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, struct GcmInit_Cmd *in);
int GcmInit_Cmd_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, struct GcmInit_Cmd *out);

void GcmInit_Res_init(struct GcmInit_Res *in);
void GcmInit_Res_cleanup(struct GcmInit_Res *in);
int GcmInit_Res_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, struct GcmInit_Res *in);
int GcmInit_Res_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, struct GcmInit_Res *out);

void GcmDoBlock_Cmd_init(struct GcmDoBlock_Cmd *in);
void GcmDoBlock_Cmd_cleanup(struct GcmDoBlock_Cmd *in);
int GcmDoBlock_Cmd_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, struct GcmDoBlock_Cmd *in);
int GcmDoBlock_Cmd_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, struct GcmDoBlock_Cmd *out);

void GcmDoBlock_Res_init(struct GcmDoBlock_Res *in);
void GcmDoBlock_Res_cleanup(struct GcmDoBlock_Res *in);
int GcmDoBlock_Res_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, struct GcmDoBlock_Res *in);
int GcmDoBlock_Res_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, struct GcmDoBlock_Res *out);


/* End function prototypes */


#endif /* __BULKENCRYPTIONPROTO_H__ */
