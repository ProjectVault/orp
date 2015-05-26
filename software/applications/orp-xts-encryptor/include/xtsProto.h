/**
 * @file xtsProto.h
 */

#ifndef __XTSPROTO_H__
#define __XTSPROTO_H__


#include <tidl.h>


/* Start forward declarations */

enum XtsAlgo;
enum XtsCommand;
enum XtsResponse;

/* End forward declarations */


/* Start array length constants */


/* End array length constants */


/* Start type definitions */

enum XtsAlgo {
  XTS_128 = 0,
  XTS_256 = 2,
};

enum XtsCommand {
  XTS_ENCRYPT = 16,
  XTS_DECRYPT = 32,
  XTS_SHUTDOWN = 48,
};

enum XtsResponse {
  XTS_ERROR = 255,
  XTS_UNSUPPORTED = 254,
  XTS_OK = 1,
};


/* End type definitions */


/* Start function prototypes */

int XtsAlgo_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, enum XtsAlgo in);
int XtsAlgo_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, enum XtsAlgo *out);

int XtsCommand_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, enum XtsCommand in);
int XtsCommand_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, enum XtsCommand *out);

int XtsResponse_serialize(uint8_t *out, uint32_t out_len, uint32_t *pos, enum XtsResponse in);
int XtsResponse_deserialize(uint8_t *in, uint32_t in_len, uint32_t *pos, enum XtsResponse *out);


/* End function prototypes */


#endif /* __XTSPROTO_H__ */
