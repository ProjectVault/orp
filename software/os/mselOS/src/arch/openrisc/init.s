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
	.global  _stack_top
	.global context_restore
.section .vectors,"ax"

/* Reset lands here */
.org 0x000100
_reset_vector:
		l.nop
	l.nop
		l.movhi r3, hi( _start)
		l.ori   r3,r3, lo(_start)
		l.jr    r3
		l.nop

/* This macro defines an interrupt handler that will save context,
	jump to a C handler, restore context and return from the
	interrupt. Since each handler only gets 0x100 of space and fast
	context switching is not implemented in OR1200, the actual context
	save/restore code is contained in a helper stub in .text because
	just 32 pushes and 32 pops is already 0x100 bytes of code..
*/
.macro handler symname orgaddr
	.org \orgaddr

	/*

	SP (r1) comes from an un-trusted task - DO NOT USE

	save r1 in r31 pending validation

	save registers to end of ram
	
	*/

	l.addi r31, r1, 0
	l.movhi r1,     hi( end )
	l.ori   r1, r1, lo( end )

    /* make some room on the stack.

	   128 bytes reserved by leaf functions w/o stack frames

	   8 bytes for stack frame (saved ret and prev fp)
	
	   128 bytes for 32 regs (r2-r31 + ESR + EPC)
	*/ 
    l.addi r1,r1,-0x108         /* r1 is now (arch_saved_regs*) */

	/* save the first few regs so we can save the addr of the handler */
    l.sw 0x00(r1), r2
    l.sw 0x04(r1), r3
	l.sw 0x08(r1), r4
    l.sw 0x0c(r1), r5
    l.sw 0x10(r1), r6
    l.sw 0x14(r1), r7
    l.sw 0x18(r1), r8
    l.sw 0x1c(r1), r9
    l.sw 0x20(r1), r10
    l.sw 0x24(r1), r11
    l.sw 0x28(r1), r12
    l.sw 0x2c(r1), r13
    l.sw 0x30(r1), r14
    l.sw 0x34(r1), r15

	/* Save the addr of the handler in r30 since it will be preserved in any C fn calls */
	l.movhi r30,     hi( \symname )
	l.ori   r30, r30, lo( \symname )

	/* Goto save_context */
	l.j context_save
	
	l.nop
.endm	

	
/* General Interrupts */
handler BusError,               0x000200
handler DataPageFault,          0x000300
handler InstructionPageFault,   0x000400
handler TickTimerHandler,       0x000500
handler AlignmentError,         0x000600
handler IllegalInstruction,     0x000700
handler ExternalInterrupt,      0x000800
handler DTLBMiss,               0x000900
handler ITLBMiss,               0x000a00
handler RangeException,         0x000b00
handler SystemCallHandler,      0x000c00
handler FloatingPointException, 0x000d00
handler TrapException,          0x000e00
handler FauxFileSystemWrite,    0x001000
handler FauxFileSystemReadAck,  0x001100
	
/* everything else up to 0x2000 is reserved */
		
.section .text

_start:

	        /* initial SR */
		l.ori   r2, r0, 0x8001
		l.mtspr r0, r2, 17
	
		/* Zero out all the regs */
		l.xor  r2, r0, r0
		l.xor  r3, r0, r0
		l.xor  r4, r0, r0
		l.xor  r5, r0, r0
		l.xor  r6, r0, r0
		l.xor  r7, r0, r0
		l.xor  r8, r0, r0
		l.xor  r9, r0, r0
		l.xor r10, r0, r0
		l.xor r11, r0, r0
		l.xor r12, r0, r0
		l.xor r13, r0, r0
		l.xor r14, r0, r0
		l.xor r15, r0, r0
		l.xor r16, r0, r0
		l.xor r17, r0, r0
		l.xor r18, r0, r0
		l.xor r19, r0, r0
		l.xor r20, r0, r0
		l.xor r21, r0, r0
		l.xor r22, r0, r0
		l.xor r23, r0, r0
		l.xor r24, r0, r0
		l.xor r25, r0, r0
		l.xor r26, r0, r0
		l.xor r27, r0, r0
		l.xor r28, r0, r0
		l.xor r29, r0, r0
		l.xor r30, r0, r0
		l.xor r31, r0, r0

/* Setup system stack as positioned by the linker */	
set_stack_ptr:	
    l.movhi r1, hi(end)
    l.ori   r1, r1, lo(end)

/* BSS must be manually zero'd */	
clear_bss:	
        l.movhi r2, hi(_bss_beg)
        l.ori   r2, r2, lo(_bss_beg)
        l.movhi r3, hi(_bss_end)
        l.ori   r3, r2, lo(_bss_end)
cb_loop:
        l.sfeq  r2, r3
        l.bf    cb_loop_end
        l.nop

        l.sw    0x0(r2), r0
        l.addi  r2, r2, 0x4
        l.j             cb_loop
        l.nop
cb_loop_end:

/* .data resides in ROM and must be copied to ram */
copy_data:
	l.movhi r2,     hi(_data_loadaddr)
	l.ori   r2, r2, lo(_data_loadaddr)
	l.movhi r3,     hi(_data_beg)
	l.ori   r3, r3, lo(_data_beg)
	l.movhi r4,     hi(_data_end)
	l.ori   r4, r4, lo(_data_end)
cd_loop:
	l.sfeq  r3,r4
	l.bf    cd_loop_end
	l.nop
	l.lbs   r5, 0x0(r2)  /* tmp = rom[r2] */
	l.sb    0x0(r3), r5  /* ram[r3] = tmp */
	l.addi  r2, r2, 1
	l.addi  r3, r3, 1
	l.j     cd_loop
	l.nop
cd_loop_end:	

/* we're done here, so... */
jump_to_main:
        l.movhi r2, hi(main)
        l.ori   r2, r2, lo(main)
        l.jr    r2
        l.nop


	/* r13  is set by the interrupt vector before branching here and
	contains the addr of the C handler to call */
context_save:
	/* Continue saving registers at r16 */
    l.sw 0x38(r1), r16
    l.sw 0x3c(r1), r17
    l.sw 0x40(r1), r18
    l.sw 0x44(r1), r19
    l.sw 0x48(r1), r20
    l.sw 0x4c(r1), r21
    l.sw 0x50(r1), r22
    l.sw 0x54(r1), r23

    l.sw 0x58(r1), r24
    l.sw 0x5c(r1), r25
    l.sw 0x60(r1), r26
    l.sw 0x64(r1), r27
    l.sw 0x68(r1), r28
    l.sw 0x6c(r1), r29
    l.sw 0x70(r1), r30
    l.sw 0x74(r1), r31

    /* Also save exception status registers */
    l.mfspr r14, r0, 64
    l.mfspr r15, r0, 32
    l.sw 0x78(r1), r14
    l.sw 0x7c(r1), r15

	/* Clear link register to keep gdb sane */
	l.addi   r9, r0, 0

	/* Save context to task struct */
	l.movhi  r15,      hi(or1k_save_context)
	l.ori    r15, r15, lo(or1k_save_context)
	l.jalr   r15
	l.nop
	
	/* skip the handler if zero retval... schedule was called, just restore the next task */
	l.sfeq   r11, r0
	l.bf     context_restore
	l.nop
	
	/* restore C fn params for handler (e.g. for svcalls) */
    l.lwz r3, 0x04(r1) 
    l.lwz r4, 0x08(r1) 
    l.lwz r5, 0x0c(r1) 
    l.lwz r6, 0x10(r1) 
    l.lwz r7, 0x14(r1) 
    l.lwz r8, 0x18(r1)
	
	/* Call the C exception handler */
	l.jalr r30
	l.nop

/* Restore previous context after the C handler has processed what it
	needs to. This can be called directly in order to launch a task
	from an interrupt context. However, take care that r10 is non-zero
	when doing so. */
context_restore:

	/* Update perf counters on resume */
	l.movhi r29,      hi(msel_task_update_ctrs_resume)
	l.ori   r29, r29, lo(msel_task_update_ctrs_resume)
	l.jalr  r29
	l.nop
	
	/* Clear link register to keep gdb sane */
	l.addi   r9, r0, 0
	
	/* Read saved context from active_task->stack */
    l.movhi r4,     hi(msel_active_task)
    l.ori   r4, r4, lo(msel_active_task)     /* r4 = (msel_tcb **)&msel_active_task */
    l.lwz   r4, 0(r4)                        /* r4 = (msel_tcb*) *r4 */
    l.lwz   r1, 0(r4)                        /* r1 = tcb->savedstack */

	/* TODO: flush MMU caches */

	/* Note: ESR restore does NOT reenable interrupts and MMUs.. rfe will set SR=ESR */
	l.lwz r4,0x7c(r1)  /* r3 = saved_epcr */
    l.lwz r3,0x78(r1)  /* r2 = saved_esr  */
    l.mtspr r0, r4, 32 /* epcr = r3       */
    l.mtspr r0, r3, 64 /* esr  = r2       */
	
    l.lwz r2, 0x00(r1) /* restore frame ptr */
    l.lwz r3, 0x04(r1) 
    l.lwz r4, 0x08(r1) 
    l.lwz r5, 0x0c(r1) 
    l.lwz r6, 0x10(r1) 
    l.lwz r7, 0x14(r1) 

    l.lwz r8, 0x18(r1) 
    l.lwz r9, 0x1c(r1) 
    l.lwz r10, 0x20(r1)
    l.lwz r11, 0x24(r1)
    l.lwz r12, 0x28(r1)
    l.lwz r13, 0x2c(r1)
    l.lwz r14, 0x30(r1)
    l.lwz r15, 0x34(r1)

    l.lwz r16, 0x38(r1)
    l.lwz r17, 0x3c(r1)
    l.lwz r18, 0x40(r1)
    l.lwz r19, 0x44(r1)
    l.lwz r20, 0x48(r1)
    l.lwz r21, 0x4c(r1)
    l.lwz r22, 0x50(r1)
    l.lwz r23, 0x54(r1)

    l.lwz r24, 0x58(r1)
    l.lwz r25, 0x5c(r1)
    l.lwz r26, 0x60(r1)
    l.lwz r27, 0x64(r1)
    l.lwz r28, 0x68(r1)
    l.lwz r29, 0x6c(r1)
    l.lwz r30, 0x70(r1)
    l.lwz r31, 0x74(r1)

    l.addi r1, r1, 0x108    /* restore stack ptr */

    l.rfe                   /* restore SR and PC */



	
