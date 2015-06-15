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
#ifndef __UL544_2__
#define __UL544_2__

#include "ul544_0.h"
#include "ul544_1.h"

/*
 * compute the modular exponentiation n^e mod p for a ul544
 */
void ul544_modexp(ul544 x, const ul544 a, const ul544 e, const mod544 N);

/*
 * compute quadratic residues modulo p using Tonelli-Shanks
 */
void ul544_modsqrt_S1(ul544 r1, ul544 r2, const ul544 n, const mod544 p);
void ul544_modsqrt(ul544 r1, ul544 r2, const ul544 n, const ul544 Q, uint32_t S, const ul544 z, const mod544 p);

#endif
