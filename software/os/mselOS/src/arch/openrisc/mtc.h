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
#ifndef _MSEL_ARCH_OR1K_MTC_H_
#define _MSEL_ARCH_OR1K_MTC_H_

/* The MTC reserves an entire bank of the flash memory because a bank
 * is the smallest unit which may be erased. One bank is a block of 64
 * 2k pages for a total of 128k of space. An mtc value is simply a 64
 * bit integer, so space exists for 2048 saved mtc values before an
 * erase is required.
 * 
 * MTC values are just written plain to a mtc_t[2048] array. Instead
 * of maintaining status to determine which value is current,
 * convention dictates new values be written in order from offset 0 of
 * the bank, and 0xffffffffffffffff is not a valid counter value. This
 * guarantees the last used mtc is the first non-erased uint64_t
 * inside the bank.
 *
 * However, one does not simply write a uint64_t to flash. The minimum
 * write size is a whole page (2048 bytes). The whole page must be
 * buffered and then flushed on every MTC update. While flushing care
 * must be taken that no zero bits are cleared past the location of
 * the last used mtc value because it takes a whole new erase cycle to
 * set bits=1.
 */

#include <msel/mtc.h>

#define MTC_BANK_NUM     1020 /* 0xE7f80000 - 0xE7f9ffff */
#define MTC_ENTRY_UNUSED ((mtc_t)0xffffffffffffffff)
#define MTC_START_ADDR   ((mtc_t*)FLASH_ADDR(MTC_BANK_NUM,0,0))

mtc_t*      mtc_find_first_unused();
msel_status mtc_increment_store(mtc_t, mtc_t*);

#endif

