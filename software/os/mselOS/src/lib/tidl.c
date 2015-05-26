/*
   Copyright 2015, Google Inc.

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
#include <stdint.h>
#include <stddef.h>
#include <msel/stdc.h>
#include <tidl.h>

#define define_integral_serialize(fn, fn_abort, type, serial_size)\
int fn(uint8_t *buf, uint32_t l_buf, uint32_t *pos, type p) {\
  uint32_t pos_ = 0;\
  int r = 0;\
\
  if (NULL == buf) {\
    *pos += serial_size;\
  }\
  else {\
    pos_ = *pos;\
    if (l_buf < pos_ + serial_size) {\
      r = -1;\
      goto fn_abort;\
    }\
    size_t i = 0;\
    type shift = ((type) (serial_size - 1));\
    while (i < serial_size) {\
      pos_ = *pos;\
      buf[pos_] = 0xff & ((shift > 0) ? (p >> (8 * shift)) : p);\
      shift = shift - 1;\
      *pos = pos_ + 1;\
      i = i + 1;\
    }\
  }\
\
fn_abort :\
  return r;\
}

#define define_integral_deserialize(fn, fn_abort, type, serial_size)\
int fn(const uint8_t *buf, uint32_t l_buf, uint32_t *pos, type *p) {\
  uint32_t pos_ = 0;\
  int r = 0;\
\
  pos_ = *pos;\
  if (l_buf < pos_ + serial_size) {\
    r = -1;\
    goto fn_abort;\
  }\
  size_t i = 0;\
  type in = 0;\
  while (i < serial_size) {\
    pos_ = *pos;\
    in = (in << 8) | (buf[pos_] & 0xff);\
    *pos = pos_ + 1;\
    i = i + 1;\
  }\
  *p = in;\
\
fn_abort :\
  return r;\
}

define_integral_serialize(int8_serialize, abort_int8_serialize, int8_t, 1)
define_integral_serialize(int16_serialize, abort_int16_serialize, int16_t, 2)
define_integral_serialize(int32_serialize, abort_int32_serialize, int32_t, 4)
define_integral_serialize(int64_serialize, abort_int64_serialize, int64_t, 8)

define_integral_deserialize(int8_deserialize, abort_int8_deserialize, int8_t, 1)
define_integral_deserialize(int16_deserialize, abort_int16_deserialize, int16_t, 2)
define_integral_deserialize(int32_deserialize, abort_int32_deserialize, int32_t, 4)
define_integral_deserialize(int64_deserialize, abort_int64_deserialize, int64_t, 8)


define_integral_serialize(uint8_serialize, abort_uint8_serialize, uint8_t, 1)
define_integral_serialize(uint16_serialize, abort_uint16_serialize, uint16_t, 2)
define_integral_serialize(uint32_serialize, abort_uint32_serialize, uint32_t, 4)
define_integral_serialize(uint64_serialize, abort_uint64_serialize, uint64_t, 8)

define_integral_deserialize(uint8_deserialize, abort_uint8_deserialize, uint8_t, 1)
define_integral_deserialize(uint16_deserialize, abort_uint16_deserialize, uint16_t, 2)
define_integral_deserialize(uint32_deserialize, abort_uint32_deserialize, uint32_t, 4)
define_integral_deserialize(uint64_deserialize, abort_uint64_deserialize, uint64_t, 8)

int string_serialize (uint8_t *out, uint32_t l_out, uint32_t *pos, const char *in) {
  uint32_t i = 0;
  uint16_t len = 0;
  int r = 0;
  len = msel_strlen(in);

  r = uint16_serialize(out, l_out, pos, len);
  if (r) goto abort_string_serialize;

  for (i = 0; i < len; i++) {
    r = uint8_serialize(out, l_out, pos, in[i]);
    if (r) goto abort_string_serialize;
  }

abort_string_serialize:
  return r;
}

int string_deserialize (const uint8_t *in, uint32_t l_in, uint32_t *pos, char **out) {
  uint16_t len = 0;
  uint32_t i = 0;
  int r = 0;

  r = uint16_deserialize(in, l_in, pos, &len);
  if (r) goto abort_string_deserialize;
  *out = (char *)malloc(sizeof(char) * (1 + len));
  if (NULL == *out) { r = -1; goto abort_string_deserialize; }
  msel_memset(*out, 0, 1 + len);
  for (i = 0; i < len; i++) {
    r = uint8_deserialize(in, l_in, pos, (uint8_t*)&out[i]);
    if (r) goto abort_string_deserialize;
  }

abort_string_deserialize:
  return r;
}
