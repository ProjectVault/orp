/**
 * @file bulkencryptionProto.c
 */


#include <string.h>
#include <stdlib.h>
#include <bulkencryptionProto.h>


int BulkEncryptionCommand_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, enum BulkEncryptionCommand in) {
  int ret = 0;
  
  switch (in) {
    case BEC_LOGOUT:
      ret = int32_serialize(out, out_len, pos, 16);
      break;
    case BEC_GEN_DHPAIR:
      ret = int32_serialize(out, out_len, pos, 32);
      break;
    case BEC_DH_EXCHANGE:
      ret = int32_serialize(out, out_len, pos, 48);
      break;
    case BEC_GCM_INIT:
      ret = int32_serialize(out, out_len, pos, 80);
      break;
    case BEC_GCM_DOBLOCK:
      ret = int32_serialize(out, out_len, pos, 96);
      break;
    default: ret = -1;
  }
  
  return ret;
}

int BulkEncryptionCommand_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, enum BulkEncryptionCommand *out) {
  int ret = 0;
  
  int32_t temp = 0;
  ret = int32_deserialize(in, in_len, pos, &temp);
  if (ret) return ret;
  switch (temp) {
    case 16:
      *out = BEC_LOGOUT;
      break;
    case 32:
      *out = BEC_GEN_DHPAIR;
      break;
    case 48:
      *out = BEC_DH_EXCHANGE;
      break;
    case 80:
      *out = BEC_GCM_INIT;
      break;
    case 96:
      *out = BEC_GCM_DOBLOCK;
      break;
    default: ret = -1;
  }
  
  return ret;
}


int BulkEncryptionResponse_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, enum BulkEncryptionResponse in) {
  int ret = 0;
  
  switch (in) {
    case BER_ERROR:
      ret = int32_serialize(out, out_len, pos, 255);
      break;
    case BER_GOODBYE:
      ret = int32_serialize(out, out_len, pos, 17);
      break;
    case BER_DHPAIR:
      ret = int32_serialize(out, out_len, pos, 33);
      break;
    case BER_DH_EXCHANGED:
      ret = int32_serialize(out, out_len, pos, 49);
      break;
    case BER_GCM_READY:
      ret = int32_serialize(out, out_len, pos, 81);
      break;
    case BER_GCM_BLOCKS:
      ret = int32_serialize(out, out_len, pos, 97);
      break;
    default: ret = -1;
  }
  
  return ret;
}

int BulkEncryptionResponse_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, enum BulkEncryptionResponse *out) {
  int ret = 0;
  
  int32_t temp = 0;
  ret = int32_deserialize(in, in_len, pos, &temp);
  if (ret) return ret;
  switch (temp) {
    case 255:
      *out = BER_ERROR;
      break;
    case 17:
      *out = BER_GOODBYE;
      break;
    case 33:
      *out = BER_DHPAIR;
      break;
    case 49:
      *out = BER_DH_EXCHANGED;
      break;
    case 81:
      *out = BER_GCM_READY;
      break;
    case 97:
      *out = BER_GCM_BLOCKS;
      break;
    default: ret = -1;
  }
  
  return ret;
}


void GenDHPair_Cmd_init(struct GenDHPair_Cmd *in) {
  memset(in, 0, sizeof(*in));
}

void GenDHPair_Cmd_cleanup(struct GenDHPair_Cmd *in) {
  GenDHPair_Cmd_init(in);
}

int GenDHPair_Cmd_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, struct GenDHPair_Cmd *in) {
  int ret = 0;
  
  for (int i = 0; i < GENDHPAIR_CMD_SEED_LENGTH; i++) {
    ret = uint8_serialize(out, out_len, pos, in->seed[i]);
    if (ret) return ret;
  }
  
  return ret;
}

int GenDHPair_Cmd_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, struct GenDHPair_Cmd *out) {
  int ret = 0;
  
  for (int i = 0; i < GENDHPAIR_CMD_SEED_LENGTH; i++) {
    ret = uint8_deserialize(in, in_len, pos, &out->seed[i]);
    if (ret) return ret;
  }
  
  return ret;
}


void GenDHPair_Res_init(struct GenDHPair_Res *in) {
  memset(in, 0, sizeof(*in));
}

void GenDHPair_Res_cleanup(struct GenDHPair_Res *in) {
  GenDHPair_Res_init(in);
}

int GenDHPair_Res_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, struct GenDHPair_Res *in) {
  int ret = 0;
  
  for (int i = 0; i < GENDHPAIR_RES_PUBKEY_LENGTH; i++) {
    ret = uint8_serialize(out, out_len, pos, in->pubkey[i]);
    if (ret) return ret;
  }
  ret = uint8_serialize(out, out_len, pos, in->privkey_idx);
  if (ret) return ret;
  
  return ret;
}

int GenDHPair_Res_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, struct GenDHPair_Res *out) {
  int ret = 0;
  
  for (int i = 0; i < GENDHPAIR_RES_PUBKEY_LENGTH; i++) {
    ret = uint8_deserialize(in, in_len, pos, &out->pubkey[i]);
    if (ret) return ret;
  }
  ret = uint8_deserialize(in, in_len, pos, &out->privkey_idx);
  if (ret) return ret;
  
  return ret;
}


void DHExchange_Cmd_init(struct DHExchange_Cmd *in) {
  memset(in, 0, sizeof(*in));
}

void DHExchange_Cmd_cleanup(struct DHExchange_Cmd *in) {
  DHExchange_Cmd_init(in);
}

int DHExchange_Cmd_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, struct DHExchange_Cmd *in) {
  int ret = 0;
  
  for (int i = 0; i < DHEXCHANGE_CMD_REMOTE_PUBKEY_LENGTH; i++) {
    ret = uint8_serialize(out, out_len, pos, in->remote_pubkey[i]);
    if (ret) return ret;
  }
  ret = uint8_serialize(out, out_len, pos, in->local_privkey_idx);
  if (ret) return ret;
  
  return ret;
}

int DHExchange_Cmd_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, struct DHExchange_Cmd *out) {
  int ret = 0;
  
  for (int i = 0; i < DHEXCHANGE_CMD_REMOTE_PUBKEY_LENGTH; i++) {
    ret = uint8_deserialize(in, in_len, pos, &out->remote_pubkey[i]);
    if (ret) return ret;
  }
  ret = uint8_deserialize(in, in_len, pos, &out->local_privkey_idx);
  if (ret) return ret;
  
  return ret;
}


void DHExchange_Res_init(struct DHExchange_Res *in) {
  memset(in, 0, sizeof(*in));
}

void DHExchange_Res_cleanup(struct DHExchange_Res *in) {
  DHExchange_Res_init(in);
}

int DHExchange_Res_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, struct DHExchange_Res *in) {
  int ret = 0;
  
  ret = uint8_serialize(out, out_len, pos, in->shared_idx);
  if (ret) return ret;
  
  return ret;
}

int DHExchange_Res_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, struct DHExchange_Res *out) {
  int ret = 0;
  
  ret = uint8_deserialize(in, in_len, pos, &out->shared_idx);
  if (ret) return ret;
  
  return ret;
}


int GCM_Mode_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, enum GCM_Mode in) {
  int ret = 0;
  
  switch (in) {
    case GCM128_ENCRYPT:
      ret = int32_serialize(out, out_len, pos, 16);
      break;
    case GCM128_DECRYPT:
      ret = int32_serialize(out, out_len, pos, 32);
      break;
    default: ret = -1;
  }
  
  return ret;
}

int GCM_Mode_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, enum GCM_Mode *out) {
  int ret = 0;
  
  int32_t temp = 0;
  ret = int32_deserialize(in, in_len, pos, &temp);
  if (ret) return ret;
  switch (temp) {
    case 16:
      *out = GCM128_ENCRYPT;
      break;
    case 32:
      *out = GCM128_DECRYPT;
      break;
    default: ret = -1;
  }
  
  return ret;
}


void GcmInit_Cmd_init(struct GcmInit_Cmd *in) {
  memset(in, 0, sizeof(*in));
}

void GcmInit_Cmd_cleanup(struct GcmInit_Cmd *in) {
  GcmInit_Cmd_init(in);
}

int GcmInit_Cmd_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, struct GcmInit_Cmd *in) {
  int ret = 0;
  
  ret = GCM_Mode_serialize(out, out_len, pos, in->mode);
  if (ret) return ret;
  ret = uint8_serialize(out, out_len, pos, in->shared_idx);
  if (ret) return ret;
  
  return ret;
}

int GcmInit_Cmd_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, struct GcmInit_Cmd *out) {
  int ret = 0;
  
  ret = GCM_Mode_deserialize(in, in_len, pos, &out->mode);
  if (ret) return ret;
  ret = uint8_deserialize(in, in_len, pos, &out->shared_idx);
  if (ret) return ret;
  
  return ret;
}


void GcmInit_Res_init(struct GcmInit_Res *in) {
  memset(in, 0, sizeof(*in));
}

void GcmInit_Res_cleanup(struct GcmInit_Res *in) {
  GcmInit_Res_init(in);
}

int GcmInit_Res_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, struct GcmInit_Res *in) {
  int ret = 0;
  
  ret = uint8_serialize(out, out_len, pos, in->gcm_idx);
  if (ret) return ret;
  
  return ret;
}

int GcmInit_Res_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, struct GcmInit_Res *out) {
  int ret = 0;
  
  ret = uint8_deserialize(in, in_len, pos, &out->gcm_idx);
  if (ret) return ret;
  
  return ret;
}


void GcmDoBlock_Cmd_init(struct GcmDoBlock_Cmd *in) {
  memset(in, 0, sizeof(*in));
}

void GcmDoBlock_Cmd_cleanup(struct GcmDoBlock_Cmd *in) {
  GcmDoBlock_Cmd_init(in);
}

int GcmDoBlock_Cmd_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, struct GcmDoBlock_Cmd *in) {
  int ret = 0;
  
  for (int i = 0; i < GCMDOBLOCK_CMD_BLOCK_LENGTH; i++) {
    ret = uint8_serialize(out, out_len, pos, in->block[i]);
    if (ret) return ret;
  }
  ret = uint8_serialize(out, out_len, pos, in->gcm_idx);
  if (ret) return ret;
  
  return ret;
}

int GcmDoBlock_Cmd_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, struct GcmDoBlock_Cmd *out) {
  int ret = 0;
  
  for (int i = 0; i < GCMDOBLOCK_CMD_BLOCK_LENGTH; i++) {
    ret = uint8_deserialize(in, in_len, pos, &out->block[i]);
    if (ret) return ret;
  }
  ret = uint8_deserialize(in, in_len, pos, &out->gcm_idx);
  if (ret) return ret;
  
  return ret;
}


void GcmDoBlock_Res_init(struct GcmDoBlock_Res *in) {
  memset(in, 0, sizeof(*in));
}

void GcmDoBlock_Res_cleanup(struct GcmDoBlock_Res *in) {
  GcmDoBlock_Res_init(in);
}

int GcmDoBlock_Res_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, struct GcmDoBlock_Res *in) {
  int ret = 0;
  
  for (int i = 0; i < GCMDOBLOCK_RES_BLOCK_LENGTH; i++) {
    ret = uint8_serialize(out, out_len, pos, in->block[i]);
    if (ret) return ret;
  }
  
  return ret;
}

int GcmDoBlock_Res_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, struct GcmDoBlock_Res *out) {
  int ret = 0;
  
  for (int i = 0; i < GCMDOBLOCK_RES_BLOCK_LENGTH; i++) {
    ret = uint8_deserialize(in, in_len, pos, &out->block[i]);
    if (ret) return ret;
  }
  
  return ret;
}



