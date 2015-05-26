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
#ifndef _MSEL_UUID_H
#define _MSEL_UUID_H

/* Only use these UUID types to avoid type issues */
typedef char uuid_t[16];
typedef char uuid_str_t[37];


#ifndef UUID_NULL 
#define UUID_NULL
extern const uuid_t uuid_null;
#endif


int  uuid_to_str(uuid_str_t *, const uuid_t);
int  uuid_from_str(uuid_t *, const uuid_str_t);
int  uuid_cmp(const uuid_t, const uuid_t);
int  uuid_is_null(const uuid_t);
void uuid_cpy(uuid_t*, const uuid_t);
#endif
