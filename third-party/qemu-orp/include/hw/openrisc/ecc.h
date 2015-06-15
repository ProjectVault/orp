/** @file ecc.h
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
 */

#ifndef _ECC_H_
#define _ECC_H_

#include <stdint.h>
#include "ul/ul544.h"

/** @ingroup crypto
 *  @{
 */

/** @defgroup ecc Elliptic curve routines
 *  @{
 */


/** @brief A point on an Edwards curve, in montgomery form. */
typedef struct edwards_point_s {
  ul X;
  ul Y;
  ul Z;
} edwards_point_t[1];

/** @brief An Edwards curve. */
typedef struct ec_group_s {
  mod p;
  ul c;
  ul d;
  edwards_point_t g;
  ul n;
} ec_group_t;

/** @brief A named curve (such as curve E-521). */
typedef uint32_t ec_curve_t;

/** @brief Curve E-521. */
#define ECC_CURVE_E521    ((ec_curve_t) 0x0000)

/** @brief An ECC key over a given group. */
typedef struct ec_key_s {
  const ec_group_t *grp;
  ul priv;
  edwards_point_t pub;
} ec_key_t[1];


/** @brief Fetch a named curve (such as curve E-521). */
const ec_group_t *ec_curve_lookup(ec_curve_t curve);

/** @brief Convert a point from Edwards form into affine coordinates. */
void edwards_to_affine(ul x, ul y, const edwards_point_t p, const ec_group_t *grp);

/** @brief Perform a scalar multiplication using the Montgomery ladder method. */
void edwards_montgomery_ladder(edwards_point_t dst, const ul n, const edwards_point_t p, const ec_group_t *grp);

/** @brief Convert a compressed point on the curve to an uncompressed point */
void uncompress_point(edwards_point_t dst, ul pt, uint8_t y_sign, const ec_group_t* grp);
void compress_point(ul x, uint8_t* y_sign, edwards_point_t pt, const ec_group_t* grp);

/** @} */

/** @} */

#endif /* _ECC_H_ */
