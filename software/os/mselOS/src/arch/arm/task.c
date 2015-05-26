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

#include "arch.h"
#include "m3.h"
#include "os/util.h"
#include "os/taskmem.h"

void arch_init_task(msel_tcb* task)
{
    /* TODO */
}

void arch_task_cleanup(msel_tcb* task)
{
    /* nothing additional required */
}

msel_status arch_task_create(msel_tcb* task)
{
    arm_saved_regs* regs;

    task->stack -= sizeof(arm_saved_regs);

    regs = (arm_saved_regs*)task->stack;
    
    msel_memset(regs, 0, sizeof(*regs));

    regs->pc = (uint32_t)task->entry;
        
    regs->r0 = (uint32_t)task->arg;
    regs->r1 = task->arg_sz;

    regs->control = CONTROL_SPSEL_PSP;

    /* Setup thread permissions */
    if(task->num == MSEL_TASK_MAIN)
        regs->control |= CONTROL_THREAD_PRIV;
    else
        regs->control |= CONTROL_THREAD_UNPRIV;
    
    /* Enable thumb state */
    regs->xpsr = 0x01000000;
    
    /* for debugging alignment
	   regs->r1   = 0xf0001111; regs->r2   = 0xf0002222;
	   regs->r3   = 0xf0003333; regs->r4   = 0xf0004444;
	   regs->r5   = 0xf0005555; regs->r6   = 0xf0006666;
	   regs->r7   = 0xf0007777; regs->r8   = 0xf0008888;

	   regs->r9   = 0xf0009999; regs->r10  = 0xf010ff10;
	   regs->r11  = 0xf011ff11; regs->lr   = 0xba55face;
    */

    return MSEL_OK;
}

void arch_task_launch_main(msel_tcb* task)
{
    /* beware the stack switch that occurs below, do not reference stack vars in this scope */
    msel_active_task = &(msel_task_list[0]);
    msel_active_task_num = 0;

    __asm__ volatile(
        "b task_launch           \n"
    );
    
}

void arm_mpu_set(size_t slot, void* addr, size_t size, uint32_t perm)
{
    uint32_t rasr=0;

    /* Enable this mpu slot */
    rasr = MPU_RASR_ENABLE;

    /* Set size of region (converting size into number of bits via quickie log2(n)) */
    if(size <= 1024*4)
        rasr |= MPU_RASR_SIZE(12);
    else if(size <= 1024*8)
        rasr |= MPU_RASR_SIZE(13);
    else if(size <= 1024*16)
        rasr |= MPU_RASR_SIZE(14);
    else if(size <= 1024*32)
        rasr |= MPU_RASR_SIZE(15);
    else if(size <= 1024*64)
        rasr |= MPU_RASR_SIZE(16);
    else if(size <= 1024*128)
        rasr |= MPU_RASR_SIZE(17);
    else if(size <= 1024*256)
        rasr |= MPU_RASR_SIZE(18);
    else if(size <= 1024*512)
        rasr |= MPU_RASR_SIZE(19);

    /* Set region permissions */
    rasr |= perm;
    
    /* Commit changes to control registers */
    *MPU_RNR  = slot;
    *MPU_RBAR = (uint32_t)addr;
    *MPU_RASR = rasr; 
}

extern int rom_start;
extern int rom_size;
extern int ram_start;
extern int ram_size;

void arch_task_setup_mm(msel_tcb* task)
{
    /* Slot 0: ROM. read-only & execute */
    arm_mpu_set(0, &rom_start, (size_t)&rom_size, MSEL_SRAM_MEM_MODE | MPU_RASR_ACCESS(MPU_AP_RW_RO));

    /* Slot 1: All ram: Read only (to allow access to .data and .bss) */
    arm_mpu_set(1, &ram_start, (size_t)&ram_size, MSEL_SRAM_MEM_MODE | MPU_RASR_XN | MPU_RASR_ACCESS(MPU_AP_RW_RO));
    
    /* Slot 2: Task RAM: RW, no execute */
    void * addr = taskmem_stack_bottom(task->num);
    size_t sz = taskmem_stack_size(task->num) + taskmem_heap_size(task->num);
    arm_mpu_set(2, addr, sz, MSEL_SRAM_MEM_MODE | MPU_RASR_XN | MPU_RASR_ACCESS(MPU_AP_RW_RW));
    
    __asm__ volatile ("dsb"); /* flush data cache */

}

void* arch_get_task_heap()
{
    volatile register uint32_t heapptr;

    /* Note that for this technique to work properly, the heap must
     * immediately follow the stack and sizeof heap must be >= sizeof
     * stack */
    
    asm volatile (
        "add %0, sp, 0x800   \n" // add sizeof(stack) to sp to generate a ptr within the heap
        "mov r4, #0          \n"
        "sub r4, r4, 0x800   \n" // mask = 0xfffff800
        "and %0, %0, r4      \n" 
        :"=r"(heapptr)
        ::"r4"
    );

    return (void *)heapptr;
}
