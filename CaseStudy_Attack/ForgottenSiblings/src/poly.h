/*
   libit - Library for basic source and channel coding functions
   Copyright (C) 2005-2005 Vivien Chappelier, Herve Jegou

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this library; if not, write to the Free
   Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/ 

/*
  Polynomial functions
  Copyright (C) 2005 Vivien Chappelier
*/

#ifndef __it_poly_h
#define __it_poly_h

#include "types.h"
#include "vec.h"
#include "mat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* matrix of polynomials */
typedef vec ** pmat;

static inline pmat pmat_new(int W, int H) { return((pmat) Mat_new(vec, W, H)); }
static inline void pmat_delete(pmat m) { Mat_delete((Mat) m); }

/* vector of polynomials */
typedef vec * pvec;

static inline pvec pvec_new(int N) { return((pvec) Vec_new(vec, N)); }
static inline void pvec_delete(pvec v) { Vec_delete((Vec) v); }

/* polynomial function */
it_function_args(itf_polynomial)
{
  vec poly;
};
extern it_function_t itf_polynomial;

/* the degree of a polynomial */
#define poly_deg(p) (vec_length(p) - 1)

/* remove null factors from the polynomial */
void poly_normalize(vec v);

/* check if the polynomial is null. */
int poly_is_null(vec v);

/* evaluate the polynomial in 'x' */
static inline double poly_eval(vec v, double x) {
  it_function_args(itf_polynomial) itf_polynomial_args;
  itf_polynomial_args.poly = v;
  return(itf_polynomial(x, &itf_polynomial_args));
}

/*-----------------------------------------------------------------*/
/* Arithmetic operations                                           */
/*-----------------------------------------------------------------*/

void poly_shift(vec v, int shift);

/* polynomial addition */
vec poly_add(vec a, vec b);

/* polynomial subtraction */
vec poly_sub(vec a, vec b);

/* polynomial multiplication */
static inline vec poly_mul(vec a, vec b) {
  return(vec_conv(a, b));
}

/* Laurent polynomial Euclidean division of a by b */
/* This finds Q and R such that A = B Q + R X^deg_x */
/* for classical polynomial division, deg_x = 0. */
vec lpoly_ediv(vec _a, vec _b, int deg_x, vec *_q);

/* polynomial Euclidean division */
#define poly_ediv(a, b, _q) lpoly_ediv(a, b, 0, _q)

/* returns the quotient of the Euclidean division of a by b */
static inline vec poly_div(vec a, vec b) {
  vec q, r;
  r = poly_ediv(a, b, &q);
  vec_delete(r);
  return(q);
}

/* returns the remainder of the Euclidean division of a by b */
static inline vec poly_mod(vec a, vec b) {
  vec q, r;
  r = poly_ediv(a, b, &q);
  vec_delete(q);
  return(r);
}

/* returns the greatest common divider of a and b */ 
vec poly_gcd(vec a, vec b);

#ifdef __cplusplus
}
#endif /* extern "C" */
#endif











