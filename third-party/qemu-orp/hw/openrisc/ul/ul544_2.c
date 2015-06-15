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

#include "ul544_2.h"

/*
 * compute the modular exponentiation a^e mod N for a ul544
 */
void ul544_modexp(ul544 x, const ul544 a, const ul544 e, const mod544 N)
{
    ul544_set(x, a);

    int i;
    int start = -1;
    for (i = 543; i >= 0 ; --i)
    {
        if (start == -1)
        {
            if (ul544_testbit(i, e)) start = i;
            continue;
        }

        ul544_modmul(x, x, x, N);   
        if (ul544_testbit(i, e))
            ul544_modmul(x, a, x, N);
    }
}

/*
 * find the quadratic reside of n for a p with S = 1
 */
void ul544_modsqrt_S1(ul544 r1, ul544 r2, const ul544 n, const mod544 p)
{
    ul544 one; ul544_set_ui(one, 1);
    ul544 exponent; ul544_add(exponent, p->n, one);
    ul544_rshift(exponent, exponent, 2);
    ul544_modexp(r1, n, exponent, p);
    ul544_sub(r2, p->n, r1);
}

/* 
 * find the quadratic residue of n (assume that n has one, undefined behavior if not)
 *
 * inputs: Q and S satisfy p - 1 = Q2^S with Q odd and S > 1 (note that S can't be 
 *           larger than 544) 
 *         some non-quadratic residue z of p
 *
 * outputs: r1 satisfying r1^2 = n (mod p) and r2 = p - r1
 */
void ul544_modsqrt(ul544 r1, ul544 r2, const ul544 n, const ul544 Q, uint32_t S, const ul544 z, const mod544 p)
{
    ul544 one; ul544_set_ui(one, 1);
    ul544 one_mont; ul_to_montgomery(one_mont, one, p);

    ul544 c; ul544_modexp(c, z, Q, p);
    ul544 exponent; ul_add(exponent, Q, one); ul544_rshift(exponent, exponent, 1);
    ul544 R; ul544_modexp(R, n, exponent, p);
    ul544 t; ul544_modexp(t, n, Q, p);
    uint32_t M = S;

    while (ul544_cmp(t, one_mont))
    {
        uint32_t i = 0;
        ul544 test; ul544_set(test, t);
        do
        {
            ++i;
            ul544_modmul(test, test, test, p);
        } while(ul544_cmp(test, one));

        uint32_t j;
        ul544 b; ul544_set(b, c);
        for (j = 0; j < M - i - 1; ++j)
            ul544_modmul(b, b, b, p);

        ul544_modmul(R, R, b, p);
        ul544_modmul(b, b, b, p);
        ul544_modmul(t, t, b, p);
        ul544_set(c, b);
        M = i;
    }

    ul544_set(r1, R);
    ul544_sub(r2, p->n, R);
}



