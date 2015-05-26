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
#include "driver/mtc.h"

msel_status arch_mtc_read_increment(mtc_t* val)    
{
    return MSEL_ENOTIMPL;
}

msel_status arch_mtc_write(mtc_t* val)
{
    return MSEL_ENOTIMPL;
}
