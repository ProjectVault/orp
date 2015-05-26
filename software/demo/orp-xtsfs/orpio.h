/* Copyright 2015, Google Inc.

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


/*
 * orpio.h
 */
#ifndef _ORPIO_H
#define _ORPIO_H

#include <stdint.h>

#define ORP_HDR_LEN 4
#define ORP_DATA_LEN 2044
#define ORP_PKT_LEN 2048
#define ORP_STATUS_LEN 16
#define ORP_BLOCK_SIZE 512

uint8_t orp_dev_wstatus();
int orp_dev_write(uint16_t session_id, const uint8_t* data);
int orp_dev_ack(uint8_t status);
int orp_dev_read(uint16_t* session_id, uint8_t* data);
int orp_dev_connect(const char* path, const char* key);

#endif // _ORPIO_H
