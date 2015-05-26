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

#define TIDL_ARRAYLEN_MAX ((uint32_t) 2048)

int int8_serialize(uint8_t *buf, uint32_t l_buf, uint32_t *pos, int8_t p);
int int16_serialize(uint8_t *buf, uint32_t l_buf, uint32_t *pos, int16_t p);
int int32_serialize(uint8_t *buf, uint32_t l_buf, uint32_t *pos, int32_t p);
int int64_serialize(uint8_t *buf, uint32_t l_buf, uint32_t *pos, int64_t p);

int uint8_serialize(uint8_t *buf, uint32_t l_buf, uint32_t *pos, uint8_t p);
int uint16_serialize(uint8_t *buf, uint32_t l_buf, uint32_t *pos, uint16_t p);
int uint32_serialize(uint8_t *buf, uint32_t l_buf, uint32_t *pos, uint32_t p);
int uint64_serialize(uint8_t *buf, uint32_t l_buf, uint32_t *pos, uint64_t p);

int int8_deserialize(const uint8_t *buf, uint32_t l_buf, uint32_t *pos, int8_t *p);
int int16_deserialize(const uint8_t *buf, uint32_t l_buf, uint32_t *pos, int16_t *p);
int int32_deserialize(const uint8_t *buf, uint32_t l_buf, uint32_t *pos, int32_t *p);
int int64_deserialize(const uint8_t *buf, uint32_t l_buf, uint32_t *pos, int64_t *p);

int uint8_deserialize(const uint8_t *buf, uint32_t l_buf, uint32_t *pos, uint8_t *p);
int uint16_deserialize(const uint8_t *buf, uint32_t l_buf, uint32_t *pos, uint16_t *p);
int uint32_deserialize(const uint8_t *buf, uint32_t l_buf, uint32_t *pos, uint32_t *p);
int uint64_deserialize(const uint8_t *buf, uint32_t l_buf, uint32_t *pos, uint64_t *p);

int string_serialize (uint8_t *out, uint32_t l_out, uint32_t *pos, const char *in);
int string_deserialize (const uint8_t *in, uint32_t l_in, uint32_t *pos, char **out);
