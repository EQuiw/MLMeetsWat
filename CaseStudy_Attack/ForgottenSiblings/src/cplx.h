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
  Complex type
  Copyright (C) 2005 Vivien Chappelier
*/


#ifndef __it_cplx_h
#define __it_cplx_h

#include "types.h"
#include <math.h>
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*               Complex type                                                */
/*---------------------------------------------------------------------------*/

typedef struct _cplx_ {
  double r; /* real part      */
  double i; /* imaginary part */
} cplx;

#define cplx(r, i) { r, i }
#define creal(c) ((c).r)
#define cimag(c) ((c).i)
#define __it_abs(x) (((x)>=0)?(x):(-x))

static inline cplx cadd(cplx a, cplx b)
{
  cplx r;
  r.r = a.r + b.r;
  r.i = a.i + b.i;
  return(r);
}


static inline cplx csub(cplx a, cplx b)
{
  cplx r;
  r.r = a.r - b.r;
  r.i = a.i - b.i;
  return(r);
}


static inline cplx cmul(cplx a, cplx b)
{
  cplx r;
  r.r = a.r * b.r - a.i * b.i;
  r.i = a.r * b.i + a.i * b.r;

  return(r);
}


static inline cplx cscale(cplx a, double b)
{
  cplx r;
  r.r = a.r * b;
  r.i = a.i * b;

  return(r);
}


static inline cplx cdiv(cplx a, cplx b)
{
  cplx r;
  double q, n;

  assert(b.r != 0 || b.i != 0);

  if(__it_abs(b.r) < __it_abs(b.i)) {
    q = b.r / b.i;
    n = b.r * q + b.i;
    r.r = (a.r * q + a.i) / n;
    r.i = (a.i * q - a.r) / n;
  } else {
    q = b.i / b.r;
    n = b.i * q + b.r;
    r.r = (a.r + a.i * q) / n;
    r.i = (a.i - a.r * q) / n;
  }

  return(r);
}


static inline cplx cconj(cplx a)
{
  cplx r;
  r.r =  a.r;
  r.i = -a.i;
  return( r );
}


static inline cplx cneg( cplx a )
{
  cplx r;
  r.r = -a.r;
  r.i = -a.i;
  return( r );
}


static inline cplx cinv(cplx a)
{
  cplx r;
  double q, n;

  assert(a.r != 0 || a.i != 0);

  if(__it_abs(a.r) < __it_abs(a.i)) {
    q = a.r / a.i;
    n = a.r * q + a.i;
    r.r =    q / n;
    r.i = -1.0 / n;
  } else {
    q = a.i / a.r;
    n = a.i * q + a.r;
    r.r = 1.0 / n;
    r.i =  -q / n;
  }

  return(r);
}

static inline int ceq( cplx a, cplx b )
{
  return( a.r == b.r && a.i == b.i );
}


static inline double cnorm( cplx a )
{
  double q;

  a.r = __it_abs(a.r);
  a.i = __it_abs(a.i);

  if(!(a.r + a.i)) return(0);

  if(a.r < a.i) {
    q = a.i / a.r;
    return(a.r * sqrt(1 + q * q));
  } else {
    q = a.r / a.i;
    return(a.i * sqrt(1 + q * q));
  }
}

/* some constants */
extern cplx const cplx_0;
extern cplx const cplx_1;
extern cplx const cplx_I;
#define cplx_zero cplx_0
#define cplx_one cplx_1

#ifdef __cplusplus
}
#endif /* extern "C" */
#endif

