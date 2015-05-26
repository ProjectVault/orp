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
#include <msel.h>
#include <msel/syscalls.h>
#include <msel/stdc.h>

#include "os/task.h"

#include "pol_int.h"

msel_status proof_of_life(pol_t* ret)
{
    return msel_svc(MSEL_SVC_POL, ret);
}

msel_status msel_proof_of_life(pol_t* ret)
{

    static uint64_t starting_systicks = 0;
    static gpio_t   button_val;
    uint64_t        elapsed;
    
    /* Validate args */
    if(!ret)
        return MSEL_EINVAL;
    
    /* Main can't be suspended, so it may not call this */
    if(msel_active_task_num == MSEL_TASK_MAIN)
        return MSEL_EINVAL;

    /* Only one task at a time may initiate this operation */
    for(size_t i=0; i<MSEL_TASKS_MAX;i++)
    {
        if(msel_task_list[i].wait_op == MSEL_TASK_WAIT_POL)
            if(i != msel_active_task_num)
                return MSEL_EBUSY;
    }

    int         timedout = 0;

    /* If we're already started the POL operation */
    if(!starting_systicks)
    {
        starting_systicks = msel_systicks;
        button_val = msel_gpio_read(POL_SWITCH);
    }

    elapsed = msel_systicks - starting_systicks;
    
    /* Check for timeout */
    if(elapsed > POL_WAIT_PERIOD)
        timedout = 1;
        
    /* Update indicator leds */
    if(elapsed % POL_BLINK_PERIOD > POL_BLINK_PERIOD / 2)
        msel_led_off(POL_LED);
    else
        msel_led_on(POL_LED);
        
    /* If button state has not changed, keep waiting */
    if(msel_gpio_read(POL_SWITCH) == button_val && !timedout)
    {
        msel_active_task->wait_op = MSEL_TASK_WAIT_POL;
        msel_active_task->state.pol.retptr = ret;
        msel_task_schedule();
        return MSEL_ESUSP;
    }

    /* Clean up static state because the syscall is not being suspended */
    starting_systicks = 0;
    button_val = 0;
    msel_led_off(POL_LED);

    *ret = (timedout)?POL_TIMEOUT:POL_USER_PRESENT;

    return MSEL_OK;
}

void msel_init_pol()
{
    /* Make sure our button is configured as an input */
    msel_gpio_config(POL_SWITCH, MSEL_GPIO_INPUT);
}
