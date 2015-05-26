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
#ifndef _MSEL_ARCH_OR1K_H
#define _MSEL_ARCH_OR1K_H

#include "spr.h"

/* Make sure this is in sync with the linker script */
#define ROM_START 0x00100000
#define ROM_SIZE  0x10000
#define RAM_START 0x00200000
#define RAM_SIZE  0x20000

/* Pages are 8k, not very embedded friendly, but it'll have to do */
#define PAGE_SIZE ((size_t)8192)
#define PAGE_BITS ((size_t)13)

/* Last 13 bits of an address, determining the offset with an 8k page */
#define PAGE_OFFSET_MASK (PAGE_SIZE-1)

/* virtual page number (bits 31-13 of a vaddr) */
#define VPN_MASK         (~(PAGE_OFFSET_MASK))

/* TLB cache management */
#define TLB_ENTRIES      ((size_t)64)
#define TLB_ENTRY_MASK   ((uint32_t)((TLB_ENTRIES-1)<<PAGE_BITS))
#define TLB_ENTRY(vaddr) ((vaddr&TLB_ENTRY_MASK) >> PAGE_BITS)

typedef struct
{
    uint8_t ctx_id; /* The HW context ID of this task, note that this may change independantly of the index into msel_task_list */
} arch_tcb;

typedef struct {
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t r12;
    uint32_t r13;
    uint32_t r14;
    uint32_t r15;

    uint32_t r16;
    uint32_t r17;
    uint32_t r18;
    uint32_t r19;
    uint32_t r20;
    uint32_t r21;
    uint32_t r22;
    uint32_t r23;

    uint32_t r24;
    uint32_t r25;
    uint32_t r26;
    uint32_t r27;
    uint32_t r28;
    uint32_t r29;
    uint32_t r30;
    uint32_t r31;

    uint32_t sr;
    uint32_t pc;
} or1k_saved_regs;

/* r2,r3 must be preloaded with syscall number and void* params */
#define ARCH_DO_SYSCALL(res) __asm __volatile__ (    \
        "l.sys 0             \n"                     \
        "l.addi %0, r11, 0   \n"                     \
        :"=r"(res)::"r11")

#define ARCH_EMIT_BREAKPOINT() __asm __volatile__ ("l.trap 0\n"); 

#define ARCH_ENABLE_INTERRUPTS()  SPR_SR_IEE_SET(1)
#define ARCH_DISABLE_INTERRUPTS() SPR_SR_IEE_SET(0)


#endif
