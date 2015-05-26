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
#include "or1k.h"

#include "driver/gpio.h"

/* Geophyte Platform only implements one 8-bit bit GPIO port */

#define OR1K_GPIO_PORT0 ((uint8_t*)0x91000000)
#define OR1K_GPIO_CONF0 ((uint8_t*)0x91000001)

void arch_gpio_config(gpioid_t ios, gpioconf_t conf)
{
    /* Save port config values here instead of reading back every time */
    static uint8_t port0_conf_shadow = 0;

    uint8_t port0_ios;

    port0_ios = (ios & 0xff);

    if(port0_ios)
    {
        /* clear bits for input mode */
        if(conf == MSEL_GPIO_INPUT)
            port0_conf_shadow &= ~port0_ios;
        else /* set bits */
            port0_conf_shadow |= port0_ios;
        
        *OR1K_GPIO_CONF0 = port0_conf_shadow;
    }    
}


void arch_gpio_write(gpioid_t ios, gpio_t vals)
{
    static uint8_t port0_vals_shadow = 0;

    uint8_t port0_ios;
    uint8_t port0_vals;

    port0_ios  = (ios & 0xff);
    port0_vals = (vals & 0xff);

    if(port0_ios)
    {
        port0_vals_shadow &= ~port0_ios;
        port0_vals_shadow |= port0_ios & port0_vals;
        *OR1K_GPIO_PORT0 = port0_vals_shadow;
    }
}

gpio_t arch_gpio_read(gpioid_t ios)
{
    uint8_t port0_ios;

    gpio_t retval = 0;

    port0_ios = (ios & 0xff);

    if(port0_ios)
        retval = (*OR1K_GPIO_PORT0) & port0_ios;

    return retval;
}

