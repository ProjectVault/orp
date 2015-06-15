/** @file ul32.h

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

 *
 */


#ifndef __UL32_H__
#define __UL32_H__

uint32_t ul32_addc(uint32_t *dst, const uint32_t *a, const uint32_t *b);
uint32_t clz(uint32_t x);

#endif /* __UL32_H__ */
