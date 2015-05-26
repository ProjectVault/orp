/** @file endpoints.h
 * 
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
#ifndef _INC_ENDPOINT_H
#define _INC_ENDPOINT_H

#include <stdint.h>
#include <stdlib.h>

/** @addtogroup ffs_session
 *  @{
 */

/** @brief The size of an application endpoint hash */
#define ENDPT_HASH_SIZE 32

/** @brief Check to see if an incoming session request matches any of the provisioned
 *  endpoints
 *
 *  @param[out] endpoint The requested application endpoint
 *  @param[out] task_fn  The application's 'main' function
 *  @param[out] port     The port or socket for communication with the application
 *  @param data     The incoming data request
 */
void get_task(const uint8_t **endpoint, void (**task_fn)(void *arg, const size_t arg_sz),
        uint16_t *port, const uint8_t* data);

/** @} */

#endif // _INC_ENDPOINT_H
