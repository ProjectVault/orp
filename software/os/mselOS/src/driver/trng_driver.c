/** @file trng_driver.c

    This file contains the syscall to access the TRNG 
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

#include <stdint.h>
#include <msel.h>
#include "arch.h"
#include "trng_driver.h"

/** @brief read a new random byte from the RNG*/
msel_status msel_trng_read(uint8_t* out)
{
    return arch_trng_read(out);
}



