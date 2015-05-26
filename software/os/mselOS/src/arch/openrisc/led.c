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

/* The Geophyte board has 4 leds mapped to GPIOs 0-3... simple mode */
#define LED_MASK (MSEL_GPIO_3|MSEL_GPIO_2|MSEL_GPIO_1|MSEL_GPIO_0)

/* Attempts to write msel_leds 4-31 are just ignored */

inline void arch_init_led()
{
    arch_gpio_config(LED_MASK,MSEL_GPIO_OUTPUT);
}

void arch_led_set(ledio_t led, ledstatus_t val)
{
    /* no need to redefine ON/OFF here, polarity is correct. just pass through */
    arch_gpio_write(led & LED_MASK, val & LED_MASK);
}
