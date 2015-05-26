/** @file flash.h
 */
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
#ifndef _MSEL_ARCH_OR1K_FLASH_H_
#define _MSEL_ARCH_OR1K_FLASH_H_

/* Routines for interfacing with the on-board Spansion flash part
 * through the FTL included in the SoC */

#include <stdlib.h>
#include <stdint.h>
#include <msel.h>

/** @brief Define MMIO maps to the FTL */
#define FLASH_CTRL_ADDR       ((uint8_t*) 0xA0000000)
#define FLASH_REGBASE_ADDR    (FLASH_CTRL_ADDR + 0x1000)

/* Various registers used to control flash operations */
#define FLASH_REG_RESERVED_0       ((uint32_t*)(FLASH_REGBASE_ADDR + 0x00))

#define FLASH_REG_SPARE_SPACE_WR_0 ((uint64_t*)(FLASH_REGBASE_ADDR + 0x10))
#define FLASH_REG_SPARE_SPACE_WR_1 ((uint64_t*)(FLASH_REGBASE_ADDR + 0x14))

#define FLASH_REG_SPARE_SPACE_RD_0 ((uint64_t*)(FLASH_REGBASE_ADDR + 0x18))
#define FLASH_REG_SPARE_SPACE_RD_1 ((uint64_t*)(FLASH_REGBASE_ADDR + 0x1C))

#define FLASH_REG_ERASE_BANK       ((uint32_t*)(FLASH_REGBASE_ADDR + 0x20))
    
#define FLASH_REG_STATUS           ((uint32_t*)(FLASH_REGBASE_ADDR + 0x28))

#define FLASH_REG_WRITE_PAGE       ((uint32_t*)(FLASH_REGBASE_ADDR + 0x30))

#define FLASH_REG_READ_PAGE        ((uint32_t*)(FLASH_REGBASE_ADDR + 0x38))

/* Status Fields */
#define FLASH_IS_BUSY  (*FLASH_REG_STATUS & 0x2)
#define FLASH_IS_READY   (!FLASH_IS_BUSY)
#define FLASH_HAD_ERROR (*FLASH_REG_STATUS & 0x1)

/* 2048 byte pages, 64 pages/bank, 1024 banks in total == 131,072,000 bytes 

   Address format
     
   31-27 : unused

   26-17 : bank number

   16-11 : page number

   10-0  : offset inside page
*/
#define FLASH_OFFSET_MASK (0x000007ff)
#define FLASH_PAGE_BYTES  ((size_t) 2048)
#define FLASH_PAGE_MASK   (0x0001f800)
#define FLASH_PAGE_SHIFT  (11)
#define FLASH_BANK_PAGES  ((size_t) 64)
#define FLASH_BANK_MASK   (0x07fe0000)
#define FLASH_BANK_SHIFT  (17)
#define FLASH_PART_BANKS  ((size_t) 1024)
#define FLASH_PART_BYTES  (FLASH_PAGE_BYTES*FLASH_BANK_PAGES*FLASH_PART_BANKS)

/* Define "virtual" address ranges. These addresses aren't really part
 * of the system memory map, but are converted to bank/page/offsets by
 * the read/write API calls. This is used versus an API that
 * references banks and pages directly so that the API won't break if
 * a transistion to a fully memory-mapped controller happens.*/

#define FLASH_ADDR_BASE ((uint8_t*)0xE0000000) /* Could be 0 offset for
                                               * perf, but non-zero is
                                               * safer */

#define FLASH_ADDR_MAX  (FLASH_ADDR_BASE + (FLASH_PART_BYTES))

#define FLASH_ADDR(bank, page, offset)                                  \
    ((void*)                                                            \
     ((uint32_t)FLASH_ADDR_BASE |                                       \
      (((uint32_t)(bank) << FLASH_BANK_SHIFT) & FLASH_BANK_MASK) |      \
      (((uint32_t)(page) << FLASH_PAGE_SHIFT) & FLASH_PAGE_MASK) |      \
      (((uint32_t)(offset) & FLASH_OFFSET_MASK))))

/* Ptr math helpers */
#define FLASH_PAGE_ADDR(bank, page)                                     \
    ((void*)                                                            \
     ((((uint32_t)(bank) << FLASH_BANK_SHIFT) & FLASH_BANK_MASK) |      \
      (((uint32_t)(page) << FLASH_PAGE_SHIFT) & FLASH_PAGE_MASK)))          

#define FLASH_OFFSET(addr) ((uint32_t)(addr) & FLASH_OFFSET_MASK)
#define FLASH_PAGE(addr)   (((uint32_t)(addr) & FLASH_PAGE_MASK)>>FLASH_PAGE_SHIFT)
#define FLASH_BANK(addr)   (((uint32_t)(addr) & FLASH_BANK_MASK)>>FLASH_BANK_SHIFT)



typedef enum {
    FLASH_STATUS_READY = 0,
    FLASH_STATUS_BUSY  = 1
} flash_status_t;

flash_status_t flash_status();
msel_status   flash_erase_bank(size_t banknum);
void          flash_read_page(size_t bank, size_t page);
void          flash_flush_page(size_t bank, size_t page);
msel_status   flash_writemem(void* dst, void* src, size_t sz);
msel_status   flash_readmem(void* dst, void* src, size_t sz);

#endif

