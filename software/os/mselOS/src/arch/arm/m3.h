/** @file m3.h

    Contains CPU specific definitions and configuration.

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

#ifndef _MSEL_M3_H_
#define _MSEL_M3_H_

#define PAGE_SIZE 4096 /* m3 doesn't really use pages, but define the smallest chunk of task mem here */
#define CLK_FREQ 50000000

/* CONTROL Register */
#define CONTROL_SPSEL_PSP     2
#define CONTROL_SPSEL_MSP     0
#define CONTROL_THREAD_UNPRIV 1
#define CONTROL_THREAD_PRIV   0

/* SYSTICK configuration registers and constants */

#define SYSTICK_CS_REG     ((uint32_t*)0xE000E010)
#define SYSTICK_COUNTFLAG  (1ul<<16)
#define SYSTICK_CLKSOURCE  (1ul<<2)
#define SYSTICK_TICKINT    (1ul<<1)
#define SYSTICK_ENABLE     (1ul<<0)
#define SYSTICK_RV_REG     ((uint32_t*)0xE000E014)
#define SYSTICK_CV_REG     ((uint32_t*)0xE000E018)
#define SYSTICK_CALIB      ((uint32_t*)0xE000E01C)

/* MPU configuration registers and constants */

#define MPU_TYPE              ((uint32_t*)0xE000ED90)
#define MPU_CTRL              ((uint32_t*)0xE000ED94)
#define MPU_PRIVDEFENA        (1ul<<2)
#define MPU_ENABLE            (1ul)
#define MPU_RNR               ((uint32_t*)0xE000ED98)
#define MPU_RBAR              ((uint32_t*)0xE000ED9C)
#define MPU_RBAR_VALID        (1ul<<4)
#define MPU_RBAR_REGION(n)    (n)
#define MPU_RASR              ((uint32_t*)0xE000EDA0)
#define MPU_RASR_XN           (1ul<<28)
#define MPU_RASR_ACCESS(n)    (((uint32_t)((n)&3))<<24)
#define MPU_RASR_TEX(n)       (((uint32_t)((n)&7))<<19)
#define MPU_RASR_S            (1ul<<18)
#define MPU_RASR_C            (1ul<<17)
#define MPU_RASR_B            (1ul<<16)
#define MPU_RASR_SUBREGION(n) ((uint32_t)(1<<(((n)&7)+8)))
#define MPU_RASR_SIZE(n)      ((uint32_t)(((n)-1)<<1)) /* sizeof region == 2**n */
#define MPU_RBAR_A1           ((uint32_t*)0xE000EDA4)
#define MPU_RASR_ENABLE       (1ul)
#define MPU_RASR_A1           ((uint32_t*)0xE000EDA8)
#define MPU_RBAR_A2           ((uint32_t*)0xE000EDAC)
#define MPU_RASR_A2           ((uint32_t*)0xE000EDB0)
#define MPU_RBAR_A3           ((uint32_t*)0xE000EDB4)
#define MPU_RASR_A3           ((uint32_t*)0xE000EDB8)
#define MPU_AP_NONE_NONE      (0ul)
#define MPU_AP_RW_NONE        (1ul)
#define MPU_AP_RW_RO          (2ul)
#define MPU_AP_RW_RW          (3ul)
#define MPU_AP_RO_NONE        (5ul)
#define MPU_AP_RO_RO          (6ul) /* 7 behaves the same */

/* CPU Status registers */

/* Hard fault related status */
#define HFSR           ((uint32_t*)0xE000ED2C)
#define HFSR_DEBUGEVT  (1ul << 31)
#define HFSR_FORCED    (1ul << 30)
#define HFSR_VECTBL    (1ul << 1)

/* CFSR (MemManage / BusFault / Usage fault) statuses combined into one word) */
#define CFSR ((uint32_t*)0xE000ED28)

/* Bus fault related status */
#define BFSR             ((uint8_t*)0xE000ED29)
#define BFSR_BFARVALID   (1ul << 7)
#define BFSR_STKERR      (1ul << 4)
#define BFSR_UNSTKERR    (1ul << 3)
#define BFSR_IMPRECISERR (1ul << 2)
#define BFSR_PRECISERR   (1ul << 1)
#define BFSR_IBUSERR     (1ul)

#define BFAR             ((uint32_t*)0xE000ED38)

/* Usage fault related status */
#define UFSR            ((uint16_t*)0xE000ED2A)
#define UFSR_DIVBYZERO  (1ul << 9)
#define UFSR_UNALIGNED  (1ul << 8)
#define UFSR_NOCP       (1ul << 3)
#define UFSR_INVPC      (1ul << 2)
#define UFSR_INVSTATE   (1ul << 1)
#define UFSR_UNDEFINSTR (1ul)

/* MemManage fault related status */
#define MMFSR           ((uint8_t*)0xE000ED28)
#define MMFSR_MMARVALID (1ul << 7)
#define MMFSR_MSTKERR   (1ul << 4)
#define MMFSR_MUNSTKERR (1ul << 3)
#define MMFSR_DACCVIOL  (1ul << 1)
#define MMFSR_IACCVIOL  (1ul << 1)

#define MMFAR           ((uint32_t*)0xE000ED34)

/* State & Management of fault handling */
#define MSEL_SHCSR     ((uint32_t*)0xE000ED24)
#define USGFAULTENA    (1ul << 18)
#define BUSFAULTENA    (1ul << 17)
#define MEMFAULTENA    (1ul << 16)
#define SVCALLPENDED   (1ul << 15)
#define BUSFAULTPENDED (1ul << 14)
#define MEMFAULTPENDED (1ul << 13)
#define USGFAULTPENDED (1ul << 12)
#define SYSTICKACT     (1ul << 11)
#define PENDSVACT      (1ul << 10)
#define MONITORACT     (1ul << 8)
#define SVCALLACT      (1ul << 7)
#define USGFAULTACT    (1ul << 3)
#define BUSFAULTACT    (1ul << 1)
#define MEMFAULTACT    (1ul)

/* Other defs */
#define NVIC_PRIO_BITS          4

/** @brief Default mem perm mode for task ram, Normal+Sharable+Write-thru */
#define MSEL_SRAM_MEM_MODE MPU_RASR_TEX(0x0)|MPU_RASR_C|MPU_RASR_S

/** @brief An overlay struct for accessing individual registers after
 * they've been saved on the stack */
typedef struct {
    uint32_t  control;
    
    uint32_t  r4;
    uint32_t  r5;
    uint32_t  r6;
    uint32_t  r7;
    uint32_t  r8;
    uint32_t  r9;
    uint32_t  r10;
    uint32_t  r11;
    uint32_t  r0;
    uint32_t  r1;
    uint32_t  r2;
    uint32_t  r3;
    uint32_t  r12;
    uint32_t  lr;
    uint32_t  pc;
    uint32_t  xpsr;
} arm_saved_regs;

/** @brief Emit breakpoint instruction to trap in debugger */
#define ARCH_EMIT_BREAKPOINT() { __asm__ volatile ( "bkpt 0" ); } (void)0

/** @brief Emit syscall instruction (args must be setup
 * manually). Also note that this will force return the function, so
 * no need to handle the result variable. */
#define ARCH_DO_SYSCALL(res)                        \
    {                                               \
        __asm__ volatile(                           \
            "svc #0          \n"                    \
        );                                          \
    } (void)0

/* Interrupts with priority value less than this are never masked
 * out. Only the faulting interrupts should be given these
 * priorities */
#define ARCH_ISR_MASK_PRIO 4

/* All interrupts should have prio < than this */
#define ARCH_ISR_BASE_PRIO ((1ul<<NVIC_PRIO_BITS)-1)

/** @brief prevent all exceptions other than NMI */
#define ARCH_DISABLE_INTERRUPTS()	\
    __asm volatile			\
    (					\
	"push {r0}            \n"	\
	"mov r0, %0           \n"	\
	"msr basepri,   r0    \n"	\
	"pop  {r0}            \n"	\
	::"I"(ARCH_ISR_MASK_PRIO)	\
    )

/** @brief stop masking interrupts */
#define ARCH_ENABLE_INTERRUPTS()	\
    __asm volatile			    \
    (			                \
	"push {r0}            \n"	\
	"mov r0, %0           \n"	\
	"msr basepri, r0      \n"	\
	"pop  {r0}            \n"	\
	::"I"(ARCH_ISR_BASE_PRIO)	\
    )

/** @brief like DISABLE INTERRUPTS but save the overriden prio so that
 * on EXIT_CRITICAL we don't enable interrupts if they were disabled
 * on ENTER_CRITIAL */
#define ARCH_ENTER_CRITICAL()			\
    __asm volatile				\
    (						\
	"push {r0,r1}                \n"	\
	"ldr r1, =msel_saved_basepri \n"	\
	"mrs r0, basepri             \n"	\
	"str r0, [r1]                \n"	\
	"mov r0, %0                  \n"	\
	"msr basepri,   r0           \n"	\
	"pop  {r0,r1}                \n"	\
	::"I"(ARCH_ISR_MASK_PRIO)		\
    )

/** @brief load saved intr priority */
#define ARCH_EXIT_CRITICAL()			\
    __asm volatile				\
    (						\
	"push {r0}                   \n"	\
	"ldr r0, =msel_saved_basepri \n"	\
	"ldr r0, [r0]                \n"	\
	"msr basepri,   r0           \n"	\
	"pop  {r0}                   \n"	\
    )


/** @brief saves context for an ISR. r1 must point to saved stack vars on exit. */
#define ARCH_ISR_CONTEXT_SAVE()						\
    __asm__ volatile							\
    (									\
    /* Abort if we preempted another interrupt */			\
    "tst lr, #0x4                \n"					\
    "ITE NE                      \n"					\
    "mrsne r1,psp                \n" /* load SP if PSP is active */	\
    "bxeq  lr                    \n" /* bail if MSP is active */	\
    									\
    /* save the non automatic registers and update the caller's stack pointer */ \
    "stmdb r1!, {r4-r11}         \n"					\
    "ldr r2, =msel_active_task   \n" /* r1 = (msel_tcb**)(msel_active_task) */ \
    "ldr r2, [r2]                \n" /* r1 = (msel_tcb*)(*r1) */	\
    "str r1, [r2]                \n" /* saved stack is first member of active task */ \
    "push {lr}                   \n" /* save LR for later pop {pc} */	\
    )


/** @brief restores context for an ISR. */
#define ARCH_ISR_CONTEXT_RESTORE_XX(arg)			   \
    __asm__ volatile						   \
    (								   \
    /* r3 = *msel_active_task */                                   \
    "ldr r3, =msel_active_task   \n"				   \
    "ldr r3, [r3]                \n"                               \
    								   \
    /*  r1 = saved stack ptr @ ((uint32_t*)r3)[0] */		   \
    "ldr r1, [r3]                \n"                               \
								   \
    /* Optionally, overwrite saved r0 w/ current value */	   \
    arg								   \
                                                                   \
    /* restore the saved non-auto registers */                     \
    "ldmia r1!, {r4-r11}         \n"                               \
                                                                   \
    /* ensure new stack is active */                               \
    "msr psp, r1                 \n"                               \
                                                                   \
    /* r2 = saved CTRL state @ ((uint32_t*)r3)[0] */		   \
    "ldr r2, [r3, #4]            \n"                               \
                                                                   \
    /* set exec perms */                                           \
    "mrs r1,control              \n"                               \
    "bfi r1, r2, #0, #1          \n"                               \
    "msr control, r1             \n"                               \
                                                                   \
    /* ret into resumed task */					   \
    "pop  {pc}"							   \
    )

#define ARCH_ISR_CONTEXT_RESTORE_W_RET() ARCH_ISR_CONTEXT_RESTORE_XX("str r0, [r1, #32]\n")
#define ARCH_ISR_CONTEXT_RESTORE()       ARCH_ISR_CONTEXT_RESTORE_XX("\n")

/** @brief suspends CPU until externally interrupted. Note that this
 * will suspend the SYSTICK timer too, so an external signal is needed
 * to ever wake up again */
#define ARCH_WAIT_FOR_INTERRUPT() \
    __asm volatile("wfi")


/* *MUST* be inline to avoid stack frame modification */
#define load_saved_regs()						\
    register arm_saved_regs *regs = NULL;				\
    /* TODO: kill current process and/or alert something */		\
    __asm__ volatile(							\
    /* Determine if MSP or PSP was active */				\
	"tst lr, #0x4                \n"				\
	"ITE EQ                      \n"				\
	"mrseq r2,psp                \n" /* load SP if PSP is active */	\
	"mrsne r2,msp                \n"				\
	"stmdb r2!, {r4-r11}         \n"				\
        "mov %0, r2                  \n"				\
	::"r"(regs)							\
	);								\
									\



#endif

