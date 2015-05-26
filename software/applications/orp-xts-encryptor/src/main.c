/* Copyright 2015, Google Inc.

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
#include <stddef.h>

#include <msel.h>
#include <msel/stdc.h>
#include <msel/endpoints.h>

void xts_encryptor_task(void *arg, const size_t arg_sz);
extern const uint8_t xts_encryptor_endpoint[32];

void get_task(const uint8_t **endpoint, void (**task_fn)(void *arg, const size_t arg_sz),
        uint16_t *port, const uint8_t* data)
{
    *endpoint = NULL;
    *task_fn = NULL;

    if (msel_memcmp(xts_encryptor_endpoint, data, ENDPT_HASH_SIZE) == 0) {
        *endpoint = xts_encryptor_endpoint;
        *task_fn = xts_encryptor_task;
        return;
    }
}

int main() {
    /* let msel initialize itself */
    msel_init();

    /* give control over to msel */
    msel_start();
    
    while(1);

    /* never reached */
    return 0;
}
