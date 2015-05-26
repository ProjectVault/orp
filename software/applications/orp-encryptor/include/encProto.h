/**
 * @file encProto.h
 */

#ifndef __ENCPROTO_H__
#define __ENCPROTO_H__

#include <stdint.h>


enum EncryptorAlgo;
enum EncryptorMode;
enum EncryptorCommand;
enum EncryptorResponse;
                       
enum EncryptorAlgo
{
  EC_GCM_128 = 0,
  EC_GCM_192 = 1,
  EC_GCM_256 = 2
};

enum EncryptorMode
{
  EC_ENCRYPT = 16,
  EC_DECRYPT = 32,
  EC_SHUTDOWN = 48
};

enum EncryptorCommand
{
  EC_DATA = 64,
  EC_DONE = 80
};

enum EncryptorResponse
{
  ER_ERROR = 255,
  ER_UNSUPPORTED = 254,
  ER_OK = 1
};


int EncryptorAlgo_serialize(uint8_t *out, uint32_t len, uint32_t *pos, enum EncryptorAlgo in);
int EncryptorAlgo_deserialize(uint8_t *in, uint32_t len, uint32_t *pos, enum EncryptorAlgo *out);
                                                                                                 
int EncryptorMode_serialize(uint8_t *out, uint32_t len, uint32_t *pos, enum EncryptorMode in);
int EncryptorMode_deserialize(uint8_t *in, uint32_t len, uint32_t *pos, enum EncryptorMode *out);
                                                                                                 
int EncryptorCommand_serialize(uint8_t *out, uint32_t len, uint32_t *pos, enum EncryptorCommand in);
int EncryptorCommand_deserialize(uint8_t *in, uint32_t len, uint32_t *pos, enum EncryptorCommand *out);
                                                                                                       
int EncryptorResponse_serialize(uint8_t *out, uint32_t len, uint32_t *pos, enum EncryptorResponse in);
int EncryptorResponse_deserialize(uint8_t *in, uint32_t len, uint32_t *pos, enum EncryptorResponse *out);


#endif /* __ENCPROTO_H__ */
