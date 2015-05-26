/** @file led.h */
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
#ifndef _MSEL_DRIVER_LED_H_
#define _MSEL_DRIVER_LED_H_

#include <stdint.h>

typedef uint32_t ledio_t;
typedef uint32_t ledstatus_t;

/* Arch-specific code shall use these generic values to map onto the
 * real GPIO pins for each LED */

#define MSEL_LED_OFF 0
#define MSEL_LED_ON  1

#define MSEL_LED_NONE ((ledio_t)0)
#define MSEL_LED_0    ((ledio_t)1<< 0)
#define MSEL_LED_1    ((ledio_t)1<< 1)
#define MSEL_LED_2    ((ledio_t)1<< 2)
#define MSEL_LED_3    ((ledio_t)1<< 3)
#define MSEL_LED_4    ((ledio_t)1<< 4)
#define MSEL_LED_5    ((ledio_t)1<< 5)
#define MSEL_LED_6    ((ledio_t)1<< 6)
#define MSEL_LED_7    ((ledio_t)1<< 7)
#define MSEL_LED_8    ((ledio_t)1<< 8)
#define MSEL_LED_9    ((ledio_t)1<< 9)
#define MSEL_LED_10   ((ledio_t)1<<10)
#define MSEL_LED_11   ((ledio_t)1<<11)
#define MSEL_LED_12   ((ledio_t)1<<12)
#define MSEL_LED_13   ((ledio_t)1<<13)
#define MSEL_LED_14   ((ledio_t)1<<14)
#define MSEL_LED_15   ((ledio_t)1<<15)
#define MSEL_LED_16   ((ledio_t)1<<16)
#define MSEL_LED_17   ((ledio_t)1<<17)
#define MSEL_LED_18   ((ledio_t)1<<18)
#define MSEL_LED_19   ((ledio_t)1<<19)
#define MSEL_LED_20   ((ledio_t)1<<20)
#define MSEL_LED_21   ((ledio_t)1<<21)
#define MSEL_LED_22   ((ledio_t)1<<22)
#define MSEL_LED_23   ((ledio_t)1<<23)
#define MSEL_LED_24   ((ledio_t)1<<24)
#define MSEL_LED_25   ((ledio_t)1<<25)
#define MSEL_LED_26   ((ledio_t)1<<26)
#define MSEL_LED_27   ((ledio_t)1<<27)
#define MSEL_LED_28   ((ledio_t)1<<28)
#define MSEL_LED_29   ((ledio_t)1<<29)
#define MSEL_LED_30   ((ledio_t)1<<30)
#define MSEL_LED_31   ((ledio_t)1<<31)

#define MSEL_LED_ALL  ((ledio_t)~0)

void msel_init_led();
void msel_led_on(ledio_t led);
void msel_led_off(ledio_t led);
void msel_led_set(ledio_t led, ledstatus_t val);

#endif
