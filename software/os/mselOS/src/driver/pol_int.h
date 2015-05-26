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
#ifndef _MSEL_DRIVER_POL_INT_H_
#define _MSEL_DRIVER_POL_INT_H_

#include <msel.h>
#include <msel/pol.h>

#include "gpio.h"
#include "led.h"

/* Timings  (systick is currently configured to tick @ 100Hz */
#define POL_WAIT_PERIOD  100 * 10 /* 10 secs */
#define POL_BLINK_PERIOD 100 / 2  /* 1/2 second */

/* IO Mappings */
#define POL_LED    MSEL_LED_0
#define POL_SWITCH MSEL_GPIO_5 

/* State parameters for suspended system call */
typedef struct
{
    pol_t* retptr;
} msel_ws_pol;

/* Handler */
msel_status msel_proof_of_life(pol_t* ret);

/* Module init */
void msel_init_pol();


#endif
