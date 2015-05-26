/** @file isr.c 
   
   Contians implemenations of all the interrupt handlers 
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

#include "os/system.h"
#include "os/syscall.h"
#include "os/task.h"
#include "isr.h"
#include "config.h"

uint32_t msel_saved_basepri = ARCH_ISR_BASE_PRIO;

/* @brief Do any interrupt configuration and setup here */
void arch_init_isr()
{
    /* Enable faults to avoid HardFaults with easily attributable causes */
    *MSEL_SHCSR |= MEMFAULTENA | USGFAULTENA | BUSFAULTENA;

    /* Clear fault registers */
    *MMFSR = 0;
    *UFSR = 0;
    *BFSR = 0;

    /* Setup system timer */
    *SYSTICK_RV_REG = ( 10 * CLK_FREQ / MSEL_TIMER_FREQ ) - 1UL;

    *SYSTICK_CS_REG = SYSTICK_CLKSOURCE | SYSTICK_TICKINT | SYSTICK_ENABLE;

#ifdef SF2BUILD
    /* clear out watchdog timer settings */
    *((uint32_t*)(0x40005000 + 20)) &= ~3; 
    *((uint32_t*)(0x40038000 + 0x6C)) = 0x0000;
#endif
    
    /* Initialize interrupt masks */
    register int tmp;

    __asm__ volatile(
        "mov %0, #0        \n"
        "msr faultmask, %0 \n"
        :"=r"(tmp)
    );
    
    __asm__ volatile
    (
        "mov %0, #0      \n"
        "msr primask, %0 \n"
        :"=r"(tmp)
    );
    
}

/** @brief handlers the non-maskable interrupt */
void nmi_handler() 
{

}

/** @brief handle non-recoverable faults */
void hardfault_handler() 
{
    msel_panic("HardFault: cause unknown");
}

/** @brief handle memory access exceptions. This implements any MPU
    protections and may kill/restart threads or reset the system. 
*/
void memmanage_handler() 
{
    /* 
    volatile uint8_t mmfsr = 0;
    volatile uint32_t mmfar = 0;

    mmfsr = *MMFSR;

    if(mmfsr & MMFSR_MMARVALID)
        mmfar = *MMFAR;

    if(mmfar)
        msel_panic("Panic!");
    */
    
    if(msel_active_task_num != 0)
    {
        msel_task_force_kill(msel_active_task_num, "MPU violation");
    }
    else
    {
        msel_panic("MemManage fault in task0");
    } 
}

void usagefault_handler()
{
    uint16_t ufsr = *UFSR;
    char *errstr = NULL;

        if(ufsr & UFSR_DIVBYZERO)
            errstr = "UsageFault: DivByZero";
        
        else if(ufsr & UFSR_UNALIGNED)
            errstr = "UsageFault: Unaligned Access";
        
        else if(ufsr & UFSR_NOCP)
            errstr = "UsageFault: No Coprocessor";
        
        else if(ufsr & UFSR_INVPC)
            errstr = "UsageFault: Invalid PC";
        
        else if(ufsr & UFSR_INVSTATE)
            errstr = "UsageFault: Invalid EPSR access";
        
        else if(ufsr & UFSR_UNDEFINSTR)
            errstr = "UsageFault: Undefined instruction";
        else
            errstr = "UsageFault: unknown!";

        
    if(msel_active_task_num != 0)
        msel_task_force_kill(msel_active_task_num, errstr);
    else
        msel_panic(errstr);
}

void busfault_handler()
{
    if(msel_active_task_num != 0)
        msel_task_force_kill(msel_active_task_num, "BusFault");
    else
        msel_panic("BusFault");
}

/** @brief handle the raw SVC exception, pass control back to msel */
void svc_handler() 
{
    msel_tcb*       curr_task = msel_active_task;
    arm_saved_regs* regs      = (arm_saved_regs*)curr_task->stack;

    msel_status retval;
    
    retval = msel_svc_handler((msel_svc_number)regs->r0, (void *)regs->r1);

    /* Make sure to use regs from saved TCB... msel_active_task may have been rescheduled */
    regs->r0 = retval;
}

/** @brief Handles the exception that triggers when SYSTICK expires */
void systick_handler() 
{
    msel_systick_handler();
}

void debugmon_handler()
{

}

void pendsv_handler()
{
    
}


