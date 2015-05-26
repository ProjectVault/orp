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
#include <string.h>

#include <msel.h>
#include <msel/stdc.h>

#include "os/system.h"
#include "os/task.h"
#include "os/taskmem.h"

#include "or1k.h"
#include "arch.h"

void arch_init_task()
{
}

msel_status arch_task_create(msel_tcb* task)
{
    or1k_saved_regs* regs;

    /* make space for saved registers */
    task->stack -= sizeof(or1k_saved_regs);
    task->stack -= 0x88; /* extra space needed to call restore w/o calling save @ first launch */
    
    regs = (or1k_saved_regs*)task->stack;

    /* initialize registers */
    msel_memset(regs, 0, sizeof(*regs));

    /* Initialize the status register for the new task */
    regs->sr |= SPR_SR_IEE; /* interrupt enable */
    regs->sr |= SPR_SR_TEE; /* timer enable */
    regs->sr |= SPR_SR_DME; /* Enable DMMU */
    regs->sr |= SPR_SR_IME; /* Enable IMMU */
    regs->sr |= SPR_SR_FO;  /* Bit is fixed to 1, clearing this bit is undefined */

    /* set pc to entrypoint */
    regs->pc = (uint32_t)task->entry; 

    /* The stack pointer (r1) will be initialized automatically on
     * context restore from the value of task->stack */
    
    /* R3 and R4 are the C args of (*msel_task_entry)() */
    regs->r3 = (uint32_t)task->arg;
    regs->r4 = (uint32_t)task->arg_sz;
    
    return MSEL_OK;
}

void arch_task_cleanup(msel_tcb* task)
{
}

void arch_task_launch_main(msel_tcb* task)
{

    msel_active_task     = &msel_task_list[0];
    msel_active_task_num = 0;

    __asm __volatile (
//        "l.addi r10, r0, 1     \n" /* r10=1 means restore from active_task->stack */
        "l.j context_restore   \n"
        "l.nop                 \n"
    );

    /* Never reached */
}

msel_status arch_mutex_lock()
{
    /* TODO! */
    return MSEL_ENOTIMPL;
}

void arch_platform_init()
{
    /* Most system init for openrisc happens in arch_init_isr() */
}

void arch_nvm_write(uint8_t* dest, const uint8_t* src, uint32_t len)
{
    (void)dest;
    (void)src;
    (void)len;
}

void arch_nvm_read(uint8_t* dest, uint8_t* src, uint32_t len)
{
    (void)dest;
    (void)src;
    (void)len;
}

/* Initialize system timer interrupt (see OpenRisc Arch 1.1, Chapter 14)*/
void arch_init_tick_timer()
{
    /* UPR[TTP] indicates if Tick Timer is present */
    if(!(spr_read(SPR_UPR) & SPR_UPR_TTP))
        msel_panic("TickTimer missing");

    /* TTCR = 0; internal ctr must be initialized manually */
    spr_write(SPR_TTCR, 0);
    
    /* TTMR[TP] = ticks_per_intr, TODO: figure out reasonable value for non-sim environment */
    SPR_TTMR_TP_SET(500000);

    /* TTMR[M] = 0x1; auto-restart timer on expire */
    SPR_TTMR_M_SET(1);
    
    /* TTMR[IP] = 0; Clear pending interrupt */
    SPR_TTMR_IP_SET(0);
    
    /* TTMR[IE] = 1; Issue interrupt when counter matches configured value */
    SPR_TTMR_IE_SET(1);

    /* SR[TEE] = 1; enable timer globally */
    // SPR_SR_TEE_SET(1);
}

void arch_systick_handler()
{
    /* TTMR[IP] = 0; Clear pending interrupt */
    SPR_TTMR_IP_SET(0);
}

/*
  There are two MMUs because of the harvard arch, DMMU and IMMU.

  The page table scheme referenced in the specification is not
  actually implemented in and software is expected to implement this
  directly in the TLB via TLB-miss and page fault exceptions.

  mselOS will forego a full-sized page table implementation and
  instead statically build out the TLB for everything except a tasks
  private memory, for which tlb entries will be swapped in and out
  manually during context switching.

  Task 0 (msel_main) can see the entire address space:
  
  ROM: 0x00000000-0x00010000, 0-64k (currently 0x20000, 0-128k for debug)
  RAM: 0x00100000-0x00120000, 1MB-+128k

  Page size is 8k, so each 128k address space requires 16 TLB entries
  for 32 total (per bus). However, the insn bus will never need to execure from
  RAM so it only needs entries for ROM.
  
  For now all addresses will be identity mapped, but with an eye
  toward future potential to map to randomized addresses per task. (ASLR)

*/



extern int end;

/* This must be called from an interrupt context with MMU disabled */
void arch_task_setup_mm(msel_tcb *task)
{
    /* The only user page permissions that ever change are for each's
     * tasks private memory areas. Currently, these are all page
     * aligned and no shared memory is possible in between tasks. If
     * that changes we'll probably want to invalidate the cache
     * entries for the entire private area here before restoring
     * context */
}


/* Set all interrupt priorities */
void arch_init_isr()
{
    /* Setup interrupt vector base addr */
    spr_write(SPR_EVBAR, ROM_START);

    /* Init timer */
    arch_init_tick_timer();

    /* Init hw interrupts */
    uint32_t intrs = 0;
    intrs |= (1<<16);
    intrs |= (1<<17);
    spr_write(SPR_PICMR, intrs);
}

inline void* arch_get_task_heap() 
{
    register void*    heapptr;

    /* assume 2k stacks, calc heap pointer from value of current stack ptr */
    __asm __volatile(
        "l.addi  %0, r1, 0x0800   \n"
        "l.movhi r29, 0xffff      \n"
        "l.ori   r29, r29, 0xf800 \n"
        "l.and   %0, %0, r29      \n"
        :"=r"(heapptr)
        ::"r29"
    );

    return heapptr;
}

int or1k_save_context()
{
    /* interrupted task's SP saved via context_save */
    register uint32_t task_sp asm("r31");

    const int        save_sz = 0x108; /* x80 for regs x80 for res space, 8 for frame */
    or1k_saved_regs *regs    = (or1k_saved_regs*)((uint8_t*)0x220000 - save_sz);
    size_t           task    = msel_active_task_num;

    /* Ensure the stack for interrupted task is valid */
    if( (task_sp <= (uint32_t)taskmem_stack_bottom(task)+save_sz) ||
       (task_sp > (uint32_t)taskmem_stack_top(task)) )
    {
        if(task != MSEL_TASK_MAIN)
        {
            msel_task_force_kill(task, "Stack error");
            /* Indicate that the handler should *NOT* be called */
            return 0;
        }
        else
        {
            msel_panic("Stack error in task0");
        }
    }

    /* Update saved stack ptr in task struct */
    msel_active_task->stack = (void*)task_sp - save_sz;

    /* Copy saved reg values */
    msel_memcpy(msel_active_task->stack, regs, sizeof(*regs));

    /* Indicate that the handler should be called */
    return 1;
}
