/**
 * @file xtsProto.c
 */


#include <string.h>
#include <stdlib.h>
#include <xtsProto.h>


int XtsAlgo_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, enum XtsAlgo in) {
  int ret = 0;
  
  switch (in) {
    case XTS_128:
      ret = int32_serialize(out, out_len, pos, 0);
      break;
    case XTS_256:
      ret = int32_serialize(out, out_len, pos, 2);
      break;
    default: ret = -1;
  }
  
  return ret;
}

int XtsAlgo_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, enum XtsAlgo *out) {
  int ret = 0;
  
  int32_t temp = 0;
  ret = int32_deserialize(in, in_len, pos, &temp);
  if (ret) return ret;
  switch (temp) {
    case 0:
      *out = XTS_128;
      break;
    case 2:
      *out = XTS_256;
      break;
    default: ret = -1;
  }
  
  return ret;
}


int XtsCommand_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, enum XtsCommand in) {
  int ret = 0;
  
  switch (in) {
    case XTS_ENCRYPT:
      ret = int32_serialize(out, out_len, pos, 16);
      break;
    case XTS_DECRYPT:
      ret = int32_serialize(out, out_len, pos, 32);
      break;
    case XTS_SHUTDOWN:
      ret = int32_serialize(out, out_len, pos, 48);
      break;
    default: ret = -1;
  }
  
  return ret;
}

int XtsCommand_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, enum XtsCommand *out) {
  int ret = 0;
  
  int32_t temp = 0;
  ret = int32_deserialize(in, in_len, pos, &temp);
  if (ret) return ret;
  switch (temp) {
    case 16:
      *out = XTS_ENCRYPT;
      break;
    case 32:
      *out = XTS_DECRYPT;
      break;
    case 48:
      *out = XTS_SHUTDOWN;
      break;
    default: ret = -1;
  }
  
  return ret;
}


int XtsResponse_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, enum XtsResponse in) {
  int ret = 0;
  
  switch (in) {
    case XTS_ERROR:
      ret = int32_serialize(out, out_len, pos, 255);
      break;
    case XTS_UNSUPPORTED:
      ret = int32_serialize(out, out_len, pos, 254);
      break;
    case XTS_OK:
      ret = int32_serialize(out, out_len, pos, 1);
      break;
    default: ret = -1;
  }
  
  return ret;
}

int XtsResponse_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, enum XtsResponse *out) {
  int ret = 0;
  
  int32_t temp = 0;
  ret = int32_deserialize(in, in_len, pos, &temp);
  if (ret) return ret;
  switch (temp) {
    case 255:
      *out = XTS_ERROR;
      break;
    case 254:
      *out = XTS_UNSUPPORTED;
      break;
    case 1:
      *out = XTS_OK;
      break;
    default: ret = -1;
  }
  
  return ret;
}



