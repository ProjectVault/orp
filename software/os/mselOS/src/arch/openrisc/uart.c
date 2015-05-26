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
#include "arch.h"
#include "driver/uart.h"

#ifndef UART_ADDR
#define UART_ADDR 0x90000000
#endif

/* 16550 Registers */
#define UART16550_DATA      0
#define UART16550_INTR_EN   1
#define UART16550_DIV_LSB   0
#define UART16550_DIV_MSB   1
#define UART16550_INTR_CTRL 2
#define UART16550_LINE_CTRL 3
#define UART16550_MDM_CTRL  4
#define UART16550_LINE_STAT 5
#define UART16550_MDM_STAT  6
#define UART16550_SCRATCH   7

uint8_t or1k_uart_inb(uint32_t portoff)
{
    register uint8_t resb;
    
    __asm __volatile(
        "l.movhi r29, %1      \n"
        "l.ori   r29, r29, %2 \n"
        "l.add   r29, r29, %3 \n"
        "l.lbz   %0, 0(r29)     \n"
        :"=r"(resb)
        :"i"((UART_ADDR >> 16)&0xffff),
         "i"(UART_ADDR & 0xffff),
         "r"(portoff)
        :"r29"
    );

    return resb;
}

void or1k_uart_outb(uint32_t portoff, uint8_t byte)
{
    __asm __volatile(
        "l.movhi r29, %0      \n"
        "l.ori   r29, r29, %1 \n"
        "l.add   r29, r29, %2 \n"
        "l.sb    0(r29),  %3  \n"
        ::"i"((UART_ADDR >> 16)&0xffff),
          "i"(UART_ADDR & 0xffff),
          "r"(portoff), "r"(byte)
        :"r29"
    );
}

void arch_uart_putc(char c)
{
    or1k_uart_outb(UART16550_DATA, c);
}

int arch_uart_is_writable()
{
    return !!(or1k_uart_inb(UART16550_LINE_STAT) & 0x20);
}

int arch_init_uart()
{
    /* Check presence via scratch reg */
    {
        uint8_t v;
        v = or1k_uart_inb(UART16550_SCRATCH); 
        or1k_uart_outb(UART16550_SCRATCH, ~v);
        if(or1k_uart_inb(UART16550_SCRATCH) != (uint8_t)~v)
            return 0;
    }
    
    /* Set BAUD rate to 115200 */
    or1k_uart_outb(UART16550_LINE_CTRL, 0x80); /* Enable divisor access bit */
    or1k_uart_outb(UART16550_DIV_LSB, 0x01); 
    or1k_uart_outb(UART16550_DIV_MSB, 0x00); 
    or1k_uart_outb(UART16550_LINE_CTRL, 0x00); /* clear divisor access bit */
    
    /* Set data bits to 8, 1 stop bit, no parity */
    or1k_uart_outb(UART16550_LINE_CTRL, 0x03);

    /* Enable FIFOs, clear them and set 14 byte threshold */
    or1k_uart_outb(UART16550_INTR_CTRL, 0xC7);
    
    /* Enable interrupts */
    or1k_uart_outb(UART16550_MDM_CTRL, 0x0B);

    return 1; /* non-zero return enables uart driver */
}
