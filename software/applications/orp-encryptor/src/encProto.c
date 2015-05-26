/**
 * @file encProto.c
 */


#include <tidl.h>
#include <encProto.h>

int EncryptorAlgo_serialize(uint8_t *out, uint32_t len, uint32_t *pos, enum EncryptorAlgo in)
{
  int ret = 0;
  
  switch (in)
  {
    case EC_GCM_128:
      ret = int32_serialize(out, len, pos, 0);
      break;
    case EC_GCM_192:
      ret = int32_serialize(out, len, pos, 1);
      break;
    case EC_GCM_256:
      ret = int32_serialize(out, len, pos, 2);
      break;
    default: ret = -1;
  }
  
  return ret;
}

int EncryptorAlgo_deserialize(uint8_t *in, uint32_t len, uint32_t *pos, enum EncryptorAlgo *out)
{
  int ret = 0;
  
  int32_t v = 0;
  ret = int32_deserialize(in, len, pos, &v);
  if (ret) return ret;
  switch (v)
  {
    case 0:
      *out = EC_GCM_128;
      break;
    case 1:
      *out = EC_GCM_192;
      break;
    case 2:
      *out = EC_GCM_256;
      break;
    default: ret = -1;
  }
  
  return ret;
}


int EncryptorMode_serialize(uint8_t *out, uint32_t len, uint32_t *pos, enum EncryptorMode in)
{
  int ret = 0;
  
  switch (in)
  {
    case EC_ENCRYPT:
      ret = int32_serialize(out, len, pos, 16);
      break;
    case EC_DECRYPT:
      ret = int32_serialize(out, len, pos, 32);
      break;
    case EC_SHUTDOWN:
      ret = int32_serialize(out, len, pos, 48);
      break;
    default: ret = -1;
  }
  
  return ret;
}

int EncryptorMode_deserialize(uint8_t *in, uint32_t len, uint32_t *pos, enum EncryptorMode *out)
{
  int ret = 0;
  
  int32_t v = 0;
  ret = int32_deserialize(in, len, pos, &v);
  if (ret) return ret;
  switch (v)
  {
    case 16:
      *out = EC_ENCRYPT;
      break;
    case 32:
      *out = EC_DECRYPT;
      break;
    case 48:
      *out = EC_SHUTDOWN;
      break;
    default: ret = -1;
  }
  
  return ret;
}


int EncryptorCommand_serialize(uint8_t *out, uint32_t len, uint32_t *pos, enum EncryptorCommand in)
{
  int ret = 0;
  
  switch (in)
  {
    case EC_DATA:
      ret = int32_serialize(out, len, pos, 64);
      break;
    case EC_DONE:
      ret = int32_serialize(out, len, pos, 80);
      break;
    default: ret = -1;
  }
  
  return ret;
}

int EncryptorCommand_deserialize(uint8_t *in, uint32_t len, uint32_t *pos, enum EncryptorCommand *out)
{
  int ret = 0;
  
  int32_t v = 0;
  ret = int32_deserialize(in, len, pos, &v);
  if (ret) return ret;
  switch (v)
  {
    case 64:
      *out = EC_DATA;
      break;
    case 80:
      *out = EC_DONE;
      break;
    default: ret = -1;
  }
  
  return ret;
}


int EncryptorResponse_serialize(uint8_t *out, uint32_t len, uint32_t *pos, enum EncryptorResponse in)
{
  int ret = 0;
  
  switch (in)
  {
    case ER_ERROR:
      ret = int32_serialize(out, len, pos, 255);
      break;
	case ER_UNSUPPORTED:
	  ret = int32_serialize(out, len, pos, 254);
	  break;
    case ER_OK:
      ret = int32_serialize(out, len, pos, 1);
      break;
    default: ret = -1;
  }
  
  return ret;
}

int EncryptorResponse_deserialize(uint8_t *in, uint32_t len, uint32_t *pos, enum EncryptorResponse *out)
{
  int ret = 0;
  
  int32_t v = 0;
  ret = int32_deserialize(in, len, pos, &v);
  if (ret) return ret;
  switch (v)
  {
    case 255:
      *out = ER_ERROR;
      break;
	case 254:
	  *out = ER_UNSUPPORTED;
	  break;
    case 1:
      *out = ER_OK;
      break;
    default: ret = -1;
  }
  
  return ret;
}

