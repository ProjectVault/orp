/** @file arch.h
    
    This is a hardware-abstract interface to the underlying
    platform. Each architecture/platform must provide it's own
    implementation of this API 

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

#ifndef _MSEL_ARCH_H
#define _MSEL_ARCH_H

/* This is sort of a virtual interface for the platform-agnostic
 * code. Actual definitions of these functions reside in thier
 * respective 'arch' directories and are included based on build
 * configuration. */

#include <stdint.h>

#include <msel.h>
#include <msel/mtc.h>
#include <msel/master_key.h>

#include "config.h"
#include "os/task.h"

/* Arch-generic driver includes define APIs between os and libarch */
#include "driver/gpio.h"
#include "driver/led.h"
#include "driver/aes_driver.h"
#include "driver/sha_driver.h"
#include "driver/ecc_driver.h"
#include "driver/ffs_driver.h"

/* TODO: nothing external to arch/$os/ should ever reference values directly from these headers! */
#ifdef BUILD_ARCH_ARM
  #include "arm/m3.h"
#else
  #ifdef BUILD_ARCH_OPENRISC
    #include "openrisc/or1k.h"
  #else
    #error "Unknown build architecture"
  #endif
#endif

/* Arch-specific helper functions */
void arch_init_isr();

/* System Module */
void        arch_systick_handler();
void        arch_task_setup_mm(msel_tcb*);
msel_status arch_mutex_lock();
void        arch_platform_init();

/* Task Module */
void        arch_init_task();
msel_status arch_task_create(msel_tcb*);
void        arch_task_launch_main(msel_tcb*);
void        arch_task_cleanup(msel_tcb*);
void*       arch_get_task_heap();

/* UART module */
int  arch_init_uart();
void arch_uart_putc(char c);
int  arch_uart_is_writable();

/* GPIO module */
void   arch_gpio_config(gpioid_t ios, gpioconf_t conf);
void   arch_gpio_write(gpioid_t ios, gpio_t vals);
gpio_t arch_gpio_read(gpioid_t ios);

/* LEDs module */
void arch_init_led();
void arch_led_set(ledio_t led, ledstatus_t val);

/* MTC module */
msel_status arch_mtc_read_increment(mtc_t* val);
msel_status arch_mtc_write(mtc_t* val);

/* Master Key Module */
msel_status arch_master_key_read(mkey_ptr_t val);
msel_status arch_master_key_write(mkey_ptr_t val);

/* TRNG Module */
msel_status arch_trng_read(uint8_t* out);

/* AES Module */
msel_status arch_do_hw_aes(aes_driver_ctx_t* ctx);

/* SHA Module */
msel_status arch_do_hw_sha(sha_data_t *data);

/* ECC Module */
msel_status arch_hw_ecc_mul(ecc_ctx_t* ctx);

/* FauxFS Module

 TODO: This API is hard-coded to match the ffs hw core for
 Geophyte. It should be made more generic to support different
 underlying implementations (e.g., FFS over USB-OTG)

*/
msel_status arch_ffs_wfile_read(ffs_packet_t *pkt);
void        arch_ffs_wfile_set_status(uint8_t status, uint8_t nonce);
uint8_t     arch_ffs_wfile_get_status();
msel_status arch_ffs_rfile_write(ffs_packet_t *pkt);
void        arch_ffs_rfile_clear();
uint8_t     arch_ffs_rfile_get_status();

#endif
