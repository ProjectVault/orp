/* @file spr.h

   Defines all (non-custom) special purpose registers for the openrisc
   architecture and some routines to help deal with them.
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

#ifndef _ARCH_OPENRISC_SPR_H_
#define _ARCH_OPENRISC_SPR_H_

#include <stdint.h>

/* Generically set/read SPRs */
uint32_t spr_read(uint32_t);
void spr_write(uint32_t, uint32_t);

#define SPR_GROUP_NUM(GROUP_NUM) (((unsigned int)GROUP_NUM)<<11)
#define SPR_REG_NUM(REG_NUM)     ((unsigned int)REG_NUM)

#define SPR_REG(GROUP,REG) (SPR_GROUP_NUM(GROUP) | SPR_REG_NUM(REG))

/****************************************/
/* Special-purpose register definitions */
/****************************************/

/* Group 0: System Control and Status Registers */
#define SPR_VR         SPR_REG(0,  0)
#define SPR_UPR        SPR_REG(0,  1)
#define SPR_CPUCFGR    SPR_REG(0,  2)
#define SPR_DMMUCFGR   SPR_REG(0,  3)
#define SPR_IMMUCFGR   SPR_REG(0,  4)
#define SPR_DCCFGR     SPR_REG(0,  5)
#define SPR_ICCFGR     SPR_REG(0,  6)
#define SPR_DCFGR      SPR_REG(0,  7)
#define SPR_PCCFGR     SPR_REG(0,  8)
#define SPR_VR2        SPR_REG(0,  9)
#define SPR_AVR        SPR_REG(0, 10)
#define SPR_EVBAR      SPR_REG(0, 11)
#define SPR_AECR       SPR_REG(0, 12)
#define SPR_AESR       SPR_REG(0, 13)
#define SPR_NPC        SPR_REG(0, 16)
#define SPR_SR         SPR_REG(0, 17)
#define SPR_PPC        SPR_REG(0, 18)
#define SPR_FPCSR      SPR_REG(0, 20)
#define SPR_ISR(num)  (SPR_REG(0, 21)   + (num&7))
#define SPR_EPCR(num) (SPR_REG(0, 32)   + (num&15))
#define SPR_EEAR(num) (SPR_REG(0, 48)   + (num&15))
#define SPR_ESR(num)  (SPR_REG(0, 64)   + (num&15))
#define SPR_GPR(num)  (SPR_REG(0, 1024) + (num&511))

/* Group 1: Data MMU */
#define SPR_DMMUCR         SPR_REG(1,    0)
#define SPR_DMMUPR         SPR_REG(1,    1)
#define SPR_DTLBEIR        SPR_REG(1,    2)
#define SPR_DATBMR(num)   (SPR_REG(1,    4) + (num & 3))
#define SPR_DATBTR(num)   (SPR_REG(1,    8) + (num & 3))
#define SPR_DTLBW0MR(num) (SPR_REG(1,  512) + (num & 127))
#define SPR_DTLBW0TR(num) (SPR_REG(1,  640) + (num & 127))
#define SPR_DTLBW1MR(num) (SPR_REG(1,  768) + (num & 127))
#define SPR_DTLBW1TR(num) (SPR_REG(1,  896) + (num & 127))
#define SPR_DTLBW2MR(num) (SPR_REG(1, 1024) + (num & 127))
#define SPR_DTLBW2TR(num) (SPR_REG(1, 1152) + (num & 127))
#define SPR_DTLBW3MR(num) (SPR_REG(1, 1280) + (num & 127))
#define SPR_DTLBW3TR(num) (SPR_REG(1, 1408) + (num & 127))
    
/* Group 2: Instruction MMU */
#define SPR_IMMUCR         SPR_REG(2,    0)
#define SPR_IMMUPR         SPR_REG(2,    1)
#define SPR_ITLBEIR        SPR_REG(2,    2)
#define SPR_IATBMR(num)   (SPR_REG(2,    4) + (num & 3))
#define SPR_IATBTR(num)   (SPR_REG(2,    8) + (num & 3))
#define SPR_ITLBW0MR(num) (SPR_REG(2,  512) + (num & 127))
#define SPR_ITLBW0TR(num) (SPR_REG(2,  640) + (num & 127))
#define SPR_ITLBW1MR(num) (SPR_REG(2,  768) + (num & 127))
#define SPR_ITLBW1TR(num) (SPR_REG(2,  896) + (num & 127))
#define SPR_ITLBW2MR(num) (SPR_REG(2, 1024) + (num & 127))
#define SPR_ITLBW2TR(num) (SPR_REG(2, 1152) + (num & 127))
#define SPR_ITLBW3MR(num) (SPR_REG(2, 1280) + (num & 127))
#define SPR_ITLBW3TR(num) (SPR_REG(2, 1408) + (num & 127))

/* Group 3: Data Cache */
#define SPR_DCCR  SPR_REG(3, 0)
#define SPR_DCBPR SPR_REG(3, 1)
#define SPR_DCBFR SPR_REG(3, 2)
#define SPR_DCBIR SPR_REG(3, 3)
#define SPR_DCBWR SPR_REG(3, 4)
#define SPR_DCBLR SPR_REG(3, 5)

/* Group 4: Instruction Cache */
#define SPR_ICCR  SPR_REG(4, 0)
#define SPR_ICBPR SPR_REG(4, 1)
#define SPR_ICBFR SPR_REG(4, 2)
#define SPR_ICBIR SPR_REG(4, 3)
#define SPR_ICBWR SPR_REG(4, 4)
#define SPR_ICBLR SPR_REG(4, 5)

/* Group 5: MAC Unit */
#define SPR_MACLO SPR_REG(5, 1)
#define SPR_MACHI SPR_REG(5, 2)

/* Group 6: Debug Unit */
#define SPR_DVR(num)  (SPR_REG(6, 0) + (num&7))
#define SPR_DCR(num)  (SPR_REG(6, 8) + (num&7))
#define SPR_DMR1       SPR_REG(6,16)
#define SPR_DMR2       SPR_REG(6,17)
#define SPR_DCWR(num) (SPR_REG(6,18) + (num&1))
    
/* Group 7: Performance Counters Unit */
#define SPR_PCCR(num) (SPR_REG(7,0) + (num&7))
#define SPR_PCMR(num) (SPR_REG(7,8) + (num&7))

/* Group 8: Power Management */
#define SPR_PMR SPR_REG(8,0)
    
/* Group 9: Programmable Interrupt Controller */
#define SPR_PICMR  SPR_REG(9,  0)
#define SPR_PICSR  SPR_REG(9,  2)

/* Group 10: Tick Timer */
#define SPR_TTMR   SPR_REG(10, 0)
#define SPR_TTCR   SPR_REG(10, 1)

/* Group 11: Floating Point Unit */
/* Groups 12-23: Reserved for future use */
/* Groups 24-31: Custom Units */

/************************************/
/* Helpers for individual registers */
/************************************/

/* UPR Register */
#define SPR_UPR_TTP ((unsigned int)(1<<10))

/* SR Register */
#define SPR_SR_TEE_SET(val) spr_write(SPR_SR,(spr_read(SPR_SR)&0xFFFFFFFD)|(val&0x1)<<1)
#define SPR_SR_TEE_GET(val) ((spr_read(SPR_SR) >> 1) & 1)

#define SPR_SR_IEE_SET(val) spr_write(SPR_SR,(spr_read(SPR_SR)&0xFFFFFFFB)|(val&0x1)<<2)
#define SPR_SR_IEE_GET(val) ((spr_read(SPR_SR) >> 2) & 1)

#define SPR_SR_SM  (1)
#define SPR_SR_TEE (1<<1) 
#define SPR_SR_IEE (1<<2)
#define SPR_SR_DCE (1<<3)
#define SPR_SR_ICE (1<<4)
#define SPR_SR_DME (1<<5)
#define SPR_SR_IME (1<<6)
#define SPR_SR_FO  (1<<15)

/* TLB Register Flags */

#define TLBMR_V             1
#define TLBMR_PL1           (1 << 1)
#define TLBTR_CC            1
#define TLBTR_CI            (1<<1)
#define TLBTR_WBC           (1<<2)
#define TLBTR_WOM           (1<<3)
#define TLBTR_A             (1<<4)
#define TLBTR_D             (1<<5)
#define TLBTR_URE           (1<<6)
#define TLBTR_UWE           (1<<7)
#define TLBTR_SRE           (1<<8)
#define TLBTR_SWE           (1<<9)
#define TLBTR_SXE           (1<<6)
#define TLBTR_UXE           (1<<7)

/* Shadowed registers by context id */

#define SHADOW_REG(ctx,regnum) ((uint32_t) SPR_REG(0, ctx*32 + regnum))


#define SPR_TTMR_M_SET(val)  spr_write(SPR_TTMR,(spr_read(SPR_TTMR)&0x3FFFFFFF)|(val&0x3)<<30)
#define SPR_TTMR_M_GET       ((spr_read(SPR_TTMR) >> 30) & 0x3)

#define SPR_TTMR_IE_SET(val) spr_write(SPR_TTMR,(spr_read(SPR_TTMR)&0xDFFFFFFF)|(val&0x1)<<29)
#define SPR_TTMR_IE_GET(val) ((spr_read(SPR_TTMR) >> 29) & 1)

#define SPR_TTMR_IP_SET(val) spr_write(SPR_TTMR,(spr_read(SPR_TTMR)&0xEFFFFFFF)|(val&0x1)<<28)
#define SPR_TTMR_IP_GET(val) ((spr_read(SPR_TTMR) >> 28) & 1)

#define SPR_TTMR_TP_SET(val) spr_write(SPR_TTMR,(spr_read(SPR_TTMR)&0xF0000000)|(val&0x0FFFFFFF))

#endif

