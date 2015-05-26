/** @file ed521.h
 *  
 *  This file contains declarations for using the software point-scalar multiply
 *  for the E-521 curve
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

#ifndef ED521_H_
#define ED521_H_

#include <stdint.h>

/** @addtogroup swcrypto
 *  @{
 */

/** @defgroup sw_ecc Software E-521
 *  @{
 */

/** @brief Number of 32-bit integers needed to represent a curve coordinate */
#define ED521_LIMBS ((521 + 31) / 32)

/** @brief A point on the curve */
typedef struct {
	uint32_t x[ED521_LIMBS];
	uint32_t y[ED521_LIMBS];
	uint32_t z[ED521_LIMBS];
} ec_point_t;

#if BITS_29
/** @brief Number of 32-bit integers needed to represent a curve coordinate */
#define ED521_LIMBS 18

/** @brief A point on the curve */
typedef struct {
	int32_t x[ED521_LIMBS];
	int32_t y[ED521_LIMBS];
	int32_t z[ED521_LIMBS];
} ec_point_t;
#endif

/** @brief Convert an integer from the MMIO region to the ECC library format 
 *
 *  @param[out] out Converted integer
 *  @param in Integer to convert
 *  @param size Position of last non-zero uint32_t in the representation
 */
void make_mp(uint32_t* out, uint8_t* in, unsigned size);

/** @brief Convert an integer from the ECC library format to the MMIO region format
 *
 *  @param[out] out Converted integer
 *  @param in Integer to convert
 *  @param size Position of last non-zero uint32_t in the representation
 */
void from_mp(uint8_t* out, uint32_t* in, unsigned size);

/** @brief Convert a compressed point on the curve into a normal point
 *
 *  @param[out] p The output point (uncompressed)
 *  @param x The input point (compressed)
 *  @param y_sign If 1, take the odd root, otherwise take the even root
 *
 *  @return 0 for success, -1 on error
 */
int point_uncompress(ec_point_t *p, uint32_t *x, int y_sign);


/** @brief Compress a point on the curve
 *
 *  @param[out] x The x-coordinate of the compressed point
 *  @param[out] y_sign The "sign" of the y-coordinate; if 1, take the odd root, and
 *    take the even root otherwise
 *  @param p The input point to compress
 *
 *  @return 0 for success, -1 on error
 */
int point_compress(uint32_t *x, int *y_sign, ec_point_t *p);

/** @brief Perform a point-scalar multiplcation on the curve
 *
 *  @param[out] d The result of the multiplication
 *  @param a The input point
 *  @param s The input scalar
 */
void point_scalar(ec_point_t *d, const ec_point_t *a, const uint32_t *s);

/** @} */

/** @} */

#endif /* ED521_H_ */

