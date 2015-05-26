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
#include <msel.h>

#include "os/task.h"
#include "os/taskmem.h"
#include "os/system.h"
#include "os/syscall.h"

#include "driver/ffs_driver.h"
#include "driver/ffs_session.h"

#include "mmio.h"
#include "or1k.h"

/* Linker symbol indicating end of .data & .bss sections and the start of the heap */
extern int end;
extern int _data_beg;
extern int _bss_end;

/* Fw decls */
extern int hasSeenReadAck;
void FauxFileSystemWrite();
void FauxFileSystemReadAck();

void SystemCallHandler(msel_svc_number num, void* arg)
{
    msel_tcb* task = msel_active_task;

    /* task may change mid SVC due to suspended calls, etc */ 
    msel_status retval = msel_svc_handler(num, arg);

    /* save the result, no matter if it will actually return to the original task */
    ((or1k_saved_regs*)task->stack)->r11 = (uint32_t)retval;
}

void TickTimerHandler()
{
    msel_systick_handler();
}

void BusError()
{
    uint32_t eear = spr_read(SPR_EEAR(0));
    uint32_t epcr = spr_read(SPR_EPCR(0));

    (void) eear;
    (void) epcr;
    
    if(msel_active_task_num != 0)
    {
        msel_task_force_kill(msel_active_task_num, "Bus error");
    }
    else
    {
        msel_panic("Bus error in task0");
    }
}
void AlignmentError()
{

    uint32_t eear = spr_read(SPR_EEAR(0));
    uint32_t epcr = spr_read(SPR_EPCR(0));

    (void) eear;
    (void) epcr;
    
    if(msel_active_task_num != 0)
    {
        msel_task_force_kill(msel_active_task_num, "Alignment error");
    }
    else
    {
        msel_panic("Alignment error in task0");
    }
}
void IllegalInstruction()
{
    if(msel_active_task_num != 0)
    {
        msel_task_force_kill(msel_active_task_num, "Illegal Instruction");
    }
    else
    {
        msel_panic("Illegal instruction in task0");
    }
}

void ExternalInterrupt()
{
    static int i; i++;

    uint32_t picsr = spr_read(SPR_PICSR);

    if(picsr & (1<<16))
    {
        FauxFileSystemWrite();
        int i = 0;
        do {
            picsr &= ~((uint32_t)1<<16);
            spr_write(SPR_PICSR,picsr);
            picsr = spr_read(SPR_PICSR);
            i++;
        } while((i < 1024) && (picsr & (1<<16)));
    }

    if(picsr & (1<<17))
    {
        FauxFileSystemReadAck();
        int i = 0;
        do {
            picsr &= ~((uint32_t)1<<17);
            spr_write(SPR_PICSR,picsr);
            picsr = spr_read(SPR_PICSR);
            i++;
        } while((i < 1024) && (picsr & (1<<17)));
    }
    
}

void DTLBMiss()
{
    static uint32_t last_eear = 0;
    static uint32_t last_epcr = 0;
    uint32_t eear = spr_read(SPR_EEAR(0));
    uint32_t epcr = spr_read(SPR_EPCR(0));
    uint32_t tlbmr = 0;
    uint32_t tlbtr = 0;
    uint32_t perms = 0;

    /* Double fault detection */
    if(eear == last_eear && epcr == last_epcr)
    {
        msel_panic("The DMMU isn't very happy about this");
    }
    last_eear = eear;
    last_epcr = epcr;
    
    /* Set the effictive and physical addresses (identity mapping) */
    tlbmr = tlbtr = eear & VPN_MASK;

    /* Calculate permissions based on active task and req address */
    if(eear >= ROM_START && eear < ROM_START+ROM_SIZE)
    {
        /* Everyone can read ROM */
        perms |= TLBTR_SRE | TLBTR_URE;
    }
    else if(eear >= RAM_START && eear < RAM_START + RAM_SIZE)
    {
        /* Supervisor can read/write entire ram space */
        perms |= TLBTR_SRE | TLBTR_SWE;

        /* Task 0 can r/w entire ram space */
        if(msel_active_task_num == MSEL_TASK_MAIN)
        {
            perms |= TLBTR_UWE | TLBTR_URE;
        }
        else
        {
            if((void*)eear >= taskmem_stack_bottom(msel_active_task_num) &&
               (void*)eear < taskmem_heap_end(msel_active_task_num))
            {
                /* User tasks access ram  thier own ram */
                perms |= TLBTR_URE | TLBTR_UWE;
            }

            if((int*)eear >= &_data_beg && (int*)eear < &_bss_end)
            {
                /* User tasks can still read bss & data to avoid linking nightmares */
                perms |= TLBTR_URE;
            }
            
        }
    } 
    /* handle MMIO ranges */
    else if((eear >= UART_ADDR && eear < UART_ADDR + 0x8000) ||
            (eear >= TRNG_ADDR && eear < TRNG_ADDR + 0x8000) ||
            (eear >= AES_ADDR  && eear < AES_ADDR  + 0x8000) ||
            (eear >= SHA_ADDR  && eear < SHA_ADDR  + 0x8000) ||
            (eear >= ECC_ADDR  && eear < ECC_ADDR  + 0x8000) ||
            (eear >= FFS_ADDR  && eear < FFS_ADDR  + 0x8000))

    {
        // Supervisor can read/write to MMIO addresses
        perms |= TLBTR_SWE | TLBTR_SRE;
    }

    /* Set V (valid) bit and context id */
    tlbmr |= TLBMR_V | (msel_active_task->arch.ctx_id << 2);

    /* Store the new TLB cache entry */
    spr_write(SPR_DTLBW0MR(TLB_ENTRY(eear)), tlbmr);
    spr_write(SPR_DTLBW0TR(TLB_ENTRY(eear)), tlbtr | perms);
}

void ITLBMiss()
{
    static uint32_t last_eear = 0;
    static uint32_t last_epcr = 0;
    uint32_t eear = spr_read(SPR_EEAR(0));
    uint32_t epcr = spr_read(SPR_EPCR(0));
    uint32_t tlbmr = 0;
    uint32_t tlbtr = 0;
    uint32_t perms = 0;

    /* Double fault detection */
    if(eear == last_eear && epcr == last_epcr)
    {
        msel_panic("The IMMU isn't very happy about this");
    }
    last_eear = eear;
    last_epcr = epcr;

    /* Set the effictive and physical addresses (identity mapping) */
    tlbmr = tlbtr = eear & VPN_MASK;

    /* Calculate permissions based on active task and req address */
//    if(eear >= ROM_START && eear < ROM_START+ROM_SIZE)
    {
        /* TODO: restrict to .text instead of ROM */
        perms |= TLBTR_SXE | TLBTR_UXE;
    }
    
    /* Set V (valid) bit and context id */
    tlbmr |= TLBMR_V | (msel_active_task->arch.ctx_id << 2);

    /* Store the new TLB cache entry */
    spr_write(SPR_ITLBW0MR(TLB_ENTRY(eear)), tlbmr);
    spr_write(SPR_ITLBW0TR(TLB_ENTRY(eear)), tlbtr | perms);
}

void DataPageFault()
{
    uint32_t epcr = spr_read(SPR_EPCR(0));
    uint32_t eear = spr_read(SPR_EEAR(0));

    /* These are here so breakpoints can access intr args */
    (void)epcr;
    (void)eear;

    if(msel_active_task_num != 0)
    {
        msel_task_force_kill(msel_active_task_num, "Data page fault");
    }
    else
    {
        msel_panic("Data page fault in task0");
    }
}

void InstructionPageFault()
{
    uint32_t epcr = spr_read(SPR_EPCR(0));
    uint32_t eear = spr_read(SPR_EEAR(0));

    /* These are here so breakpoints can access intr args */
    (void)epcr;
    (void)eear;

    if(msel_active_task_num != 0)
    {
        msel_task_force_kill(msel_active_task_num, "Instruction page fault");
    }
    else
    {
        msel_panic("Instruction page fault in task0");
    }
}

void RangeException()
{
    if(msel_active_task_num != 0)
    {
        msel_task_force_kill(msel_active_task_num, "Range exception");
    }
    else
    {
        msel_panic("Range exception in task0");
    }
}

void FloatingPointException()
{
    if(msel_active_task_num != 0)
    {
        msel_task_force_kill(msel_active_task_num,"Floating Point exception");
    }
    else
    {
        msel_panic("Floating Point exception in task0");
    }
}
void TrapException()
{
    if(msel_active_task_num != 0)
    {
        msel_task_force_kill(msel_active_task_num,"Trap exception");
    }
    else
    {
        msel_panic("Trap exception in task0");
    }    
}

void FauxFileSystemWrite()
{
    msel_wfile_get_packet();
    (*FFS_CTRL_ADDR) |= 1;
}

int hasSeenReadAck = 0;

void FauxFileSystemReadAck()
{
    hasSeenReadAck = 1;
    uint8_t status = msel_ffs_rfile_get_status();
    if (status == FFS_CHANNEL_READY || status == FFS_CHANNEL_LAST_SUCC)
        msel_rfile_step_queue();
    (*FFS_CTRL_ADDR) |= 2;
}
