/** @file isr.h
   
   contains all the declarations for interrupt handlers 
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

#ifndef _MSEL_ISR_H_
#define _MSEL_ISR_H_

#include "m3.h"


/* ISRs are all declared naked for pure ASM interrupt handling */

void NMI_Handler()        __attribute__((naked));
void HardFault_Handler()  __attribute__((naked));
void MemManage_Handler()  __attribute__((naked));
void BusFault_Handler()   __attribute__((naked));
void UsageFault_Handler() __attribute__((naked));
void SVC_Handler()        __attribute__((naked));
void SysTick_Handler()    __attribute__((naked));

void msel_init_isr();

#endif 
