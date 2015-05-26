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
.syntax unified
.cpu cortex-m3
.thumb

.global _reset
.global pfnVectors
.global context_restore
.global task_launch
	
/*

Exception Vectors
-----------------
	
ARM vectors include 16 required function pointers and 240
optional IRQ handlers depending on specific implmentation of M3

*/

.section .vectors,"a",%progbits
.type pfnVectors, %object
.size pfnVectors, .-pfnVectors

pfnVectors:	
	// Standard exceptions 0-15
	.word exception_stack
	.word _reset
	.word isr_nmi_handler
	.word isr_hardfault_handler
	.word isr_memmanage_handler
	.word isr_busfault_handler
	.word isr_usagefault_handler
	.word 0 // 7-10 are reserved
	.word 0
	.word 0
	.word 0
	.word isr_svc_handler
	.word isr_debugmon_handler
	.word 0
	.word isr_pendsv_handler
	.word isr_systick_handler
	
	.fill 240, 4, 0xffffffff  // Not using IRQ0-IRQ239

/*

Reset Vector
------------

*/

.section .text	

.type _reset, %function

_reset:
_start:	


	/*

	.data is already in ram for qemu build, on hw it
	will need to be initialized explicitly from flash

	*/

zero_bss:
	ldr r0, =_bss_beg
	ldr r1, =_bss_end
	ldr r2, =0
zero_bss_loop:
	cmp r0, r1
	it ne
	strne r2, [r0], #4
	bne zero_bss_loop

jump_to_main:	
	
	blx main
	b .

/*

Interrupt Routines
------------------

*/

.macro ISR symname
	.global isr_\symname
	.type isr_\symname, %function
isr_\symname:
	ldr r3, isr_addr_\symname
	b   context_save
	/* never reached */
	b .
isr_addr_\symname:
	.word \symname
.endm	

ISR nmi_handler
ISR	hardfault_handler
ISR memmanage_handler
ISR busfault_handler
ISR usagefault_handler
ISR svc_handler
ISR debugmon_handler
ISR pendsv_handler
ISR systick_handler

/* Save thread context and jump to handler spcified in R3 */
context_save:
	tst lr, #0x4	
	ITE NE                      					
	mrsne r1,psp

	/* load SP if PSP is active */	
	bxeq  lr                    /* bail if MSP is active (non-reentrant) */	
    									
	/* save the non automatic registers and update the caller's stack pointer */ 
	mrs   r2, control
	stmdb r1!, {r2,r4-r11}         					
	ldr r2, =msel_active_task   /* r1 = (msel_tcb**)(msel_active_task) */ 
	ldr r2, [r2]                /* r1 = (msel_tcb*)(*r1) */	
	str r1, [r2]                /* saved stack is first member of active task */ 

	push {lr}
call_isr:
	blx r3

context_restore:

	/* Update perf counters */
	bl =msel_task_update_ctrs_resume
	

	/* r3 = *msel_active_task */                                   
    ldr r3, =msel_active_task   				   
    ldr r3, [r3]                                               
    								   
    /* r1 = saved stack ptr @ ((uint32_t*)r3)[0] */		   
    ldr r1, [r3]                                               
								   
    /* restore the saved non-auto registers */                     
    ldmia r1!, {r2,r4-r11}                                        

	pop {lr}

    /* ensure new stack is active */                               
    msr psp, r1                                                
                                                                   
    /* set exec perms from saved control value */ 
    msr control, r2                                            

    /* ret into resumed task via saved lr (EXC_RETURN) */
    bx lr


/* This is basically context_restore without returning from an
	interrupt context in order to launch the first task */
task_launch:	

	/* r3 = *msel_active_task */                                   
    ldr r3, =msel_active_task   				   
    ldr r3, [r3]                                               
    								   
    /*  r1 = saved stack ptr @ ((uint32_t*)r3)[0] */		   
    ldr r1, [r3]                                               
								   
    /* r2 = saved CTRL state @ ((uint32_t*)r3)[0] */		   
    ldr r2, [r1, #0]                                           

    /* restore the saved non-auto registers */                     
    ldmia r1!, {r2,r4-r11}                                        

    /* ensure new stack is active */                               
    msr psp, r1                                                
                                                                   
    /* set exec perms from saved control value */ 
    msr control, r2                                            

	/* Flush instruction cache to activate PSP */
	isb

	pop {r0-r3,r12,lr}

	pop {pc}
	
