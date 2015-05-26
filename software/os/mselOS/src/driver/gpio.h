/** @file gpio.h */
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
#ifndef _MSEL_DRIVER_GPIO_H_
#define _MSEL_DRIVER_GPIO_H_

#include <stdint.h>

/* The arch-specific code maps up to 32 IOs to a single uint32_t so
 * that OS internal code may access GPIO without platform specific
 * concerns */

#define MSEL_GPIO_INPUT  0
#define MSEL_GPIO_OUTPUT 1

typedef uint32_t gpioid_t;

#define MSEL_GPIO_NONE ((gpioid_t)0)
#define MSEL_GPIO_0    ((gpioid_t)(1<< 0))
#define MSEL_GPIO_1    ((gpioid_t)(1<< 1))
#define MSEL_GPIO_2    ((gpioid_t)(1<< 2))
#define MSEL_GPIO_3    ((gpioid_t)(1<< 3))
#define MSEL_GPIO_4    ((gpioid_t)(1<< 4))
#define MSEL_GPIO_5    ((gpioid_t)(1<< 5))
#define MSEL_GPIO_6    ((gpioid_t)(1<< 6))
#define MSEL_GPIO_7    ((gpioid_t)(1<< 7))
#define MSEL_GPIO_8    ((gpioid_t)(1<< 8))
#define MSEL_GPIO_9    ((gpioid_t)(1<< 9))
#define MSEL_GPIO_10   ((gpioid_t)(1<<10))
#define MSEL_GPIO_11   ((gpioid_t)(1<<11))
#define MSEL_GPIO_12   ((gpioid_t)(1<<12))
#define MSEL_GPIO_13   ((gpioid_t)(1<<13))
#define MSEL_GPIO_14   ((gpioid_t)(1<<14))
#define MSEL_GPIO_15   ((gpioid_t)(1<<15))
#define MSEL_GPIO_16   ((gpioid_t)(1<<16))
#define MSEL_GPIO_17   ((gpioid_t)(1<<17))
#define MSEL_GPIO_18   ((gpioid_t)(1<<18))
#define MSEL_GPIO_19   ((gpioid_t)(1<<19))
#define MSEL_GPIO_20   ((gpioid_t)(1<<20))
#define MSEL_GPIO_21   ((gpioid_t)(1<<21))
#define MSEL_GPIO_22   ((gpioid_t)(1<<22))
#define MSEL_GPIO_23   ((gpioid_t)(1<<23))
#define MSEL_GPIO_24   ((gpioid_t)(1<<24))
#define MSEL_GPIO_25   ((gpioid_t)(1<<25))
#define MSEL_GPIO_26   ((gpioid_t)(1<<26))
#define MSEL_GPIO_27   ((gpioid_t)(1<<27))
#define MSEL_GPIO_28   ((gpioid_t)(1<<28))
#define MSEL_GPIO_29   ((gpioid_t)(1<<29))
#define MSEL_GPIO_30   ((gpioid_t)(1<<30))
#define MSEL_GPIO_31   ((gpioid_t)(1<<31))

typedef uint32_t gpio_t;
typedef uint32_t gpioconf_t;

void   msel_gpio_config(gpioid_t ios, gpioconf_t conf);
void   msel_gpio_write(gpioid_t ios, gpio_t vals);
gpio_t msel_gpio_read(gpioid_t ios);

#endif
