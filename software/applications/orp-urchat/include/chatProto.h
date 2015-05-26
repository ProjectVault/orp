/**
 * @file chatProto.h
 */

#ifndef __CHATPROTO_H__
#define __CHATPROTO_H__


#include <tidl.h>


/* Start forward declarations */

enum ChatCommand;
enum ChatResponse;

/* End forward declarations */


/* Start array length constants */


/* End array length constants */


/* Start type definitions */

enum ChatCommand {
  CC_LOGOUT = 16,
  CC_SETSEEK = 32,
  CC_GETSEEK = 48,
  CC_GET_CHANNEL = 64,
  CC_CLOSE_CHANNEL = 80,
  CC_GET_EXCHANGE = 96,
  CC_ENCRYPT = 112,
  CC_INCOMING = 128,
};

enum ChatResponse {
  CR_ERROR = 255,
  CR_PUBKEY = 0,
  CR_GOODBYE = 17,
  CR_SEEKUPDATED = 33,
  CR_SEEK = 49,
  CR_CHANNEL = 65,
  CR_CHANNEL_CLOSED = 81,
  CR_EXCHANGE = 97,
  CR_CIPHERTEXT = 113,
  CR_FOUND = 129,
  CR_EXCHANGED = 130,
  CR_PLAINTEXT = 131,
  CR_NEVERMIND = 132,
};


/* End type definitions */


/* Start function prototypes */

int ChatCommand_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, enum ChatCommand in);
int ChatCommand_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, enum ChatCommand *out);

int ChatResponse_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, enum ChatResponse in);
int ChatResponse_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, enum ChatResponse *out);


/* End function prototypes */


#endif /* __CHATPROTO_H__ */
