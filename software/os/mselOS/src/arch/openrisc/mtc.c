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
#include <stdlib.h>
#include <stdint.h>
#include <msel.h>
#include "mtc.h"
#include "flash.h"

/* cached location of next mtc address */
static mtc_t* first_unused = NULL;

mtc_t* mtc_find_first_unused()
{
    mtc_t scratch;
    
    if(!first_unused)
        first_unused = MTC_START_ADDR;

    size_t page = FLASH_PAGE(first_unused);
    size_t offset = FLASH_OFFSET(first_unused);

    /* foreach page in bank */
    for(;page<FLASH_BANK_PAGES;page++)
    {
        /* foreach mtc in page */
        for(;offset<FLASH_PAGE_BYTES/sizeof(mtc_t);offset+=sizeof(mtc_t))
        {
            mtc_t* check = (mtc_t*)FLASH_ADDR(MTC_BANK_NUM, page, offset);
            flash_readmem(&scratch, check, sizeof(scratch));
            if(scratch == MTC_ENTRY_UNUSED)
            {
                first_unused = check;
                return first_unused;
            }
        }
        offset = 0;
    }

    /* No unused value was found, so we wrap back to page=0 offset=0
     * which will trigger a complete erase cycle before writing */
    first_unused = MTC_START_ADDR;
    return first_unused;
}

msel_status mtc_increment_store(mtc_t forced_next, mtc_t *retval)
{
    mtc_t* lastptr;
    mtc_t  lastmtc;
    mtc_t* nextptr;
    mtc_t  nextmtc;

    msel_status ret;

    /* Next is the first still-erased mtc in the reserved flash space */
    nextptr = mtc_find_first_unused();

    /* Last is the mtc ptr immediately before next, but it may wrap around */
    lastptr = nextptr-1;
    if(lastptr < MTC_START_ADDR)
        lastptr = FLASH_ADDR(MTC_BANK_NUM,FLASH_BANK_PAGES,FLASH_PAGE_BYTES-sizeof(mtc_t));
    
    flash_readmem(&lastmtc,lastptr,sizeof(lastmtc));

    /* force the value if it is being set explicitly, else just increment */
    if(forced_next != 0 && forced_next!=MTC_ENTRY_UNUSED)
        nextmtc = forced_next;
    else
        nextmtc = lastmtc+1;

    /* Skip to next mtc in the rare case of a u64 wrapping */
    if(nextmtc == MTC_ENTRY_UNUSED)
        nextmtc=0;

    /* if we are starting fresh on a bank, erase it first */
    if(nextptr == MTC_START_ADDR)
    {
        if((ret=flash_erase_bank(MTC_BANK_NUM)) != MSEL_OK)
            return ret; 
    }

    /* let the flash layer perform the write */
    ret = flash_writemem(nextptr, &nextmtc, sizeof(nextmtc));
    if(ret != MSEL_OK)
        return ret;

    if(retval)
        *retval = nextmtc;

    return MSEL_OK;
}

msel_status arch_mtc_read_increment(mtc_t* val)
    
{
    msel_status ret;
    
    if(!val)
        return MSEL_EINVAL;
    *val = MTC_ENTRY_UNUSED;
    
    /* Increment and write-back updated mtc value. This might possibly
     * fail if a block gets marked as bad or potentially if something
     * else is already performing a flash operation. */
    if((ret=mtc_increment_store(0,val)) != MSEL_OK)
        return ret;

    return MSEL_OK;
}

msel_status arch_mtc_write(mtc_t* val)
{
    return mtc_increment_store(*val, NULL);
}
