/** @file mtc.h 

    Monotonic counter API

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

#ifndef _MSEL_INC_MTC_H_
#define _MSEL_INC_MTC_H_

#include <stdlib.h>
#include <stdint.h>

#include <msel.h>

typedef uint64_t mtc_t;

/** @brief This provides access to the monotonic counter. The only
 * operation permitted on this counter (aside from resetting it via
 * "msel_provision") is to return the current value and increment it
 * transparently. This ensures that every call made to this function
 * will return a unique value called until 2^64 times or the device is
 * re-provisioned. */
msel_status mtc_read_increment(mtc_t* val);

#endif
