/** @file pol.h

    API for "Proof of Life" allowing for verification of a real human
    presence.

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
#ifndef _MSEL_INC_POL_H_
#define _MSEL_INC_POL_H_

#include <msel.h>

typedef enum
{
    POL_TIMEOUT = 0,
    POL_USER_PRESENT
} pol_t;


/* @brief Provides a means for a task to verify that a real human user
   is present for any secure operation that the task may be about to
   perform. A call to this function generates some sort of externally
   detectable signal that the user should take as a queue to assert
   thier presence. 

   While the exact details of how this works is architecture/platform
   specific, this usually takes the form of a status LED that blinks
   until a user toggles the value of some physical switch or touch
   interface. The user is given some number of seconds to respond
   before a timeout occurs.

   @param ret Result of the pol operation. TIMEOUT=no user presence
   detected; USER_PRESENT=The pol IO changed states after the pol
   operation started

   @return On successful completion, MSEL_OK will be returned and
   'ret' will contain the result.
*/
msel_status proof_of_life(pol_t* ret);

#endif
