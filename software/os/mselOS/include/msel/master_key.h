/** @file master_key.h

    (privileged) API for obtaining setting the master key
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

#ifndef _MSEL_INC_MASTER_KEY_H_
#define _MSEL_INC_MASTER_KEY_H_

#include <stdint.h>

#include <msel.h>

typedef uint8_t* mkey_ptr_t;

#define MASTER_KEY_SIZE 256

/** @brief Reads the saved master key from non-volatile storage. This
    function accesses privileged IO ranges and thus may not be called
    from a task. It is possible, however, to utilize this during
    system initialization before the call to msel_start.
  
    @param val Pointer to key structure where result will be stored.
*/
msel_status msel_master_key_read(mkey_ptr_t val);

/** @brief Writes a new master key value into non-volatile
    storage. This function accesses privileged IO ranges and thus may
    not be called from a task. It is possible, however, to utilize
    this during system initialization before the call to
    msel_start. If a task needs to set a new key, the "msel_provision"
    API is provided as a means to do so without requiring system
    privileges or the ability to read older keys.
  
    @param val Pointer to new key structure.
*/
msel_status msel_master_key_write(mkey_ptr_t val);


#endif
