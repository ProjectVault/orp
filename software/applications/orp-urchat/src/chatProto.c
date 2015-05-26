/**
 * @file chatProto.c
 */


#include <string.h>
#include <stdlib.h>
#include <chatProto.h>


int ChatCommand_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, enum ChatCommand in) {
  int ret = 0;
  
  switch (in) {
    case CC_LOGOUT:
      ret = int32_serialize(out, out_len, pos, 16);
      break;
    case CC_SETSEEK:
      ret = int32_serialize(out, out_len, pos, 32);
      break;
    case CC_GETSEEK:
      ret = int32_serialize(out, out_len, pos, 48);
      break;
    case CC_GET_CHANNEL:
      ret = int32_serialize(out, out_len, pos, 64);
      break;
    case CC_CLOSE_CHANNEL:
      ret = int32_serialize(out, out_len, pos, 80);
      break;
    case CC_GET_EXCHANGE:
      ret = int32_serialize(out, out_len, pos, 96);
      break;
    case CC_ENCRYPT:
      ret = int32_serialize(out, out_len, pos, 112);
      break;
    case CC_INCOMING:
      ret = int32_serialize(out, out_len, pos, 128);
      break;
    default: ret = -1;
  }
  
  return ret;
}

int ChatCommand_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, enum ChatCommand *out) {
  int ret = 0;
  
  int32_t temp = 0;
  ret = int32_deserialize(in, in_len, pos, &temp);
  if (ret) return ret;
  switch (temp) {
    case 16:
      *out = CC_LOGOUT;
      break;
    case 32:
      *out = CC_SETSEEK;
      break;
    case 48:
      *out = CC_GETSEEK;
      break;
    case 64:
      *out = CC_GET_CHANNEL;
      break;
    case 80:
      *out = CC_CLOSE_CHANNEL;
      break;
    case 96:
      *out = CC_GET_EXCHANGE;
      break;
    case 112:
      *out = CC_ENCRYPT;
      break;
    case 128:
      *out = CC_INCOMING;
      break;
    default: ret = -1;
  }
  
  return ret;
}


int ChatResponse_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, enum ChatResponse in) {
  int ret = 0;
  
  switch (in) {
    case CR_ERROR:
      ret = int32_serialize(out, out_len, pos, 255);
      break;
    case CR_PUBKEY:
      ret = int32_serialize(out, out_len, pos, 0);
      break;
    case CR_GOODBYE:
      ret = int32_serialize(out, out_len, pos, 17);
      break;
    case CR_SEEKUPDATED:
      ret = int32_serialize(out, out_len, pos, 33);
      break;
    case CR_SEEK:
      ret = int32_serialize(out, out_len, pos, 49);
      break;
    case CR_CHANNEL:
      ret = int32_serialize(out, out_len, pos, 65);
      break;
    case CR_CHANNEL_CLOSED:
      ret = int32_serialize(out, out_len, pos, 81);
      break;
    case CR_EXCHANGE:
      ret = int32_serialize(out, out_len, pos, 97);
      break;
    case CR_CIPHERTEXT:
      ret = int32_serialize(out, out_len, pos, 113);
      break;
    case CR_FOUND:
      ret = int32_serialize(out, out_len, pos, 129);
      break;
    case CR_EXCHANGED:
      ret = int32_serialize(out, out_len, pos, 130);
      break;
    case CR_PLAINTEXT:
      ret = int32_serialize(out, out_len, pos, 131);
      break;
    case CR_NEVERMIND:
      ret = int32_serialize(out, out_len, pos, 132);
      break;
    default: ret = -1;
  }
  
  return ret;
}

int ChatResponse_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, enum ChatResponse *out) {
  int ret = 0;
  
  int32_t temp = 0;
  ret = int32_deserialize(in, in_len, pos, &temp);
  if (ret) return ret;
  switch (temp) {
    case 255:
      *out = CR_ERROR;
      break;
    case 0:
      *out = CR_PUBKEY;
      break;
    case 17:
      *out = CR_GOODBYE;
      break;
    case 33:
      *out = CR_SEEKUPDATED;
      break;
    case 49:
      *out = CR_SEEK;
      break;
    case 65:
      *out = CR_CHANNEL;
      break;
    case 81:
      *out = CR_CHANNEL_CLOSED;
      break;
    case 97:
      *out = CR_EXCHANGE;
      break;
    case 113:
      *out = CR_CIPHERTEXT;
      break;
    case 129:
      *out = CR_FOUND;
      break;
    case 130:
      *out = CR_EXCHANGED;
      break;
    case 131:
      *out = CR_PLAINTEXT;
      break;
    case 132:
      *out = CR_NEVERMIND;
      break;
    default: ret = -1;
  }
  
  return ret;
}



