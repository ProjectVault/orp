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
#include <msel/stdc.h>
#include "flash.h"

/* WARNING: This interface is *not* for concurrent access */

/* Whole pages must be written at a time, so we must buffer in order
 * to perform partial writes. TODO: It may be safe to just drop this
 * buffer and use the page register from the controller directly. */
static uint8_t flash_scratch_page[FLASH_PAGE_BYTES];

/* Tracks the bank/page currently in the buffers */
static int32_t flash_current_bank = -1;
static int32_t flash_current_page = -1;

msel_status flash_erase_bank(size_t banknum)
{
    msel_status retval = MSEL_EUNKNOWN;

    if(banknum > FLASH_PART_BANKS)
    {
        retval = MSEL_EINVAL;
        goto rethook;
    }
    
    if(FLASH_IS_BUSY)
    {
        retval = MSEL_EBUSY;
        goto rethook;
    }

    /* Drop cache */
    flash_current_bank = -1;
    flash_current_page = -1;
    
    /* Initiate the block erase */
    *FLASH_REG_ERASE_BANK = banknum;

    /* we must block here until the entire op finishes or else mutex
     * all other IO */
    while(FLASH_IS_BUSY);

    /* Check for hw errors */
    if(FLASH_HAD_ERROR)
        return MSEL_EUNKNOWN;

    retval = MSEL_OK;
    
rethook:
    return retval;
}

void flash_read_page(size_t bank, size_t page)
{
    uint8_t* ptr = FLASH_CTRL_ADDR;
    size_t i;

    /* Return cached result if applicable */
    if(flash_current_bank == bank && flash_current_page == page)
        return;

    /* Drop cache */
    flash_current_bank = -1;
    flash_current_page = -1;
    
    /* wait for pending to complete */
    while(FLASH_IS_BUSY);    

    /* Initiate read of page */
    *FLASH_REG_READ_PAGE = (uint32_t)FLASH_PAGE_ADDR(bank, page);

    /* wait for read to complete */
    while(FLASH_IS_BUSY);

    /* Copy to buffer */ 
    for(i=0; i<sizeof(flash_scratch_page); i+=4)
        *((uint32_t*)(flash_scratch_page+i)) = *((uint32_t*)(ptr+i));

    /* Update cache info */
    flash_current_bank = bank;
    flash_current_page = page;
}

void flash_flush_page(size_t bank, size_t page)
{
    uint8_t* ptr = FLASH_CTRL_ADDR;
    size_t i;

    /* wait for pending to complete */
    while(FLASH_IS_BUSY);    

    /* no caching on writes */

    for(i=0; i<sizeof(flash_scratch_page); i+=4)
        *((uint32_t*)(ptr+i)) = *((uint32_t*)(flash_scratch_page+i));

    *FLASH_REG_WRITE_PAGE = (uint32_t)FLASH_PAGE_ADDR(bank, page);

    /* wait for read to complete */
    while(FLASH_IS_BUSY);    
}



/* Buffering memcpy into flash that satisfys the requirement that all
 * writes consist of in-order writes of complete pages only */
msel_status flash_writemem(void* dst, void* src, size_t sz)
{
    size_t count = 0;
    
    if(dst < (void*)FLASH_ADDR_BASE ||
       (dst+sz) >= (void*)FLASH_ADDR_MAX ||
       sz > FLASH_PART_BYTES)
        return MSEL_EINVAL;

    while(sz-count > 0)
    {
        uint8_t* ptr    = (uint8_t*)dst + count;
        size_t   bank   = FLASH_BANK(ptr);
        size_t   page   = FLASH_PAGE(ptr);
        size_t   offset = FLASH_OFFSET(ptr);
        size_t   tmpcnt;

        flash_read_page(bank,page);

        /* first iteration will page align */
        if(offset > 0)
            tmpcnt = FLASH_PAGE_BYTES - offset;
        else
            tmpcnt = FLASH_PAGE_BYTES;

        /* Don't write past requested area */
        if(tmpcnt > sz - count)
            tmpcnt = sz - count;

        /* Copy snippet into current buffered page */
        msel_memcpy(flash_scratch_page+offset, (uint8_t*)src+count, tmpcnt);

        count += tmpcnt; /* dst+count should now be aligned if sz-count>0 */

        flash_flush_page(bank, page);
    }

    return MSEL_OK;
}

/* Buffering memcpy from flash that transparently handles any page
 * alignment issues */
msel_status flash_readmem(void* dst, void* src, size_t sz)
{
    size_t count = 0;
    
    if(src < (void*)FLASH_ADDR_BASE ||
       (src+sz) >= (void*)FLASH_ADDR_MAX ||
        sz > FLASH_PART_BYTES)
        return MSEL_EINVAL;

    while(sz-count > 0)
    {
        uint8_t* ptr    = (uint8_t*)src + count;
        size_t   bank   = FLASH_BANK(ptr);
        size_t   page   = FLASH_PAGE(ptr);
        size_t   offset = FLASH_OFFSET(ptr);
        size_t   tmpcnt;
        
        flash_read_page(bank,page);

        /* first iteration will page align */
        if(offset > 0)
            tmpcnt = FLASH_PAGE_BYTES - offset;
        else
            tmpcnt = FLASH_PAGE_BYTES;

        /* Don't write past requested area */
        if(tmpcnt > sz - count)
            tmpcnt = sz - count;

        /* Copy snippet into current buffered page */
        msel_memcpy((uint8_t*)dst+count, flash_scratch_page+offset, tmpcnt);

        count += tmpcnt; /* dst+count should now be aligned if sz-count>0 */

    }

    return MSEL_OK;
}
