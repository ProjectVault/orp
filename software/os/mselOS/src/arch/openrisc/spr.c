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
#include "spr.h"


/* Read a 32-bit SPR */
inline uint32_t spr_read(uint32_t regnum)
{
    register unsigned int tmp;
    __asm __volatile__ ("l.mfspr %0, %1, 0 \n" : "=r"(tmp) : "r"(regnum));
    return tmp;
}

/* Write a 32bit SPR */
inline void spr_write(uint32_t regnum, uint32_t val)
{
    __asm __volatile__ ("l.mtspr %0, %1, 0 \n" :: "r"(regnum), "r"(val));
}
