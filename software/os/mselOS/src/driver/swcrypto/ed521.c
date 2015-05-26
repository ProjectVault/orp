/** @file ed521.c
 *
 *  This file contains a software implementation of point-scalar multiplication on the
 *  E-521 elliptic curve
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ed521.h"
#ifdef ECC_TEST
#include <unistd.h>
#include <gmp.h>
#endif

#define ED521_D -376014

const uint32_t ED521_P[ED521_LIMBS] = {
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0x000001ff
};

const uint32_t ED521_Gx[ED521_LIMBS] = {
	0x2f19ba6c,
	0x302a940a,
	0x364838aa,
	0x59d0fb13,
	0x8fc99c60,
	0xae949d56,
	0xc72434b1,
	0xf6ecc5cc,
	0xc6203913,
	0x8bf3c9c0,
	0xc6c818ec,
	0xbfd9f42f,
	0x6b2878a3,
	0xf90cb229,
	0x648b189d,
	0x2cb45c48,
	0x00000075
};

#define ED521_Gy 0xc

const uint32_t ED521_order[ED521_LIMBS] = {
	0xf5180d6b,
	0x40ea2435,
	0x9a8f1f45,
	0xfbd8c456,
	0x7ec53f04,
	0x36b8af5e,
	0x46fc85f7,
	0x15b6c647,
	0xfffffffd,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0xffffffff,
	0x0000007f
};


void make_mp(uint32_t* out, uint8_t* in, unsigned size)
{
    // Need to convert from the MMIO region to the ECC library.  The MMIO region takes a point in
    // big-endian format.  The ECC library expects 32-bit integers ordered little-endian.  So each
    // group of four bytes from the MMIO region needs to be preserved in order, but the ordering of
    // the "groups-of-four" needs to be flip-flopped.
    unsigned i;
    for (i = 0; i < ED521_LIMBS; ++i)
    {
        out[i] = 0x0;
        out[i] |= (in[size - 4*i - 4] << 24);
        out[i] |= (in[size - 4*i - 3] << 16);
        out[i] |= (in[size - 4*i - 2] <<  8);
        out[i] |= (in[size - 4*i - 1]);
    }
}

void from_mp(uint8_t* out, uint32_t* in, unsigned size)
{
    unsigned i;
    for (i = 0; i < ED521_LIMBS; ++i)
    {
        out[size - 4*i - 4] = (in[i] >> 24);
        out[size - 4*i - 3] = (in[i] >> 16);
        out[size - 4*i - 2] = (in[i] >>  8);
        out[size - 4*i - 1] = (in[i]);
    }
}

static void mp_set(uint32_t *d, const uint32_t *s)
{
	int i;

	for(i = 0; i < ED521_LIMBS; i++)
		d[i] = s[i];
	return;
}

static void mp_set_ui(uint32_t *d, const uint32_t s)
{
	int i;

	d[0] = s;
	for(i = 1; i < ED521_LIMBS; i++)
		d[i] = 0;
	return;
}

static int mp_cmp(const uint32_t *a, const uint32_t *b)
{
	int i, ret;

	ret = 0;
	for(i = 0; i < ED521_LIMBS; i++)
		ret += (a[i] != b[i]);
	return (ret == 0) ? 0 : -1;
}

static int mp_cmp_ui(const uint32_t *a, const uint32_t b)
{
	int i, ret;

	ret = (a[0] != b);
	for(i = 1; i < ED521_LIMBS; i++)
		ret += (a[i] != 0);
	return (ret == 0) ? 0 : -1;
}

static void mp_add(uint32_t *d, const uint32_t *a, const uint32_t *b)
{
	int i;
	uint64_t v;

	for(i = 0, v = 0; i < ED521_LIMBS; i++, v >>= 32)
	{
		v = v + a[i] + b[i];
		d[i] = v;
	}
	return;
}

static void mp_add_ui(uint32_t *d, const uint32_t *a, const uint32_t b)
{
	int i;
	uint64_t v;

	for(i = 0, v = b; i < ED521_LIMBS; i++, v >>= 32)
	{
		v = v + a[i];
		d[i] = v;
	}
	return;
}

static void mp_sub(uint32_t *d, const uint32_t *a, const uint32_t *b)
{
	int i;
	uint64_t v;

	for(i = 0, v = 0; i < ED521_LIMBS; i++, v >>= 32)
	{
		v = (uint64_t) a[i] - b[i] - (v & 1);
		d[i] = v;
	}
	return;
}

static void mp_sub_ui(uint32_t *d, const uint32_t *a, const uint32_t b)
{
	int i;
	uint64_t v;

	v = (uint64_t) a[0] - b;
	d[0] = v;
	for(i = 1; i < ED521_LIMBS; i++)
	{
		v >>= 32;
		v = (uint64_t) a[i] - (v & 1);
		d[i] = v;
	}
	return;
}

static void mp_modadd(uint32_t *d, const uint32_t *a, const uint32_t *b)
{
	uint32_t v;
	mp_add(d, a, b);
	v = d[ED521_LIMBS - 1] >> 9;
	d[ED521_LIMBS - 1] &= 0x1ff;
	mp_add_ui(d, d, v);
	return;
}

static void mp_modsub(uint32_t *d, const uint32_t *a, const uint32_t *b)
{
	uint32_t v;
	mp_sub(d, a, b);
	v = (d[ED521_LIMBS - 1] >> 9) & 1;
	d[ED521_LIMBS - 1] &= 0x1ff;
	mp_sub_ui(d, d, v);
	return;
}

static void mp_modmul2(uint32_t *d, const uint32_t *a)
{
	int i;
	uint32_t c, v;

	for(i = 0, c = (a[ED521_LIMBS - 1] >> 8); i < ED521_LIMBS; i++)
	{
		v = a[i];
		d[i] = (v << 1) + c;
		c = v >> 31;
	}
	d[ED521_LIMBS - 1] &= 0x1ff;
	return;
}

static void mp_modmul(uint32_t *d, const uint32_t *a, const uint32_t *b)
{
	int i, j, k;
	uint32_t tmp[ED521_LIMBS * 2];
	uint64_t v;

	for(i = 0; i < sizeof(tmp) / sizeof(tmp[0]); i++)
		tmp[i] = 0;

	for(i = 0; i < ED521_LIMBS; i++)
	{
		for(j = 0, k = i, v = 0; j < ED521_LIMBS; j++, k++)
		{
			v = (uint64_t) a[i] * b[j] + v + tmp[k];
			tmp[k] = v;
			v >>= 32;
		}
		tmp[k] = v;
	}

	for(i = 0, j = ED521_LIMBS - 1; i < ED521_LIMBS; i++, j++)
		d[i] = (tmp[j] >> 9) + (tmp[j + 1] << (32 - 9));
	tmp[ED521_LIMBS - 1] &= 0x1ff;
	mp_modadd(d, d, tmp);
	return;
}

/* find a quadratic residue of a mod p */
static void mp_modsqrt(uint32_t *r1, const uint32_t *a)
{
	int i;

	mp_set(r1, a);
	for(i = 1; i < 520; i++)
		mp_modmul(r1, r1, r1);
	return;
}

static void mp_modinv(uint32_t *d, uint32_t *a)
{
	int i;
	uint32_t t[ED521_LIMBS];

	mp_set(t, a);
	mp_set(d, a);

	mp_modmul(t, t, t);
	for(i = 2; i < 521; i++)
	{
		mp_modmul(t, t, t);
		mp_modmul(d, d, t);
	}
	return;
}

static void point_set(ec_point_t *d, const ec_point_t *a)
{
	mp_set(d->x, a->x);
	mp_set(d->y, a->y);
	mp_set(d->z, a->z);
	return;
}

static int point_make_affine(ec_point_t *d, ec_point_t *a)
{
	uint32_t t[ED521_LIMBS];

	mp_modinv(t, a->z);
	mp_modmul(d->x, a->x, t);
	mp_modmul(d->y, a->y, t);
	mp_modmul(d->z, a->z, t);
	return mp_cmp_ui(d->z, 1);
}

int point_uncompress(ec_point_t *p, uint32_t *x, int y_sign)
{
	int ret;
	uint32_t t[ED521_LIMBS];

	ret = 0;
	mp_set(p->x, x);
	mp_modmul(p->z, p->x, p->x);

	/* t = 1/((X^2 * 376014) + 1) (mod P) */
	mp_set_ui(t, 376014);
	mp_modmul(t, t, p->z);
	mp_add_ui(t, t, 1);
	mp_modinv(p->y, t);

	/* check modinv */
	mp_modmul(t, t, p->y);
	if(mp_cmp_ui(t, 1) != 0)
		ret = -1;

	/* x2 = 1 - x^2 (mod P) */
	mp_set_ui(t, 1);
	mp_modsub(t, t, p->z);

	mp_modmul(t, t, p->y);
	mp_modsqrt(p->y, t);

	/* check modsqrt */
	mp_modmul(p->z, p->y, p->y);
	if(mp_cmp(p->z, t) != 0)
		ret = -1;

	if((p->y[0] & 1) != y_sign)
		mp_sub(p->y, ED521_P, p->y);
	mp_set_ui(p->z, 1);
	return ret;
}

int point_compress(uint32_t *x, int *y_sign, ec_point_t *p)
{
	int ret;

	ret = point_make_affine(p, p);
	*y_sign = p->y[0] & 1;
	mp_set(x, p->x);
	return ret;
}

static void point_double(ec_point_t *d, ec_point_t *a)
{
	uint32_t R4[ED521_LIMBS];

	mp_set(d->x, a->x); // R1 = X1
	mp_set(d->y, a->y); // R2 = Y1
	mp_set(d->z, a->z); // R3 = Z1
	// R3 = c*R3 : c = 1 for ed521
	mp_modmul(R4, d->x, d->x); // R4 = R1^2
	mp_modadd(d->x, d->x, d->y); // R1 = R1 + R2
	mp_modmul(d->x, d->x, d->x); // R1 = R1^2
	mp_modmul(d->y, d->y, d->y); // R2 = R2^2
	mp_modmul(d->z, d->z, d->z); // R3 = R3^2
	mp_modmul2(d->z, d->z); // R3 = 2 * R3
	mp_modadd(R4, d->y, R4); // R4 = R2 + R4
	mp_modmul2(d->y, d->y); // R2 = 2 * R2
	mp_modsub(d->y, R4, d->y); // R2 = R4 - R2
	mp_modsub(d->x, d->x, R4); // R1 = R1 - R4
	mp_modmul(d->y, d->y, R4); // R2 = R2 * R4
	mp_modsub(d->z, R4, d->z); // R3 = R4 - R3
	mp_modmul(d->x, d->x, d->z); // R1 = R1 * R3
	mp_modmul(d->z, d->z, R4); // R3 = R3 * R4
	// R1 = c*R1 : c = 1 for ed521
	// R2 = c*R2 : c = 1 for ed521
	// X3 = R1 : already set
	// Y3 = R2 : already set
	// Z3 = R3 : already set
	return;
}

static void point_add(ec_point_t *d, ec_point_t *a, ec_point_t *b)
{
	int i;
	uint32_t R4[ED521_LIMBS], R5[ED521_LIMBS];
	uint32_t R7[ED521_LIMBS], R8[ED521_LIMBS];

	mp_set(R4, b->x); // R4 = X2
	mp_set(R5, b->y); // R5 = Y2
	mp_set(R7, b->z); // R7 = Z2
	mp_set(d->x, a->x); // R1 = X1
	mp_set(d->y, a->y); // R2 = Y1
	mp_set(d->z, a->z); // R3 = Z1
	mp_modmul(d->z, d->z, R7); // R3 = R3*R7
	mp_modadd(R7, d->x, d->y); // R7 = R1+R2
	mp_modadd(R8, R4, R5); // R8 = R4+R5
	mp_modmul(d->x, d->x, R4); // R1 = R1*R4
	mp_modmul(d->y, d->y, R5); // R2 = R2*R5
	mp_modmul(R7, R7, R8); // R7 = R7*R8
	mp_modsub(R7, R7, d->x); // R7 = R7-R1
	mp_modsub(R7, R7, d->y); // R7 = R7-R2
	mp_modmul(R7, R7, d->z); // R7 = R7*R3
	mp_modmul(R8, d->x, d->y); // R8 = R1*R2

	R4[0] = ED521_P[0] + ED521_D;
	for(i = 1; i < ED521_LIMBS; i++)
		R4[i] = ED521_P[i];
	mp_modmul(R8, R8, R4); // R8 = d*R8
	mp_modsub(d->y, d->y, d->x); // R2 = R2-R1
	mp_modmul(d->y, d->y, d->z); // R2 = R2*R3
	mp_modmul(d->z, d->z, d->z); // R3 = R3^2
	mp_modsub(d->x, d->z, R8); // R1 = R3-R8
	mp_modadd(d->z, d->z, R8); // R3 = R3+R8
	mp_modmul(d->y, d->y, d->z); // R2 = R2*R3
	mp_modmul(d->z, d->z, d->x); // R3 = R3*R1
	mp_modmul(d->x, d->x, R7); // R1 = R1*R7
	// R3 = c*R3 : c = 1 for ed521
	// X3 = R1 : already set
	// Y3 = R2 : already set
	// Z3 = R3 : already set
	return;
}

void point_scalar(ec_point_t *d, const ec_point_t *a, const uint32_t *s)
{
	int i, j;
	uint32_t v, b;
	ec_point_t tmp;
	ec_point_t *p[2];

	p[0] = d;
	p[1] = &tmp;

	point_set(p[1], a);
	mp_set_ui(p[0]->x, 0);
	mp_set_ui(p[0]->y, 1);
	mp_set_ui(p[0]->z, 1);

	for(i = ED521_LIMBS - 1; i >= 0; i--)
	{
		v = s[i];
		for(j = 0; j < 32; j++)
		{
			b = (v >> 31);
			point_add(p[b ^ 1], p[0], p[1]);
			point_double(p[b], p[b]);
			v <<= 1;
		}
	}
	return;
}

#ifdef ECC_TEST
static void mpz_set_mp(mpz_t d, const uint32_t *s)
{
	int i;

	mpz_set_ui(d, 0);
	for(i = ED521_LIMBS - 1; i >= 0; i--)
	{
		mpz_mul_2exp(d, d, 32);
		mpz_add_ui(d, d, s[i]);
	}
	return;
}

static void mp_set_mpz(uint32_t *d, mpz_t s)
{
	int i;
	mpz_t t;

	mpz_init_set(t, s);
	for(i = 0; i < ED521_LIMBS; i++)
	{
		d[i] = mpz_get_ui(t);
		mpz_div_2exp(t, t, 32);
	}
	mpz_clear(t);
	return;
}

int mp_test()
{
	int i, j, y_sign;
	gmp_randstate_t rnd;
	mpz_t X, Y, Z, Zmp, P, E;
	mpz_t X2, Y2, denom, numer;
	uint32_t x[ED521_LIMBS], y[ED521_LIMBS], z[ED521_LIMBS];

	gmp_randinit_default(rnd);
	gmp_randseed_ui(rnd, 43); //getpid());

	mpz_init(X);
	mpz_init(Y);
	mpz_init(Z);
	mpz_init(Zmp);
	mpz_init_set_str(P, "1ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff", 16);
	mpz_init(E);
	mpz_add_ui(E, P, 1);
	mpz_div_2exp(E, E, 2);
	mpz_init(X2);
	mpz_init(Y2);
	mpz_init(denom);
	mpz_init(numer);
/*
	for(i = 0; i < 50000; i++)
	{
		mpz_urandomm(X, rnd, P);

		mp_set_mpz(x, X);

		mpz_set_mp(Zmp, x);
		if(mpz_cmp(X, Zmp) != 0)
		{
			gmp_printf("mp convert mpz mismatch %d\nexp: %Zx\ngot: %Zx\n", i, X, Zmp);
			exit(-1);
		}
	}

	for(i = 0; i < 100000; i++)
	{
		mpz_urandomm(X, rnd, P);
		mpz_urandomm(Y, rnd, P);
		mpz_add(Z, X, Y);
		mpz_mod(Z, Z, P);

		mp_set_mpz(x, X);
		mp_set_mpz(y, Y);

		mp_modadd(z, x, y);

		mpz_set_mp(Zmp, z);

		if(mpz_cmp(Z, Zmp) != 0)
		{
			mpz_xor(X, Z, Zmp);
			gmp_printf("mp_modadd mismatch %d\nexp: %0136Zx\ngot: %0136Zx\nxor: %0136Zx\n", i, Z, Zmp, X);
			exit(-1);
		}
	}

	for(i = 0; i < 100000; i++)
	{
		mpz_urandomm(X, rnd, P);
		mpz_urandomm(Y, rnd, P);
		mpz_sub(Z, X, Y);
		mpz_mod(Z, Z, P);

		mp_set_mpz(x, X);
		mp_set_mpz(y, Y);

		mp_modsub(z, x, y);

		mpz_set_mp(Zmp, z);
		if(mpz_cmp(Z, Zmp) != 0)
		{
			gmp_printf("mp_modsub mismatch %d\nexp: %Zx\ngot: %Zx\n", i, Z, Zmp);
			exit(-1);
		}
	}

	for(i = 0; i < 100000; i++)
	{
		mpz_urandomm(X, rnd, P);
		mpz_urandomm(Y, rnd, P);
		mpz_mul_2exp(Z, X, 1);
		mpz_mod(Z, Z, P);

		mp_set_mpz(x, X);
		mp_set_mpz(y, Y);

		mp_modmul2(z, x);

		mpz_set_mp(Zmp, z);
		if(mpz_cmp(Z, Zmp) != 0)
		{
			gmp_printf("mp_modmul2 mismatch %d\nexp: %Zx\ngot: %Zx\n", i, Z, Zmp);
			mpz_mod(Zmp, Zmp, P);
			gmp_printf("rst: %Zx\n", P);
			exit(-1);
		}
	}

	for(i = 0; i < 100000; i++)
	{
		mpz_urandomm(X, rnd, P);
		mpz_urandomm(Y, rnd, P);
		mpz_mul(Z, X, Y);
		mpz_mod(Z, Z, P);

		mp_set_mpz(x, X);
		mp_set_mpz(y, Y);

		mp_modmul(z, x, y);

		mpz_set_mp(Zmp, z);
		if(mpz_cmp(Z, Zmp) != 0)
		{
			gmp_printf("mp_modmul mismatch %d\nexp: %Zx\ngot: %Zx\n", i, Z, Zmp);
			mpz_mod(Zmp, Zmp, P);
			gmp_printf("rst: %Zx\n", P);
			exit(-1);
		}
	}

	for(i = 0; i < 10000; i++)
	{
		mpz_urandomm(X, rnd, P);
		mp_set_mpz(x, X);

		mpz_invert(Y, X, P);
		mp_modinv(y, x);

		mpz_set_mp(Zmp, y);
		if(mpz_cmp(Y, Zmp) != 0)
		{
			gmp_printf("mp_modinv mismatch %d\nexp: %Zx\ngot: %Zx\n", i, Y, Zmp);
			exit(-1);
		}

	}

	for(i = 0; i < 10000; i++)
	{
		ec_point_t pt_a;

		mpz_urandomm(X, rnd, P);
		mp_set_mpz(x, X);

		mpz_mul(X2, X, X);
		mpz_mod(X2, X2, P); // X2 = X^2 (mod P)
		mpz_set_ui(numer, 1);
		mpz_sub(numer, numer, X2);
		mpz_mod(numer, numer, P); // numer = 1 - X^2 (mod P)

		mpz_mul_ui(denom, X2, 376014);
		mpz_add_ui(denom, denom, 1);
		mpz_mod(denom, denom, P);
		mpz_invert(denom, denom, P); // denom = 1/((X^2 * 376014) + 1) (mod P)

		mpz_mul(Y2, numer, denom);
		mpz_mod(Y2, Y2, P); // Y^2 = 1 - X^2 / ((X^2 * 376014) + 1) (mod P)

		mpz_powm(Y, Y2, E, P); // Y = sqrt(Y2) (mod P)
		if((mpz_get_ui(Y) & 1) != 0)
			mpz_sub(Y, P, Y);

		if(point_uncompress(&pt_a, x, 0) < 0)
			continue;

		mpz_set_mp(Zmp, pt_a.y);
		if(mpz_cmp(Y, Zmp) != 0)
		{
			gmp_printf("point_uncompress mismatch %d\nexp: %Zx\ngot: %Zx\n", i, Y, Zmp);
			exit(-1);
		}
		mp_set_ui(y, 0x34343434);
		for(j = 0; j < 8; j++)
			mp_modmul(y, y, y);

		mp_modmul(pt_a.x, pt_a.x, y);
		mp_modmul(pt_a.y, pt_a.y, y);
		mp_modmul(pt_a.z, pt_a.z, y);

		if(point_compress(z, &y_sign, &pt_a) < 0)
		{
			gmp_printf("point_compress failed\n");
			exit(-1);
		}
		if(y_sign != 0)
		{
			gmp_printf("point_compress y_sign mismatch\n");
			exit(-1);
		}
		if(mp_cmp(x, z) != 0)
		{
			gmp_printf("point_compress x mismatch\n");
			exit(-1);
		}
	}

	for(i = 0; i < 10000; i++)
	{
		ec_point_t pt_a, pt_b;

		for(j = 0; j < ED521_LIMBS; j++)
			x[j] = random();
		x[ED521_LIMBS - 1] &= 0x1ff;

		if(point_uncompress(&pt_a, x, 0) < 0)
			continue;

		mp_set_ui(pt_b.x, 0);
		mp_set_ui(pt_b.y, 1);
		mp_set_ui(pt_b.z, 1);

		point_add(&pt_b, &pt_a, &pt_b);
		point_make_affine(&pt_b, &pt_b);

		if(mp_cmp(pt_a.x, pt_b.x) != 0)
		{
			gmp_printf("x mismatch\n");
			exit(-1);
		}
		if(mp_cmp(pt_a.y, pt_b.y) != 0)
		{
			gmp_printf("y mismatch\n");
			exit(-1);
		}
		if(mp_cmp(pt_a.z, pt_b.z) != 0)
		{
			gmp_printf("z mismatch\n");
			exit(-1);
		}
	}

	for(i = 0; i < 10000; i++)
	{
		ec_point_t pt_a, pt_b, pt_c;

		for(j = 0; j < ED521_LIMBS; j++)
			x[j] = random();

		x[ED521_LIMBS - 1] &= 0x1ff;
		if(point_uncompress(&pt_a, x, 0) < 0)
			continue;

		point_add(&pt_b, &pt_a, &pt_a);
		point_make_affine(&pt_b, &pt_b);
		point_double(&pt_c, &pt_a);
		point_make_affine(&pt_c, &pt_c);

		if(mp_cmp(pt_b.x, pt_c.x) != 0)
		{
			gmp_printf("x mismatch\n");
			exit(-1);
		}
		if(mp_cmp(pt_b.y, pt_c.y) != 0)
		{
			gmp_printf("y mismatch\n");
			exit(-1);
		}
		if(mp_cmp(pt_b.z, pt_c.z) != 0)
		{
			gmp_printf("z mismatch\n");
			exit(-1);
		}
	}
*/
	for(i = 0; i < 100; i++)
	{
		ec_point_t pt_a, pt_b, pt_c;

		for(j = 0; j < ED521_LIMBS; j++)
			x[j] = random();
		x[ED521_LIMBS - 1] &= 0x1ff;
		if(point_uncompress(&pt_a, x, 0) < 0)
			continue;
		for(j = 0; j < ED521_LIMBS; j++)
			y[j] = random();
		y[ED521_LIMBS - 1] &= 0x1ff;
		for(j = 0; j < ED521_LIMBS; j++)
			z[j] = random();
		z[ED521_LIMBS - 1] &= 0x1ff;

		point_scalar(&pt_b, &pt_a, y);
		point_scalar(&pt_b, &pt_b, z);
		point_scalar(&pt_c, &pt_a, z);
		point_scalar(&pt_c, &pt_c, y);

		point_make_affine(&pt_b, &pt_b);
		point_make_affine(&pt_c, &pt_c);

		if(mp_cmp(pt_b.x, pt_c.x) != 0)
		{
			gmp_printf("x mismatch\n");
			exit(-1);
		}
		if(mp_cmp(pt_b.y, pt_c.y) != 0)
		{
			gmp_printf("y mismatch\n");
			exit(-1);
		}
		if(mp_cmp(pt_b.z, pt_c.z) != 0)
		{
			gmp_printf("z mismatch\n");
			exit(-1);
		}
	}

/*
	{
		ec_point_t pt_a, pt_b;

		mp_set(x, ED521_Gx);
		if(point_uncompress(&pt_a, x, 0) < 0)
		{
			printf("uncompress of generator failed\n");
			exit(-1);
		}
		if(mp_cmp_ui(pt_a.y, ED521_Gy) != 0)
		{
			printf("generator y incorrect\n");
			exit(-1);
		}
		point_scalar(&pt_b, &pt_a, ED521_order);
		point_make_affine(&pt_b, &pt_b);

		if(mp_cmp_ui(pt_b.x, 0) != 0)
		{
			gmp_printf("x mismatch\n");
			exit(-1);
		}
		if(mp_cmp_ui(pt_b.y, 1) != 0)
		{
			gmp_printf("y mismatch\n");
			exit(-1);
		}

	}
*/
	printf("OUT\n");
	gmp_randclear(rnd);
	return 0;
}
#endif

