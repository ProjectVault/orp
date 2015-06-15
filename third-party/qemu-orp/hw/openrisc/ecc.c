/** @file ecc.c
 *
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

 * Elliptic curve operations. Arithmetic is performed with numbers in
 * Montgomery form, using the addition operation given by
 * http://hyperelliptic.org/EFD/g1p/auto-edwards-projective.html#addition-add-2007-bl-2
 * and the doubling operation given by
 * http://hyperelliptic.org/EFD/g1p/auto-edwards-projective.html#doubling-dbl-2007-bl-3
 */

#include "hw/openrisc/ecc.h"

#include "ul/ul_common.c"
#include "ul/ul32.c"
#include "ul/ul544_0.c"
#include "ul/ul544_1.c"
#include "ul/ul544_2.c"
#include "ul/ul576_0.c"

#include <string.h>

ec_group_t ecc_curve_e521;
int ecc_curve_e521_is_init = 0;

const ec_group_t *ec_curve_lookup(ec_curve_t curve) {
  ec_group_t *grp = NULL;

  switch (curve) {
    case ECC_CURVE_E521:
    default:
      grp = &ecc_curve_e521;
      if (0 == ecc_curve_e521_is_init) {
        /* Set our prime p = 2^521 - 1 */
        {
          ul one;
          ul_set_ui(one, 1);
          ul_set_ui(grp->p->n, 0);
          ul_setbit(521, grp->p->n);
          ul_sub(grp->p->n, grp->p->n, one);
          mod_set(grp->p, grp->p->n);
        }
        /* Set our c = 1*/
        ul_set_ui(grp->c, 1); ul_to_montgomery(grp->c, grp->c, grp->p);
        /* Set our d = -376014 */
        {
          ul zero; ul_set_ui(zero, 0); ul_to_montgomery(zero, zero, grp->p);
          ul_set_ui(grp->d, 376014);   ul_to_montgomery(grp->d, grp->d, grp->p);
          ul_modsub(grp->d, zero, grp->d, grp->p);
        }

        /* set the generator */
        {
          ul_set_fullui(grp->g->X,                    0x75, 
            0x2cb45c48, 0x648b189d, 0xf90cb229, 0x6b2878a3,
            0xbfd9f42f, 0xc6c818ec, 0x8bf3c9c0, 0xc6203913,
            0xf6ecc5cc, 0xc72434b1, 0xae949d56, 0x8fc99c60,
            0x59d0fb13, 0x364838aa, 0x302a940a, 0x2f19ba6c);
          ul_to_montgomery(grp->g->X, grp->g->X, grp->p);
          ul_set_ui(grp->g->Y, 0xc);
          ul_to_montgomery(grp->g->Y, grp->g->Y, grp->p);
          ul_set_ui(grp->g->Z, 0x1);
          ul_to_montgomery(grp->g->Z, grp->g->Z, grp->p);
        }

        /* set the group order */
        {
          ul_set_fullui(grp->n,                       0x7f,
            0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
            0xffffffff, 0xffffffff, 0xffffffff, 0xfffffffd,
            0x15b6c647, 0x46fc85f7, 0x36b8af5e, 0x7ec53f04,
            0xfbd8c456, 0x9a8f1f45, 0x40ea2435, 0xf5180d6b);
        }

        ecc_curve_e521_is_init = 1;
      }
      return grp;
  }
}

void edwards_to_affine(ul x, ul y, const edwards_point_t p, const ec_group_t *grp) {
  ul Zinv;
  ul_from_montgomery(Zinv, p->Z, grp->p);
  ul_modinv(Zinv, Zinv, grp->p->n);
  ul_to_montgomery(Zinv, Zinv, grp->p);

  ul_modmul(x, p->X, Zinv, grp->p); ul_from_montgomery(x, x, grp->p);
  ul_modmul(y, p->Y, Zinv, grp->p); ul_from_montgomery(y, y, grp->p);
}

static void edwards_point_set(edwards_point_t dst, const edwards_point_t src) {
  ul_set(dst->X, src->X);
  ul_set(dst->Y, src->Y);
  ul_set(dst->Z, src->Z);
}

static void edwards_add(edwards_point_t dst, const edwards_point_t src1, const edwards_point_t src2, const ec_group_t *grp) {
  /* R1 = X1 */
  ul R1;
  ul_set(R1, src1->X);

  /* R2 = Y1 */
  ul R2;
  ul_set(R2, src1->Y);

  /* R3 = Z1 */
  ul R3;
  ul_set(R3, src1->Z);

  /* R4 = X2 */
  ul R4;
  ul_set(R4, src2->X);

  /* R5 = Y2 */
  ul R5;
  ul_set(R5, src2->Y);

  /* R6 = Z2 */
  ul R6;
  ul_set(R6, src2->Z);

  /* R3 = R3*R6 */
  ul_modmul(R3, R3, R6, grp->p);

  /* R7 = R1+R2 */
  ul R7;
  ul_modadd(R7, R1, R2, grp->p);

  /* R8 = R4+R5 */
  ul R8;
  ul_modadd(R8, R4, R5, grp->p);

  /* R1 = R1*R4 */
  ul_modmul(R1, R1, R4, grp->p);

  /* R2 = R2*R5 */
  ul_modmul(R2, R2, R5, grp->p);

  /* R7 = R7*R8 */
  ul_modmul(R7, R7, R8, grp->p);

  /* R7 = R7-R1 */
  ul_modsub(R7, R7, R1, grp->p);

  /* R7 = R7-R2 */
  ul_modsub(R7, R7, R2, grp->p);

  /* R7 = R7*R3 */
  ul_modmul(R7, R7, R3, grp->p);

  /* R8 = R1*R2 */
  ul_modmul(R8, R1, R2, grp->p);

  /* R8 = d*R8 */
  ul_modmul(R8, grp->d, R8, grp->p);

  /* R2 = R2-R1 */
  ul_modsub(R2, R2, R1, grp->p);

  /* R2 = R2*R3 */
  ul_modmul(R2, R2, R3, grp->p);

  /* R3 = R3^2 */
  ul_modmul(R3, R3, R3, grp->p);

  /* R1 = R3-R8 */
  ul_modsub(R1, R3, R8, grp->p);

  /* R3 = R3+R8 */
  ul_modadd(R3, R3, R8, grp->p);

  /* R2 = R2*R3 */
  ul_modmul(R2, R2, R3, grp->p);

  /* R3 = R3*R1 */
  ul_modmul(R3, R3, R1, grp->p);

  /* R1 = R1*R7 */
  ul_modmul(R1, R1, R7, grp->p);

  /* R3 = c*R3 */
  ul_modmul(R3, grp->c, R3, grp->p);

  /* X3 = R1 */
  ul_set(dst->X, R1);

  /* Y3 = R2 */
  ul_set(dst->Y, R2);

  /* Z3 = R3 */
  ul_set(dst->Z, R3);
}

static void edwards_double(edwards_point_t dst, const edwards_point_t p, const ec_group_t *grp) {
  /* R1 = X1 */
  ul R1;
  ul_set(R1, p->X);

  /* R2 = Y1 */
  ul R2;
  ul_set(R2, p->Y);

  /* R3 = Z1 */
  ul R3;
  ul_set(R3, p->Z);

  /* R3 = c*R3 */
  ul_modmul(R3, grp->c, R3, grp->p);

  /* R4 = R1^2 */
  ul R4;
  ul_modmul(R4, R1, R1, grp->p);

  /* R1 = R1+R2 */
  ul_modadd(R1, R1, R2, grp->p);

  /* R1 = R1^2 */
  ul_modmul(R1, R1, R1, grp->p);

  /* R2 = R2^2 */
  ul_modmul(R2, R2, R2, grp->p);

  /* R3 = R3^2 */
  ul_modmul(R3, R3, R3, grp->p);

  /* R3 = 2*R3 */
  ul_modadd(R3, R3, R3, grp->p);

  /* R4 = R2+R4 */
  ul_modadd(R4, R2, R4, grp->p);

  /* R2 = 2*R2 */
  ul_modadd(R2, R2, R2, grp->p);

  /* R2 = R4-R2 */
  ul_modsub(R2, R4, R2, grp->p);

  /* R1 = R1-R4 */
  ul_modsub(R1, R1, R4, grp->p);

  /* R2 = R2*R4 */
  ul_modmul(R2, R2, R4, grp->p);

  /* R3 = R4-R3 */
  ul_modsub(R3, R4, R3, grp->p);

  /* R1 = R1*R3 */
  ul_modmul(R1, R1, R3, grp->p);

  /* R3 = R3*R4 */
  ul_modmul(R3, R3, R4, grp->p);

  /* R1 = c*R1 */
  ul_modmul(R1, grp->c, R1, grp->p);

  /* R2 = c*R2 */
  ul_modmul(R2, grp->c, R2, grp->p);

  /* X3 = R1 */
  ul_set(dst->X, R1);

  /* Y3 = R2 */
  ul_set(dst->Y, R2);

  /* Z3 = R3 */
  ul_set(dst->Z, R3);
}

void edwards_montgomery_ladder(edwards_point_t dst, const ul n, const edwards_point_t p, const ec_group_t *grp) {
  uint32_t l = ul_msb(n);
  edwards_point_t p1, p2;

  edwards_point_set(p1, p);
  edwards_double(p2, p1, grp);

  int32_t i = l - 2;
  while (i >= 0) {
    if (0 == ul_testbit(i, n)) {
      edwards_add(p2, p1, p2, grp);
      edwards_double(p1, p1, grp);
    }
    else {
      edwards_add(p1, p1, p2, grp);
      edwards_double(p2, p2, grp);
    }
    i--;
  }

  edwards_point_set(dst, p1);
}

// Convert a point on the elliptic curve in compressed form into an edwards_point_t
void uncompress_point(edwards_point_t dst, ul pt, uint8_t y_sign, const ec_group_t* grp)
{
    ul one; ul_set_ui(one, 1); ul_to_montgomery(one, one, grp->p);

    // Set the X and Z coordinates of the EC point
    ul_set(dst->X, pt);
    ul_set(dst->Z, one);

    // Need to compute the Y coordinate as [y^2] = = (1 - [x]^2) / (1 + 376014[x]^2)
    
    // First compute x^2
    ul x2; ul_modmul(x2, pt, pt, grp->p);

    // Compute the numerator
    ul numer; ul_modsub(numer, one, x2, grp->p);

    // Compute the denominator
    ul coeff; ul_set_ui(coeff, 376014); ul_to_montgomery(coeff, coeff, grp->p);
    ul prod; ul_modmul(prod, coeff, x2, grp->p);
    ul denom; ul_modadd(denom, one, prod, grp->p);

    // Compute the inverse of the denominator
    // The mod-inv function needs a non-montgomery-form number
    ul_from_montgomery(denom, denom, grp->p);
    ul denom_inv; ul_modinv(denom_inv, denom, grp->p->n);
    ul_to_montgomery(denom_inv, denom_inv, grp->p);

    // Compute y^2
    ul y2; ul_modmul(y2, numer, denom_inv, grp->p);

    // Find the two roots of y^2, and then pick the appropriate one based on the given sign
    ul r1; ul r2; ul_modsqrt_S1(r1, r2, y2, grp->p);
    ul unroot; ul_from_montgomery(unroot, r1, grp->p);
    if (ul_testbit(0, unroot) == y_sign)
        ul_set(dst->Y, r1);
    else ul_set(dst->Y, r2);
}

// Convert an edwards_point_t into compressed form
void compress_point(ul x, uint8_t* y_sign, edwards_point_t pt, const ec_group_t* grp)
{
    ul y;
    edwards_to_affine(x, y, pt, grp);
    *y_sign = ul_testbit(0, y);
}
