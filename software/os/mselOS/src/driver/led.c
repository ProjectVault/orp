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
#include "led.h"
#include "arch.h"

inline void msel_init_led()
{
    arch_init_led();
}

inline void msel_led_on(ledio_t led)
{
    msel_led_set(led, MSEL_LED_ON);
}

inline void msel_led_off(ledio_t led)
{
    msel_led_set(led, MSEL_LED_OFF);
}

inline void msel_led_set(ledio_t led, ledstatus_t val)
{
    arch_led_set(led,val);
}
