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
  Vectors
  Copyright (C) 2005 Vivien Chappelier, Herve Jegou
*/


#include "math.h"
#include "vec.h"
#include "io.h"
#include "random.h"

/*---------------------------------------------------------------------------*/
/*                Constant vectors                                           */
/*---------------------------------------------------------------------------*/

/* empty vectors */
static Vec_header_t __vec_null = { 
  0,               /* effective length of the vector (<= max_length)     */
  0,               /* amount of memory allocated for the vector elements */
  NULL,            /* memory block associated to this vector             */
  sizeof(double), /* size of the stored elements                        */
};
static Vec_header_t __ivec_null = { 
  0,               /* effective length of the vector (<= max_length)     */
  0,               /* amount of memory allocated for the vector elements */
  NULL,            /* memory block associated to this vector             */
  sizeof(int),     /* size of the stored elements                        */
};
static Vec_header_t __bvec_null = { 
  0,               /* effective length of the vector (<= max_length)     */
  0,               /* amount of memory allocated for the vector elements */
  NULL,            /* memory block associated to this vector             */
  sizeof(byte),    /* size of the stored elements                        */
};
static Vec_header_t __cvec_null = { 
  0,               /* effective length of the vector (<= max_length)     */
  0,               /* amount of memory allocated for the vector elements */
  NULL,            /* memory block associated to this vector             */
  sizeof(cplx),    /* size of the stored elements                        */
};


/* empty vectors for all types (+ 1 is to align at the end of the struct
   on the first element which is non-existent).                         */
vec const vec_null = (vec) (&__vec_null + 1);
ivec const ivec_null = (ivec) (&__ivec_null + 1);
bvec const bvec_null = (bvec) (&__bvec_null + 1);
cvec const cvec_null = (cvec) (&__cvec_null + 1);

/*---------------------------------------------------------------------------*/
/*                Vector allocation functions                                */
/*---------------------------------------------------------------------------*/

void *__Vec_new_alloc(size_t elem_size, idx_t length, idx_t length_max) 
{
  Vec_header_t *hdr;
  char *ptr, *aligned;
  int padding;

  /* allocate a vector of size 'length_max' with some extra room 
     for the header and padding                                  */
  ptr = (char *) malloc(sizeof(Vec_header_t) + length_max * elem_size + IT_ALLOC_ALIGN);
  it_assert( ptr, "No enough memory to allocate the vector" );

  /* make sure the first element is properly aligned */
  aligned = ptr + sizeof(Vec_header_t) + IT_ALLOC_ALIGN - 1;
  padding = ((int) (long) (aligned)) & (IT_ALLOC_ALIGN - 1);
  aligned -= padding;
  /* put the header before the first element and fill it */
  hdr = (Vec_header_t *) aligned - 1;
  hdr->length = length;
  hdr->length_max = length_max;
  hdr->ptr = ptr;
  hdr->element_size = elem_size;
  return(aligned);
}


/* We basically had two choices to have a properly aligned realloc.
   One is to call realloc (which does a memory copy if the new block cannot
   fit at the location of the old one). In this case the returned memory block
   may not be aligned properly and another copy is required to ensure proper
   alignment. If the block could fit, no memory copy is required.
   The second method is to free the memory systematically and call a new malloc.
   Obviously this method is slower if the resized block could fit at the
   location of the old one, since it always requires a memory copy.
   However, due to the geometric reallocation procedure used throughout the
   code, a realloc has very high chances of returning a different pointer. 
   This has been verified in practice. 
   Therefore, we decided to use the free/malloc method.
*/
void *__Vec_new_realloc(void *V, size_t elem_size, idx_t length, idx_t length_max) 
{
  void *new_Vec;
  idx_t old_length;

  new_Vec = __Vec_new_alloc(elem_size, length, length_max);
  assert(new_Vec);
  if(V) {
    old_length = Vec_length(V);
    memcpy(new_Vec, V, old_length * elem_size);
    Vec_delete(V);
  }
  return (new_Vec);
}


/*------------------------------------------------------------------------------*/
/*                Copy and Conversions Functions                                */
/*------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------*/
void vec_copy_mem( double * buf, vec v ) 
{
  idx_t i;
  assert( v );
  assert( buf );
  for( i = 0 ; i < vec_length( v ) ; i++ )
    buf[ i ] = v[ i ];
}


/*------------------------------------------------------------------------------*/
void ivec_copy_mem( int * buf, ivec v ) 
{
  idx_t i;
  assert( v );
  assert( buf );
  for( i = 0 ; i < ivec_length( v ) ; i++ )
    buf[ i ] = v[ i ];
}


/*------------------------------------------------------------------------------*/
void bvec_copy_mem( byte * buf, bvec v ) 
{
  idx_t i;
  assert( v );
  assert( buf );
  for( i = 0 ; i < bvec_length( v ) ; i++ )
    buf[ i ] = v[ i ];
}


/*--------------------------------------------------------------------*/
void cvec_copy_mem( cplx * buf,  cvec v ) 
{
  idx_t i;
  assert( v );
  assert( buf );
  for( i = 0 ; i < cvec_length( v ) ; i++ )
    buf[ i ] = v[ i ];
}

/*--------------------------------------------------------------------*/
void bvec_pack( byte * buf, bvec v )
{
  idx_t i, j;
  idx_t l = bvec_length( v );
  byte t;
  assert( v );
  assert( buf );
  for( i = 0 ; i < l / 8; i++ ) {
    t = 0;
    for( j = 0; j < 8; j++ )
      t |= v[8 * i + j] << (7 - j);
    buf[i] = t;
  }
  t = 0;
  for( j = 0; j < l % 8; j++ )
    t |= v[8 * i + j] << (7 - j);
  if( l % 8 )
    buf[i] = t;
}

void bvec_unpack( bvec v, byte * buf )
{
  idx_t i, j;
  idx_t l = bvec_length( v );
  byte t = 0;
  assert( v );
  assert( buf );
  for( i = 0 ; i < l / 8; i++ ) {
    t = buf[i];
    for( j = 0; j < 8; j++ )
      v[8 * i + j] = (t >> (7 - j)) & 1;
  }
  if( l % 8 )
    t = buf[i];

  for( j = 0; j < l % 8; j++ )
    v[8 * i + j] = (t >> (7 - j)) & 1;
}

/*--------------------------------------------------------------------*/
vec ivec_to_vec( ivec v )
{
  idx_t i;
  idx_t l = ivec_length(v);
  vec r = vec_new(l);

  for( i = 0; i < l ; i++ )
    r[ i ] = (double) v[ i ];

  return(r);
}


bvec ivec_to_bvec( ivec v )
{
  idx_t i;
  idx_t l = ivec_length( v );
  bvec r = bvec_new( l );

  for( i = 0; i < l ; i++ )
    r[ i ] = (byte) v[ i ];

  return( r );
}

cvec ivec_to_cvec( ivec v )
{
  idx_t i;
  idx_t l = ivec_length( v );
  cvec r = cvec_new( l );

  for( i = 0; i < l ; i++ ) {
    creal(r[ i ]) = v[ i ];
    cimag(r[ i ]) = 0;
  }

  return( r );
}


/*--------------------------------------------------------------------*/
ivec bvec_to_ivec( bvec v )
{
  idx_t i;
  idx_t l = bvec_length( v );
  ivec r = ivec_new( l );

  for( i = 0 ; i < l ; i++ )
    r[i] = (int) v[ i ];

  return(r);
}


vec bvec_to_vec( bvec v )
{
  idx_t i;
  idx_t l = bvec_length( v );
  vec r = vec_new( l );

  for(i = 0; i < l; i++)
    r[ i ] = (double) v[ i ];

  return( r );
}

cvec bvec_to_cvec( bvec v )
{
  idx_t i;
  idx_t l = bvec_length( v );
  cvec r = cvec_new( l );

  for( i = 0; i < l ; i++ ) {
    creal(r[ i ]) = v[ i ];
    cimag(r[ i ]) = 0;
  }

  return( r );
}

/*--------------------------------------------------------------------*/
ivec vec_to_ivec( vec v )
{
  idx_t i;
  idx_t l = vec_length( v );
  ivec r = ivec_new( l );

  for( i = 0 ; i < l ; i++ )
    r[i] = (int) v[ i ];

  return(r);
}


bvec vec_to_bvec( vec v )
{
  idx_t i;
  idx_t l = vec_length( v );
  bvec r = bvec_new( l );

  for(i = 0; i < l; i++)
    r[ i ] = (byte) v[ i ];

  return( r );
}

cvec vec_to_cvec( vec v )
{
  idx_t i;
  idx_t l = vec_length( v );
  cvec r = cvec_new( l );

  for( i = 0; i < l ; i++ ) {
    creal(r[ i ]) = v[ i ];
    cimag(r[ i ]) = 0;
  }

  return( r );
}

/*--------------------------------------------------------------------*/
/*                Comparisons functions                               */
/*--------------------------------------------------------------------*/

int vec_eq( vec v1, vec v2 ) 
{
  idx_t i;
  assert( v1 );
  assert( v2 );

  if( vec_length( v1 ) != vec_length( v2 ) )
    return 0;

  for( i = 0 ; i < vec_length( v1 ) ; i++ )
    if( v1[ i ] != v2[ i ] )
      return 0;
  return 1;
}


/*-----------------------------------------------------------------*/
int ivec_eq( ivec v1, ivec v2 ) 
{
  idx_t i;
  assert( v1 );
  assert( v2 );

  if( ivec_length( v1 ) != ivec_length( v2 ) )
    return 0;

  for( i = 0 ; i < ivec_length( v1 ) ; i++ )
    if( v1[ i ] != v2[ i ] )
      return 0;
  return 1;
}


/*-----------------------------------------------------------------*/
int bvec_eq( bvec v1, bvec v2 ) 
{
  idx_t i;
  assert( v1 );
  assert( v2 );

  if( bvec_length( v1 ) != bvec_length( v2 ) )
    return 0;

  for( i = 0 ; i < bvec_length( v1 ) ; i++ )
    if( v1[ i ] != v2[ i ] )
      return 0;
  return 1;
}


/*-----------------------------------------------------------------*/
int cvec_eq( cvec v1, cvec v2 ) 
{
  idx_t i;
  assert( v1 );
  assert( v2 );

  if( cvec_length( v1 ) != cvec_length( v2 ) )
    return 0;

  for( i = 0 ; i < cvec_length( v1 ) ; i++ )
    if( ! ceq(v1[ i ], v2[ i ]) )
      return 0;
  return 1;
}


/*-----------------------------------------------------------------*/
int vec_geq( vec v1, vec v2 ) 
{
  idx_t i, minl;
  assert( v1 );
  assert( v2 );
  minl = ( vec_length( v1 ) > vec_length( v2 ) ? vec_length( v2 ) : vec_length( v1 ) );

  for( i = 0 ; i < minl ; i++ )
    if( v1[ i ] > v2[ i ] )
      return 0;

  if( vec_length( v1 ) >= vec_length( v2 ) )
    return 1;
  else
    return 0;
}


/*-----------------------------------------------------------------*/
int ivec_geq( ivec v1, ivec v2 ) 
{
  idx_t i, minl;
  assert( v1 );
  assert( v2 );
  minl = ( ivec_length( v1 ) > ivec_length( v2 ) ? ivec_length( v2 ) : ivec_length( v1 ) );

  for( i = 0 ; i < minl ; i++ )
    if( v1[ i ] > v2[ i ] )
      return 0;

  if( ivec_length( v1 ) >= ivec_length( v2 ) )
    return 1;
  else
    return 0;
}


/*------------------------------------------------------------------------------*/
int bvec_geq( bvec v1, bvec v2 ) 
{
  idx_t i, minl;
  assert( v1 );
  assert( v2 );
  minl = ( bvec_length( v1 ) > bvec_length( v2 ) ? bvec_length( v2 ) : bvec_length( v1 ) );

  for( i = 0 ; i < minl ; i++ )
    if( v1[ i ] > v2[ i ] )
      return 0;

  if( bvec_length( v1 ) >= bvec_length( v2 ) )
    return 1;
  else
    return 0;
}




/*------------------------------------------------------------------------------*/
/*                Arithmetic functions                                          */
/*------------------------------------------------------------------------------*/


/* Operations with a scalar value                                               */
void vec_incr( vec v, double a ) 
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < vec_length( v ) ; i++ )
    v[ i ] += a;
}


void vec_decr( vec v, double a )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < vec_length( v ) ; i++ )
    v[ i ] -= a;
}


void vec_mul_by( vec v, double a )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < vec_length( v ) ; i++ )
    v[ i ] *= a;
}


void vec_div_by( vec v, double a )  
{
  idx_t i;
  assert( v );
  assert( a );
  for( i = 0 ; i < vec_length( v ) ; i++ )
    v[ i ] /= a;
}


/* Operations with a scalar value                                               */
void ivec_incr( ivec v, int a ) 
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < ivec_length( v ) ; i++ )
    v[ i ] += a;
}


void ivec_decr( ivec v, int a )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < ivec_length( v ) ; i++ )
    v[ i ] -= a;
}


void ivec_mul_by( ivec v, int a )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < ivec_length( v ) ; i++ )
    v[ i ] *= a;
}


void ivec_div_by( ivec v, int a )  
{
  idx_t i;
  assert( v );
  assert( a );
  for( i = 0 ; i < ivec_length( v ) ; i++ )
    v[ i ] /= a;
}


/* Operations with a scalar value                                               */
void cvec_incr_real( cvec v, double a ) 
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < cvec_length( v ) ; i++ )
    creal(v[ i ]) += a;
}


void cvec_decr_real( cvec v, double a )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < cvec_length( v ) ; i++ )
    creal(v[ i ]) -= a;
}


void cvec_mul_by_real( cvec v, double a )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < cvec_length( v ) ; i++ ) {
    v[ i ].r *= a;
    v[ i ].i *= a;
  }
}


void cvec_div_by_real( cvec v, double a )  
{
  idx_t i;
  assert( v );
  assert( a );
  a = 1 / a;
  for( i = 0 ; i < cvec_length( v ) ; i++ ) {
    v[ i ].r *= a;
    v[ i ].i *= a;
  }
}


void cvec_conj( cvec v )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < cvec_length( v ) ; i++ )
    cconj(v[i]);
}

void cvec_incr( cvec v, cplx a ) 
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < cvec_length( v ) ; i++ )
    v[ i ] = cadd( v[ i ], a );
}


void cvec_decr( cvec v, cplx a )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < cvec_length( v ) ; i++ )
    v[ i ] = csub( v[ i ], a );
}


void cvec_mul_by( cvec v, cplx a )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < cvec_length( v ) ; i++ )
    v [ i ] = cmul( v[ i ], a );
}


void cvec_div_by( cvec v, cplx a )  
{
  idx_t i;
  assert( v );
  assert( !ceq(a, cplx_0) );
  a = cinv( a );
  for( i = 0 ; i < cvec_length( v ) ; i++ )
    v[ i ] = cmul( v[ i ], a );
}


/*------------------------------------------------------------------------------*/
/* Components per components operations (vectors must be of same size)    */
void vec_add( vec v1, vec v2 )  
{
  idx_t i;
  assert( v1 );
  assert( v2 );
  assert( vec_length( v1 ) == vec_length( v2 ) );
  for( i = 0 ; i < vec_length( v1 ) ; i++ )
    v1[ i ] += v2[ i ];
}


void vec_sub( vec v1, vec v2 )  
{
  idx_t i;
  assert( v1 );
  assert( v2 );
  assert( vec_length( v1 ) == vec_length( v2 ) );
  for( i = 0 ; i < vec_length( v1 ) ; i++ )
    v1[ i ] -= v2[ i ];
}


void vec_mul( vec v1, vec v2 )  
{
  idx_t i;
  assert( v1 );
  assert( v2 );
  assert( vec_length( v1 ) == vec_length( v2 ) );
  for( i = 0 ; i < vec_length( v1 ) ; i++ )
    v1[ i ] *= v2[ i ];
}


void vec_div( vec v1, vec v2 )  
{
  idx_t i;
  assert( v1 );
  assert( v2 );
  assert( vec_length( v1 ) == vec_length( v2 ) );
  for( i = 0 ; i < vec_length( v1 ) ; i++ )
    v1[ i ] /= v2[ i ];
}


void ivec_add( ivec v1, ivec v2 )  
{
  idx_t i;
  assert( v1 );
  assert( v2 );
  assert( ivec_length( v1 ) == ivec_length( v2 ) );
  for( i = 0 ; i < ivec_length( v1 ) ; i++ )
    v1[ i ] += v2[ i ];
}


void ivec_sub( ivec v1, ivec v2 )  
{
  idx_t i;
  assert( v1 );
  assert( v2 );
  assert( ivec_length( v1 ) == ivec_length( v2 ) );
  for( i = 0 ; i < ivec_length( v1 ) ; i++ )
    v1[ i ] -= v2[ i ];
}


void ivec_mul( ivec v1, ivec v2 )  
{
  idx_t i;
  assert( v1 );
  assert( v2 );
  assert( ivec_length( v1 ) == ivec_length( v2 ) );
  for( i = 0 ; i < ivec_length( v1 ) ; i++ )
    v1[ i ] *= v2[ i ];
}


void ivec_div( ivec v1, ivec v2 )  
{
  idx_t i;
  assert( v1 );
  assert( v2 );
  assert( ivec_length( v1 ) == ivec_length( v2 ) );
  for( i = 0 ; i < ivec_length( v1 ) ; i++ )
    v1[ i ] /= v2[ i ];
}


void cvec_add( cvec v1, cvec v2 )  
{
  idx_t i;
  assert( v1 );
  assert( v2 );
  assert( cvec_length( v1 ) == cvec_length( v2 ) );
  for( i = 0 ; i < cvec_length( v1 ) ; i++ )
    v1[ i ] = cadd( v1[ i ], v2[ i ] );
}


void cvec_sub( cvec v1, cvec v2 )  
{
  idx_t i;
  assert( v1 );
  assert( v2 );
  assert( cvec_length( v1 ) == cvec_length( v2 ) );
  for( i = 0 ; i < cvec_length( v1 ) ; i++ )
    v1[ i ] = csub( v1[ i ], v2[ i ] );
}


void cvec_mul( cvec v1, cvec v2 )  
{
  idx_t i;
  assert( v1 );
  assert( v2 );
  assert( cvec_length( v1 ) == cvec_length( v2 ) );
  for( i = 0 ; i < cvec_length( v1 ) ; i++ )
    v1[ i ] = cmul( v1[ i ], v2[ i ] );
}


void cvec_div( cvec v1, cvec v2 )  
{
  idx_t i;
  assert( v1 );
  assert( v2 );
  assert( cvec_length( v1 ) == cvec_length( v2 ) );
  for( i = 0 ; i < cvec_length( v1 ) ; i++ )
    v1[ i ] = cdiv( v1[ i ], v2[ i ] );
}


/*------------------------------------------------------------------------------*/
vec  vec_new_add( vec v1, vec v2 )
{
  vec r = vec_clone( v1 );
  vec_add( r, v2 );
  return r;
}


vec  vec_new_sub( vec v1, vec v2 )
{
  vec r = vec_clone( v1 );
  vec_sub( r, v2 );
  return r;
}


vec  vec_new_mul( vec v1, vec v2 )
{
  vec r = vec_clone( v1 );
  vec_mul( r, v2 );
  return r;
}


vec  vec_new_div( vec v1, vec v2 )
{
  vec r = vec_clone( v1 );
  vec_div( r, v2 );
  return r;
}


ivec ivec_new_add( ivec v1, ivec v2 )
{
  ivec r = ivec_clone( v1 );
  ivec_add( r, v2 );
  return r;
}


ivec ivec_new_sub( ivec v1, ivec v2 )
{
  ivec r = ivec_clone( v1 );
  ivec_sub( r, v2 );
  return r;
}


ivec ivec_new_mul( ivec v1, ivec v2 )
{
  ivec r = ivec_clone( v1 );
  ivec_mul( r, v2 );
  return r;
}


ivec ivec_new_div( ivec v1, ivec v2 )
{
  ivec r = ivec_clone( v1 );
  ivec_div( r, v2 );
  return r;
}


/*------------------------------------------------------------------------------*/
double vec_inner_product( vec v1, vec v2 )  
{
  double p = 0;
  idx_t i;
  assert( v1 );
  assert( v2 );
  assert( vec_length( v1 ) == vec_length( v2 ) );
  for( i = 0 ; i < vec_length( v1 ) ; i++ )
    p += v1[ i ] * v2[ i ];
  return p;
}


/*------------------------------------------------------------------------------*/
int ivec_inner_product( ivec v1, ivec v2 )  
{
  int p = 0;
  idx_t i;
  assert( v1 );
  assert( v2 );
  assert( ivec_length( v1 ) == ivec_length( v2 ) );
  for( i = 0 ; i < ivec_length( v1 ) ; i++ )
    p += v1[ i ] * v2[ i ];
  return p;
}


/*------------------------------------------------------------------------------*/
void vec_neg( vec v )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < vec_length( v ) ; i++ )
    v[ i ] = -v[ i ];
}


void ivec_neg( ivec v )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < ivec_length( v ) ; i++ )
    v[ i ] = -v[ i ];
}


void cvec_neg( cvec v )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < cvec_length( v ) ; i++ )
    v[ i ] = cneg( v[ i ] );
}


void vec_sqr( vec v )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < vec_length( v ) ; i++ )
    v[ i ] *= v[ i ];
}


void ivec_sqr( ivec v )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < ivec_length( v ) ; i++ )
    v[ i ] *= v[ i ];
}


void vec_sqrt( vec v )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < Vec_length( v ) ; i++ )
    v[ i ] = sqrt( v[ i ] );
}


void vec_log( vec v )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < Vec_length( v ) ; i++ )
    v[ i ] = log( v[ i ] );
}


void vec_log10( vec v )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < Vec_length( v ) ; i++ )
    v[ i ] = log10( v[ i ] );
}


void vec_exp( vec v )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < Vec_length( v ) ; i++ )
    v[ i ] = exp( v[ i ] );
}


void vec_abs( vec v )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < vec_length( v ) ; i++ )
    if( v[ i ] < 0 )
      v[ i ] = -( v[ i ] );
}


void ivec_abs( ivec v )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < ivec_length( v ) ; i++ )
    if( v[ i ] < 0 )
      v[ i ] = -( v[ i ] );
}


vec cvec_new_abs( cvec v )  
{
  idx_t i;
  vec va;
  assert( v );
  va = vec_new( cvec_length( v ) );
  for( i = 0 ; i < cvec_length( v ) ; i++ )
    va[ i ] = cnorm( v[ i ] );
  return va;
}


void vec_pow( vec v, double a )  
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < Vec_length( v ) ; i++ )
    if( v[ i ] < 0 )
      v[ i ] = pow( v[ i ], a );
}


void vec_normalize( vec v, double nr )  
{
  idx_t i;
  double s = 0;
  assert( v );

  /* For optimization purpose, the norm 1 is treated separately */
  if( nr == 1 )
    s = vec_sum( v );
  else {
    for( i = 0 ; i < vec_length( v ) ; i++ )
      s += pow( fabs( v[ i ] ), nr );
    s = pow( s, 1.0 / nr );
  }

  for( i = 0 ; i < vec_length( v ) ; i++ )
    v[ i ] /= s;
}


int ivec_min( ivec v ) 
{
  idx_t i;
  int m = INT_MAX;
  assert( v );
  for( i = 0 ; i < Vec_length( v ) ; i++ )
    if( v[ i ] < m )
      m = v[ i ];
  return m;  
}


int ivec_max( ivec v ) 
{
  idx_t i;
  int m = INT_MIN;
  assert( v );
  for( i = 0 ; i < Vec_length( v ) ; i++ )
    if( v[ i ] > m )
      m = v[ i ];
  return m;  
}

idx_t ivec_min_index( ivec v ) 
{
  idx_t i, mi = NULL_INDEX;
  int m = INT_MAX;
  assert( v );
  for( i = 0 ; i < ivec_length( v ) ; i++ )
    if( v[ i ] < m ) {
      m = v[ i ];
      mi = i;
    }
  return mi;  
}


idx_t ivec_max_index( ivec v ) 
{
  idx_t i, mi = NULL_INDEX;
  int m = INT_MIN;
  assert( v );
  for( i = 0 ; i < ivec_length( v ) ; i++ )
    if( v[ i ] > m ) {
      m = v[ i ];
      mi = i;
    }
  return mi;  
}


double ivec_mean( ivec v ) 
{
  assert( v );
  return ivec_sum( v ) / (double) ivec_length( v );
}


int ivec_median( ivec v )
{
  ivec c;
  int m;
  assert( v );
  if( ivec_length( v ) == 0 ) {
    it_warning( "Undefined median value for vector of size 0. Return 0.\n" );
    return 0;
  }
  c = ivec_clone( v );
  ivec_sort( c );

  if( ivec_length( v ) & 1 )
    m = c[ ( ivec_length( v ) - 1 ) / 2 ];
  else
    m = ( c[ ivec_length( v ) / 2 ] + c[ ivec_length( v ) / 2 + 1 ] ) / 2;

  ivec_delete( c );
  return m;
}


/*------------------------------------------------------------------------------*/
double vec_sum( vec v )  
{
  idx_t i;
  double s = 0;
  assert( v );
  for( i = 0 ; i < Vec_length( v ) ; i++ )
    s += v[ i ];
  return s;
}


int ivec_sum( ivec v )  
{
  idx_t i;
  int s = 0;
  assert( v );
  for( i = 0 ; i < ivec_length( v ) ; i++ ) {
    if(v[i] < 0 && s < INT_MIN - v[i]) it_warning("underflow in ivec_sum");
    if(v[i] > 0 && s > INT_MAX - v[i]) it_warning("overflow in ivec_sum");
    s += v[ i ];
  }
  return s;
}


cplx cvec_sum( cvec v )  
{
  idx_t i;
  cplx s = cplx_0;
  assert( v );
  for( i = 0 ; i < cvec_length( v ) ; i++ )
     s = cadd( s, v[ i ] );
  return s;
}


vec vec_cum_sum( vec v )  
{
  vec cs = vec_new( vec_length( v ) );
  idx_t i;
  assert( v );

  if( vec_length( v ) > 0 )
    cs[ 0 ] = v[ 0 ];
  
  for( i = 1 ; i < Vec_length( v ) ; i++ )
    cs[ i ] = cs[ i - 1 ] + v[ i ];
  
  return cs;
}


ivec ivec_cum_sum( ivec v )  
{
  ivec cs = ivec_new( ivec_length( v ) );
  idx_t i;
  assert( v );

  if( ivec_length( v ) > 0 )
    cs[ 0 ] = v[ 0 ];

  for( i = 1 ; i < Vec_length( v ) ; i++ ) {
    if( v[i] < 0 && cs[ i - 1 ] < INT_MIN - v[ i ] ) 
      it_warning("underflow in vec_cum_sum");
    if( v[i] > 0 && cs[ i - 1 ] > INT_MAX - v[ i ] ) 
      it_warning("overflow in vec_cum_sum");
    cs[ i ] = cs[ i - 1 ] + v[ i ];
  }
  return cs;
}


cvec cvec_cum_sum( cvec v )  
{
  cvec cs = cvec_new( cvec_length( v ) );
  idx_t i;
  assert( v );

  if( cvec_length( v ) > 0 )
    cs[ 0 ] = v[ 0 ];

  for( i = 1 ; i < cvec_length( v ) ; i++ ) {
    cs[ i ] = cadd( cs[ i - 1 ], v[ i ] );
  }
  return cs;
}


double vec_sum_sqr( vec v )  
{
  idx_t i;
  double s = 0;
  assert( v );
  for( i = 0 ; i < Vec_length( v ) ; i++ )
    s += v[ i ] * v[ i ];
  return s;
}


double vec_sum_between( vec v, idx_t i1, idx_t i2 )  
{
  idx_t i;
  double s = 0;
  VEC_END_PARAM( v, i2 );
  assert( v );
  for( i = i1 ; i <= i2 ; i++ )
    s += v[ i ];
  return s;
}


int ivec_sum_between( ivec v, idx_t i1, idx_t i2 )  
{
  idx_t i;
  int s = 0;
  VEC_END_PARAM( v, i2 );
  assert( v );
  for( i = i1 ; i <= i2 ; i++ )  {
      if(v[i] < 0 && s < INT_MIN - v[i]) it_warning("underflow in ivec_sum");
      if(v[i] > 0 && s > INT_MAX - v[i]) it_warning("overflow in ivec_sum");
      s += v[ i ];
  }
  return s;
}


cplx cvec_sum_between( cvec v, idx_t i1, idx_t i2 )  
{
  idx_t i;
  cplx s = cplx_0;
  VEC_END_PARAM( v, i2 );
  assert( v );
  for( i = i1 ; i <= i2 ; i++ ) 
      s = cadd( s, v[ i ] );
  return s;
}


double vec_min( vec v ) 
{
  idx_t i;
  double m = HUGE_VAL;
  assert( v );
  for( i = 0 ; i < Vec_length( v ) ; i++ )
    if( v[ i ] < m )
      m = v[ i ];
  return m;  
}


double vec_max( vec v ) 
{
  idx_t i;
  double m = -HUGE_VAL;
  assert( v );
  for( i = 0 ; i < Vec_length( v ) ; i++ )
    if( v[ i ] > m )
      m = v[ i ];
  return m;  
}


idx_t vec_min_index( vec v ) 
{
  idx_t i, mi = NULL_INDEX;
  double m = HUGE_VAL;
  assert( v );
  for( i = 0 ; i < vec_length( v ) ; i++ )
    if( v[ i ] < m ) {
      m = v[ i ];
      mi = i;
    }
  return mi;  
}


idx_t vec_max_index( vec v ) 
{
  idx_t i, mi = NULL_INDEX;
  double m = -HUGE_VAL;
  assert( v );
  for( i = 0 ; i < vec_length( v ) ; i++ )
    if( v[ i ] > m ) {
      m = v[ i ];
      mi = i;
    }
  return mi;  
}


double vec_mean( vec v ) 
{
  assert( v );
  return vec_sum( v ) / Vec_length( v );
}


double vec_median( vec v )
{
  vec c;
  double m;
  assert( v );
  if( vec_length( v ) == 0 ) {
    it_warning( "Undefined median value for vector of size 0. Return 0.\n" );
    return 0;
  }
  c = vec_clone( v );
  vec_sort( c );

  if( vec_length( v ) & 1 )
    m = c[ ( vec_length( v ) - 1 ) / 2 ];
  else
    m = ( c[ vec_length( v ) / 2 - 1 ] + c[ vec_length( v ) / 2 ] ) / 2;
  vec_delete( c );
  return m;
}


double vec_variance( vec v ) 
{
  idx_t i, l;
  double sum = 0;
  double var = 0;
  assert( v );
  l = Vec_length( v );
  assert( l > 1 );  /* otherwise the unbiased variance is not defined */
  for( i = 0 ; i < l ; i++ ) {
    sum += v[i];
    var += v[i] * v[i];
  }

  return (var - sum * sum / l) / (l - 1 );
}


double vec_norm( vec v, double n )  
{
  idx_t i;
  double nr = 0;
  assert( v );
  assert( n > 0 );
  for( i = 0 ; i < Vec_length( v ) ; i++ )
    nr += pow( fabs( v[ i ] ), n );

  return pow( nr, 1.0 / n );
}


/* General function                                                             */
vec vec_apply_function( vec v, it_function_t function, it_args_t args ) 
{
  idx_t i;
  idx_t l;
  assert( v );
  l = vec_length( v );
  for( i = 0 ; i < l; i++ )
    v[ i ] = function ( v[ i ], args );
  return v;
}


vec vec_new_apply_function( vec v, it_function_t function, it_args_t args ) 
{
  vec r;
  idx_t i;
  idx_t l;
  assert( v );
  l = vec_length( v );
  r = vec_new( l );
  for( i = 0 ; i < l ; i++ )
    r[ i ] = function ( v[ i ], args );
  return(r);
}


ivec ivec_apply_function( ivec v, it_ifunction_t function, it_args_t args ) 
{
  idx_t i;
  idx_t l;
  assert( v );
  l = ivec_length( v );
  for( i = 0 ; i < l; i++ )
    v[ i ] = function ( v[ i ], args );
  return v;
}


ivec ivec_new_apply_function( ivec v, it_ifunction_t function, it_args_t args ) 
{
  ivec r;
  idx_t i;
  idx_t l;
  assert( v );
  l = ivec_length( v );
  r = ivec_new( l );
  for( i = 0 ; i < l ; i++ )
    r[ i ] = function ( v[ i ], args );
  return(r);
}


/*------------------------------------------------------------------------------*/
void vec_reverse( vec v ) 
{ 
  idx_t i, j, m;
  double tmp;
  assert( v );

  m = vec_length( v ) / 2;
  for( i = 0, j = vec_length( v ) - 1; i < m ; i++, j-- ) {
    tmp = v[ i ];
    v[ i ] = v[ j ];
    v[ j ] = tmp;
  }
}


void ivec_reverse( ivec v ) 
{ 
  idx_t i, j, m;
  int tmp;
  assert( v );

  m = ivec_length( v ) / 2;
  for( i = 0, j = ivec_length( v ) - 1; i < m ; i++, j-- ) {
    tmp = v[ i ];
    v[ i ] = v[ j ];
    v[ j ] = tmp;
  }
}


void bvec_reverse( bvec v ) 
{ 
  idx_t i, j, m;
  byte tmp;
  assert( v );

  m = bvec_length( v ) / 2;
  for( i = 0, j = bvec_length( v ) - 1; i < m ; i++, j-- ) {
    tmp = v[ i ];
    v[ i ] = v[ j ];
    v[ j ] = tmp;
  }
}


void cvec_reverse( cvec v ) 
{ 
  idx_t i, j, m;
  double tmp_real, tmp_imag;
  assert( v );

  m = cvec_length( v ) / 2;
  for( i = 0, j = cvec_length( v ) - 1; i < m ; i++, j-- ) {
    tmp_real = creal( v[ i ] );
    tmp_imag = cimag( v[ i ] );
    creal( v[ i ] ) = creal( v[ j ] );
    cimag( v[ i ] ) = cimag( v[ j ] );
    creal( v[ j ] ) = tmp_real;
    creal( v[ j ] ) = tmp_imag;
  }
}


/*------------------------------------------------------------------------------*/
vec vec_new_reverse( vec v ) 
{
  idx_t i;
  vec cl;
  assert( v );

  cl = vec_new( vec_length( v ) );
  if( cl == NULL )
    return NULL;

  for( i = 0 ; i < vec_length( v ) ; i++ )
    cl[ i ] = v[ vec_length( v ) - 1 - i];

  return cl;
}

ivec ivec_new_reverse( ivec v ) 
{
  idx_t i;
  ivec cl;
  assert( v );

  cl = ivec_new( ivec_length( v ) );
  if( cl == NULL )
    return NULL;

  for( i = 0 ; i < ivec_length( v ) ; i++ )
    cl[ i ] = v[ ivec_length( v ) - 1 - i ];

  return cl;
}

bvec bvec_new_reverse( bvec v ) 
{
  idx_t i;
  bvec cl;
  assert( v );

  cl = bvec_new( bvec_length( v ) );
  if( cl == NULL )
    return NULL;

  for( i = 0 ; i < bvec_length( v ) ; i++ )
    cl[ i ] = v[ bvec_length( v ) - 1 - i ];

  return cl;
}


/*-----------------------------------------------------------------*/
/* Return the first position where the value a occurs              */
idx_t vec_find_first( vec v, double a ) 
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < Vec_length( v ) ; i++ )
    if( v[ i ] == a )
      return i;
  return NULL_INDEX;
}


idx_t ivec_find_first( ivec v, int a ) 
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < ivec_length( v ) ; i++ )
    if( v[ i ] == a )
      return i;
  return NULL_INDEX;
}


idx_t bvec_find_first( bvec v, byte a ) 
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < bvec_length( v ) ; i++ )
    if( v[ i ] == a )
      return i;
  return NULL_INDEX;
}


idx_t cvec_find_first( cvec v, cplx a ) 
{
  idx_t i;
  assert( v );
  for( i = 0 ; i < cvec_length( v ) ; i++ )
    if( ceq( v[ i ], a ) )
      return i;
  return NULL_INDEX;
}


/*-----------------------------------------------------------------*/
ivec vec_find( vec v, double a ) 
{
  idx_t count = vec_count( v, a );
  ivec pos = ivec_new( count );
  idx_t i, j;
  assert( v );
  
  for( i = 0, j = 0 ; i < Vec_length( v ) ; i++ )
    if( v[ i ] == a )
      pos[ j++ ] = i;

  return pos;
}


ivec ivec_find( ivec v, int a ) {
  idx_t count = ivec_count( v, a );
  ivec pos = ivec_new( count );
  idx_t i, j;
  assert( v );
  
  for( i = 0, j = 0 ; i < ivec_length( v ) ; i++ )
    if( v[ i ] == a )
      pos[ j++ ] = i;

  return pos;
}


ivec bvec_find( bvec v, byte a ) {
  idx_t count = bvec_count( v, a );
  ivec pos = ivec_new( count );
  idx_t i, j;
  assert( v );
  
  for( i = 0, j = 0 ; i < bvec_length( v ) ; i++ )
    if( v[ i ] == a )
      pos[ j++ ] = i;

  return pos;
}


ivec cvec_find( cvec v, cplx a ) {
  idx_t count = cvec_count( v, a );
  ivec pos = ivec_new( count );
  idx_t i, j;
  assert( v );
  
  for( i = 0, j = 0 ; i < cvec_length( v ) ; i++ )
    if( ceq( v[ i ], a ) )
      pos[ j++ ] = i;

  return pos;
}


/*-----------------------------------------------------------------*/
ivec vec_replace( vec v, double a, double b ) 
{
  idx_t count = vec_count( v, a );
  ivec pos = ivec_new( count );
  idx_t i, j;
  assert( v );
  
  for( i = 0, j = 0 ; i < Vec_length( v ) ; i++ )
    if( v[ i ] == a ) {
      pos[ j++ ] = i;
      v[ i ] = b;
    }

  return pos;
}


ivec ivec_replace( ivec v, int a, int b ) 
{
  idx_t count = ivec_count( v, a );
  ivec pos = ivec_new( count );
  idx_t i, j;
  assert( v );
  
  for( i = 0, j = 0 ; i < ivec_length( v ) ; i++ )
    if( v[ i ] == a ) {
      pos[ j++ ] = i;
      v[ i ] = b;
    }

  return pos;
}


ivec bvec_replace( bvec v, byte a, byte b ) 
{
  idx_t count = bvec_count( v, a );
  ivec pos = ivec_new( count );
  idx_t i, j;
  assert( v );
  
  for( i = 0, j = 0 ; i < bvec_length( v ) ; i++ )
    if( v[ i ] == a ) {
      pos[ j++ ] = i;
      v[ i ] = b;
    }

  return pos;
}


ivec cvec_replace( cvec v, cplx a, cplx b ) 
{
  idx_t count = cvec_count( v, a );
  ivec pos = ivec_new( count );
  idx_t i, j;
  assert( v );
  
  for( i = 0, j = 0 ; i < cvec_length( v ) ; i++ )
    if( ceq( v[ i ], a ) ) {
      pos[ j++ ] = i;
      v[ i ] = b;
    }

  return pos;
}


/*-----------------------------------------------------------------*/
idx_t vec_count( vec v, double a ) {
  idx_t c = 0;
  idx_t i;
  assert( v );
  for( i = 0 ; i < Vec_length( v ) ; i++ )
    if( v[ i ] == a )
      c++;
  return c;
}


idx_t bvec_count( bvec v, byte a ) {
  idx_t c = 0;
  idx_t i;
  assert( v );
  for( i = 0 ; i < bvec_length( v ) ; i++ )
    if( v[ i ] == a )
      c++;
  return c;
}


idx_t ivec_count( ivec v, int a ) {
  idx_t c = 0;
  idx_t i;
  assert( v );
  for( i = 0 ; i < ivec_length( v ) ; i++ )
    if( v[ i ] == a )
      c++;
  return c;
}


idx_t cvec_count( cvec v, cplx a ) {
  idx_t c = 0;
  idx_t i;
  assert( v );
  for( i = 0 ; i < cvec_length( v ) ; i++ )
    if( ceq( v[ i ], a ) )
      c++;
  return c;
}


/*----------------------------------------------------------------*/
/* Return the set of positions of value a                                       */
/*----------------------------------------------------------------*/

vec vec_concat( vec v1, vec v2 ) {
  idx_t i, j;
  vec v;
  assert( v1 );
  assert( v2 );
  v = vec_new( vec_length( v1 ) + vec_length( v2 ) );
  for( i = 0 ; i < vec_length( v1 ) ; i++ )
    v[ i ] = v1[ i ];

  for( j = 0 ; j < vec_length( v2 ) ; j++, i++ )
    v[ i ] = v2[ j ];
  return v;
}


ivec ivec_concat( ivec v1, ivec v2 ) {
  idx_t i, j;
  ivec v;
  assert( v1 );
  assert( v2 );
  v = ivec_new( ivec_length( v1 ) + ivec_length( v2 ) );
  for( i = 0 ; i < ivec_length( v1 ) ; i++ )
    v[ i ] = v1[ i ];

  for( j = 0 ; j < ivec_length( v2 ) ; j++, i++ )
    v[ i ] = v2[ j ];
  return v;
}


bvec bvec_concat( bvec v1, bvec v2 ) {
  idx_t i, j;
  bvec v;
  assert( v1 );
  assert( v2 );
  v = bvec_new( bvec_length( v1 ) + bvec_length( v2 ) );
  for( i = 0 ; i < bvec_length( v1 ) ; i++ )
    v[ i ] = v1[ i ];

  for( j = 0 ; j < bvec_length( v2 ) ; j++, i++ )
    v[ i ] = v2[ j ];
  return v;
}


cvec cvec_concat( cvec v1, cvec v2 ) {
  idx_t i, j;
  cvec v;
  assert( v1 );
  assert( v2 );
  v = cvec_new( cvec_length( v1 ) + cvec_length( v2 ) );
  for( i = 0 ; i < cvec_length( v1 ) ; i++ )
    v[ i ] = v1[ i ];

  for( j = 0 ; j < cvec_length( v2 ) ; j++, i++ )
    v[ i ] = v2[ j ];
  return v;
}


/*----------------------------------------------------------------*/

vec vec_unique( vec v ) 
{
  idx_t i;
  vec vsorted, vtmp = vec_new( 0 );

  assert( v );
  if( vec_length( v ) == 0 )
    return vtmp;

  vsorted = vec_clone( v );
  vec_sort( vsorted ); 

  vec_push( vtmp, vsorted[ 0 ] );
  for( i = 1 ; i < vec_length( v ) ; i++ )
    if( vsorted[ i ] != vsorted[ i - 1 ] )
      vec_push( vtmp, vsorted[ i ] );
  
  vec_delete( vsorted );
  return vtmp;
}


ivec ivec_unique( ivec v ) 
{
  idx_t i;
  ivec vsorted, vtmp = ivec_new( 0 );

  assert( v );
  if( ivec_length( v ) == 0 )
    return vtmp;

  vsorted = ivec_clone( v );
  ivec_sort( vsorted ); 

  ivec_push( vtmp, vsorted[ 0 ] );
  for( i = 1 ; i < ivec_length( v ) ; i++ )
    if( vsorted[ i ] != vsorted[ i - 1 ] )
      ivec_push( vtmp, vsorted[ i ] );
  
  ivec_delete( vsorted );
  return vtmp;
}


bvec bvec_unique( bvec v ) 
{
  idx_t i;
  bvec vsorted = bvec_new( 0 );     /* The vector that will be generated */
  ivec vtmp = ivec_new_zeros( 1 << ( 8 * sizeof( byte ) ) );

  assert( v );

  for( i = 0 ; i < bvec_length( v ) ; i++ )
    vtmp[ v[ i ] ]++;

  for( i = 0 ; i < ivec_length( vtmp ) ; i++ )
    if( vtmp[ i ] > 0 )
      bvec_push( vsorted, (byte) i );

  ivec_delete( vtmp );
  return vsorted;
}


vec vec_union( vec v1, vec v2 )
{
  idx_t i1, i2;
  vec vu1 = vec_unique( v1 );
  vec vu2 = vec_unique( v2 );
  vec vu = vec_new_alloc( 0, vec_length( vu1 ) + vec_length( vu2 ) );

  for( i1 = 0, i2 = 0 ; i1 < vec_length( vu1 ) && i2 < vec_length( vu2 ) ; )
    if( vu1[ i1 ] < vu2[ i2 ] )
      vec_push( vu, vu1[ i1++ ] );
    else if( vu1[ i1 ] > vu2[ i2 ] )
      vec_push( vu, vu2[ i2++ ] );
    else { 
      vec_push( vu, vu1[ i1 ] );
      i1++; 
      i2++; 
    };

  /* Put the remaining elements */
  while( i1 < vec_length( vu1 ) )
    vec_push( vu, vu1[ i1++ ] );

  while( i2 < vec_length( vu2 ) )
    vec_push( vu, vu2[ i2++ ] );

  vec_delete( vu1 );
  vec_delete( vu2 );
  return vu;
}


ivec ivec_union( ivec v1, ivec v2 )
{
  idx_t i1, i2;
  ivec vu1 = ivec_unique( v1 );
  ivec vu2 = ivec_unique( v2 );
  ivec vu = ivec_new_alloc( 0, ivec_length( vu1 ) + ivec_length( vu2 ) );
 
  for( i1 = 0, i2 = 0 ; i1 < ivec_length( vu1 ) && i2 < ivec_length( vu2 ) ; )
    if( vu1[ i1 ] < vu2[ i2 ] )
      ivec_push( vu, vu1[ i1++ ] );
    else if( vu1[ i1 ] > vu2[ i2 ] )
      ivec_push( vu, vu2[ i2++ ] );
    else  { 
      ivec_push( vu, vu1[ i1 ] );
      i1++; 
      i2++; 
    };

  /* Put the remaining elements */
  while( i1 < ivec_length( vu1 ) )
    ivec_push( vu, vu1[ i1++ ] );

  while( i2 < ivec_length( vu2 ) )
    ivec_push( vu, vu2[ i2++ ] );

  ivec_delete( vu1 );
  ivec_delete( vu2 );
  return vu;
}


bvec bvec_union( bvec v1, bvec v2 ) 
{
  idx_t i;
  bvec vsorted = bvec_new( 0 );     /* The vector that will be generated */
  ivec vtmp = ivec_new_zeros( 1 << ( 8 * sizeof( byte ) ) );

  assert( v1 );
  assert( v2 );

  for( i = 0 ; i < bvec_length( v1 ) ; i++ )
    vtmp[ v1[ i ] ]++;

  for( i = 0 ; i < bvec_length( v2 ) ; i++ )
    vtmp[ v2[ i ] ]++;

  for( i = 0 ; i < ivec_length( vtmp ) ; i++ )
    if( vtmp[ i ] > 0 )
      bvec_push( vsorted, (byte) i );

  ivec_delete( vtmp );
  return vsorted;
}


vec vec_intersection( vec v1, vec v2 )
{
  idx_t i1, i2;
  vec vu1 = vec_unique( v1 );
  vec vu2 = vec_unique( v2 );
  vec vu = vec_new_alloc( 0, vec_length( vu1 ) > vec_length( vu2 ) ? 
			    vec_length( vu2 ) : vec_length( vu1 ) );

  for( i1 = 0, i2 = 0 ; i1 < vec_length( vu1 ) && i2 < vec_length( vu2 ) ; i1++, i2++ ) {
    while( vu1[ i1 ] < vu2[ i2 ] && i1 < vec_length( vu1 ) )
      i1++;

    if( i1 == vec_length( vu1 ) )
      break;

    while( vu2[ i2 ] < vu1[ i1 ] && i2 < vec_length( vu2 ) )
      i2++;
    
    if( i2 == vec_length( vu2 ) )
      break;

    vec_push( vu, vu1[ i1 ] );
  }

  vec_delete( vu1 );
  vec_delete( vu2 );
  return vu;
}


ivec ivec_intersection( ivec v1, ivec v2 )
{
  idx_t i1, i2;
  ivec vu1 = ivec_unique( v1 );
  ivec vu2 = ivec_unique( v2 );
  ivec vu = ivec_new_alloc( 0, ivec_length( vu1 ) > ivec_length( vu2 ) ? 
			    ivec_length( vu2 ) : ivec_length( vu1 ) );

  for( i1 = 0, i2 = 0 ; i1 < ivec_length( vu1 ) && i2 < ivec_length( vu2 ) ; i1++, i2++ ) {
    while( vu1[ i1 ] < vu2[ i2 ] && i1 < ivec_length( vu1 ) )
      i1++;

    if( i1 == ivec_length( vu1 ) )
      break;

    while( vu2[ i2 ] < vu1[ i1 ] && i2 < ivec_length( vu2 ) )
      i2++;
    
    if( i2 == ivec_length( vu2 ) )
      break;

    ivec_push( vu, vu1[ i1 ] );
  }

  ivec_delete( vu1 );
  ivec_delete( vu2 );
  return vu;
}


bvec bvec_intersection( bvec v1, bvec v2 ) 
{
  idx_t i;
  bvec vsorted = bvec_new( 0 );     /* The vector that will be generated */
  ivec vtmp = ivec_new_zeros( 1 << ( 8 * sizeof( byte ) ) );

  assert( v1 );
  assert( v2 );

  for( i = 0 ; i < bvec_length( v1 ) ; i++ )
    vtmp[ v1[ i ] ]++;

  for( i = 0 ; i < bvec_length( v2 ) ; i++ )
    if( vtmp[ v2[ i ] ] > 0 )
      vtmp[ v2[ i ] ] = -1;

  for( i = 0 ; i < ivec_length( vtmp ) ; i++ )
    if( vtmp[ i ] == -1 )
      bvec_push( vsorted, (byte) i );

  ivec_delete( vtmp );
  return vsorted;
}


/*----------------------------------------------------------------*/
/* Return the vector composed of the elements of v indexed by idx */
vec vec_index_by( vec v, ivec idx ) {
  vec r = vec_new( ivec_length( idx ) );
  idx_t i;
  assert( v );
  assert( idx );
  for( i = 0 ; i < ivec_length( idx ) ; i++ )
    r[ i ] = v[ idx[ i ] ];
  return r;
}


ivec ivec_index_by( ivec v, ivec idx ) {
  ivec r = ivec_new( ivec_length( idx ) );
  idx_t i;
  assert( v );
  assert( idx );
  for( i = 0 ; i < ivec_length( idx ) ; i++ )
    r[ i ] = v[ idx[ i ] ];
  return r;
}


bvec bvec_index_by( bvec v, ivec idx ) {
  bvec r = bvec_new( ivec_length( idx ) );
  idx_t i;
  assert( v );
  assert( idx );
  for( i = 0 ; i < ivec_length( idx ) ; i++ )
    r[ i ] = v[ idx[ i ] ];
  return r;
}


cvec cvec_index_by( cvec v, ivec idx ) {
  cvec r = cvec_new( ivec_length( idx ) );
  idx_t i;
  assert( v );
  assert( idx );
  for( i = 0 ; i < ivec_length( idx ) ; i++ )
    r[ i ] = v[ idx[ i ] ];
  return r;
}


/*----------------------------------------------------------------*/
/* Sorting                                                        */

void Vec_qsort(Vec v, int (* elem_leq)(const void *, const void *))
{
  assert(v);
  qsort(v, Vec_length(v), Vec_element_size(v), elem_leq );
}

ivec Vec_qsort_index(Vec v, int (* elem_leq_idx)(const void *, const void *))
{
  /* The trick here is to create a vector of pointers to the elements
     to sort and view it jointly as a vector of integers of the form
     index * sizeof(element_type) + offset. The sort is done on the
     pointed elements, actually sorting the pointers. By doing some
     pointer arithmetic to remove the offset and divide properly by
     the element size, the indexes are retrieved.
     On platforms where sizeof(void *) > sizeof(int) it will return
     a pointer with unused allocated memory at the end.
  */
  int i;
  size_t elem_size = Vec_element_size(v);
  void **ptr = Vec_new(void *, Vec_length(v));
  ivec idx = (ivec) ptr;

  /* create a vector of pointers to each element of v */
  for(i = 0; i < Vec_length(v); i++)
    ptr[i] = (void *)((char *) v + i * elem_size);

  /* sort it */
  qsort(ptr, Vec_length(ptr), sizeof(void *), elem_leq_idx );

  /* now subtract the address of v from each element */
  for(i = 0; i < Vec_length(v); i++)
    idx[i] = (int) ((char *) ptr[i] - (char *) v) / elem_size;
 
  /* correct the vector size (ultimately ugly) */
  if(sizeof(void *) != sizeof(int)) {
    Vec_element_size(idx) = sizeof(int);
    Vec_length_max(idx) *= sizeof(void *) / sizeof(int);
  }
  return(idx);
}


/*----------------------------------------------------------------*/
static int double_leq( const void * d1, const void * d2 ) {
  if( *( (double *) d1) > *( (double *) d2) )
    return 1;
  else if( *( (double *) d1) == *( (double *) d2) )
    return 0;
  return -1;
}


static int double_leq_index( const void * d1, const void * d2 ) {
  return(double_leq(*(void **) d1, *(void **) d2));
}

void  vec_qsort( vec v ) {
  Vec_qsort(v, double_leq);
}


ivec vec_qsort_index( vec v ) {
  return(Vec_qsort_index(v, double_leq_index));
}


/*----------------------------------------------------------------*/
static int int_leq( const void * d1, const void * d2 ) {
  if( *( (int *) d1) > *( (int *) d2) )
    return 1;
  else if( *( (int *) d1) == *( (int *) d2) )
    return 0;
  return -1;
}


static int int_leq_index( const void * d1, const void * d2 ) {
  return(int_leq(*(void **) d1, *(void **) d2));
}

void ivec_qsort( ivec v ) {
  Vec_qsort(v, int_leq);
}


ivec ivec_qsort_index( ivec v ) {
  return(Vec_qsort_index(v, int_leq_index));
}


/*----------------------------------------------------------------*/
static int byte_leq( const void * d1, const void * d2 ) {
  if( *( (byte *) d1) > *( (byte *) d2) )
    return 1;
  else if( *( (byte *) d1) == *( (byte *) d2) )
    return 0;
  return -1;
}


static int byte_leq_index( const void * d1, const void * d2 ) {
  return(byte_leq(*(void **) d1, *(void **) d2));
}


void bvec_qsort( bvec v ) {
  Vec_qsort(v, byte_leq);
}


ivec bvec_qsort_index( bvec v ) {
  return(Vec_qsort_index(v, byte_leq_index));
}


/*---------------------------------------------------------------------------*/
/*                Special Vectors                                            */
/*---------------------------------------------------------------------------*/

/* The following functions proceeds with the modification of already allocated
   vector.                                                                   */

void vec_void( vec v ) {
  Vec_void( v );
}


void ivec_void( ivec v ) {
  Vec_void( v );
}


void bvec_void( bvec v ) {
  Vec_void( v );
}


void cvec_void( cvec v ) {
  Vec_void( v );
}


/*------------------------------------------------------------------------------*/
void vec_zeros( vec v ) {
  Vec_set( v, 0 );
}


void ivec_zeros( ivec v ) {
  Vec_set( v, 0 );
}


void bvec_zeros( bvec v ) {
  Vec_set( v, 0 );
}


void cvec_zeros( cvec v ) {
  Vec_set( v, cplx_0 );
}


/*------------------------------------------------------------------------------*/
void vec_ones( vec v ) {
  vec_set( v, 1 );
}


void ivec_ones( ivec v ) {
  ivec_set( v, 1 );
}


void bvec_ones( bvec v ) {
  bvec_set( v, 1 );
}


void cvec_ones( cvec v ) {
  cvec_set( v, cplx_1 );
}



/*------------------------------------------------------------------------------*/
void vec_range( vec v ) {
  idx_t i;
  assert( v );
  for( i = 0 ; i < vec_length( v ) ; i++ )
    v[ i ] = i;
}


void ivec_range( ivec v ) {
  idx_t i;
  assert( v );
  for( i = 0 ; i < ivec_length( v ) ; i++ )
    v[ i ] = i;
}


void bvec_range( bvec v ) {
  idx_t i;
  assert( v );
  for( i = 0 ; i < bvec_length( v ) ; i++ )
    v[ i ] = i;
}


void cvec_range( cvec v ) {
  idx_t i;
  assert( v );
  for( i = 0 ; i < cvec_length( v ) ; i++ ) {
    v[ i ].r = i;
    v[ i ].i = 0;
  }
}


/*------------------------------------------------------------------------------*/
void vec_1N( vec v ) {
  idx_t i;
  assert( v );
  for( i = 0 ; i < vec_length( v ) ; i++ )
    v[ i ] = i + 1;
}


void ivec_1N( ivec v ) {
  idx_t i;
  assert( v );
  for( i = 0 ; i < ivec_length( v ) ; i++ )
    v[ i ] = i + 1;
}


void bvec_1N( bvec v ) {
  idx_t i;
  assert( v );
  for( i = 0 ; i < bvec_length( v ) ; i++ )
    v[ i ] = i + 1;
}


void cvec_1N( cvec v ) {
  idx_t i;
  assert( v );
  for( i = 0 ; i < cvec_length( v ) ; i++ ) {
    v[ i ].r = i + 1;
    v[ i ].i = 0;
  }
}


/*------------------------------------------------------------------------------*/
void vec_arithm( vec v, double start, double incr ) {
  idx_t i;
  double value = start;
  assert( v );
  for( i = 0 ; i < vec_length( v ) ; i++, value += incr )
    v[ i ] = value;
}


void ivec_arithm( ivec v, int start, int incr ) {
  idx_t i;
  int value = start;
  assert( v );
  for( i = 0 ; i < ivec_length( v ) ; i++, value += incr )
    v[ i ] = value;
}


void bvec_arithm( bvec v, byte start, byte incr ) {
  idx_t i;
  byte value = start;
  assert( v );
  for( i = 0 ; i < bvec_length( v ) ; i++, value += incr )
    v[ i ] = value;
}


void cvec_arithm( cvec v, cplx start, cplx incr ) {
  idx_t i;
  cplx value = start;
  assert( v );
  for( i = 0 ; i < cvec_length( v ) ; i++, value = cadd( value, incr ) )
    v[ i ] = value;
}


/*------------------------------------------------------------------------------*/
void vec_geom( vec v, double start, double r ) {
  idx_t i;
  double value = start;
  assert( v );
  for( i = 0 ; i < vec_length( v ) ; i++, value *= r )
    v[ i ] = value;
}


void ivec_geom( ivec v, int start, int r ) {
  idx_t i;
  int value = start;
  assert( v );
  for( i = 0 ; i < ivec_length( v ) ; i++, value *= r )
    v[ i ] = value;
}


void bvec_geom( bvec v, byte start, byte r ) {
  idx_t i;
  byte value = start;
  assert( v );
  for( i = 0 ; i < bvec_length( v ) ; i++, value *= r )
    v[ i ] = value;
}


void cvec_geom( cvec v, cplx start, cplx r ) {
  idx_t i;
  cplx value = start;
  assert( v );
  for( i = 0 ; i < cvec_length( v ) ; i++, value = cmul( value, r ) )
    v[ i ] = value;
}


/*------------------------------------------------------------------------------*/
/* Note: the functions returning a vector pointer allocate memory that must     */
/* be free afterwards                                                           */

vec vec_new_void() {
  return vec_new( 0 );
}


ivec ivec_new_void() {
  return ivec_new( 0 );
}


bvec bvec_new_void() {
  return bvec_new( 0 );
}

cvec cvec_new_void() {
  return cvec_new( 0 );
}


/*---------------------------------------------------------------------------*/
vec vec_new_set( double val, idx_t N ) {
  vec v = vec_new(N);
  vec_set(v, val);
  return(v);
}

ivec ivec_new_set( int val, idx_t N ) {
  ivec v = ivec_new(N);
  ivec_set(v, val);
  return(v);
}

bvec bvec_new_set( byte val, idx_t N ) {
  bvec v = bvec_new(N);
  bvec_set(v, val);
  return(v);
}

cvec cvec_new_set( cplx val, idx_t N ) {
  cvec v = cvec_new(N);
  cvec_set(v, val);
  return(v);
}

/*------------------------------------------------------------------------------*/
vec vec_new_1N( idx_t N ) {
  idx_t i;
  vec v;
  v = vec_new( N );
  for( i = 0 ; i < N ; i++ )
    v[ i ] = (double) (i+1);
  return v;
}


ivec ivec_new_1N( idx_t N ) {
  idx_t i;
  ivec v;
  v = ivec_new( N );
  for( i = 0 ; i < N ; i++ )
    v[ i ] = i+1;
  return v;
}


bvec bvec_new_1N( idx_t N ) {
  idx_t i;
  bvec v;
  v = bvec_new( N );
  for( i = 0 ; i < N ; i++ )
    v[ i ] = i+1;
  return v;
}


cvec cvec_new_1N( idx_t N ) {
  idx_t i;
  cvec v;
  v = cvec_new( N );
  for( i = 0 ; i < N ; i++ ) {
    creal(v[ i ]) = i+1;
    cimag(v[ i ]) = 0;
  }
  return v;
}

vec vec_new_range( idx_t N ) {
  idx_t i;
  vec v;
  v = vec_new( N );
  for( i = 0 ; i < N ; i++ )
    v[ i ] = (double) (i);
  return v;
}


ivec ivec_new_range( idx_t N ) {
  idx_t i;
  ivec v;
  v = ivec_new( N );
  for( i = 0 ; i < N ; i++ )
    v[ i ] = i;
  return v;
}


bvec bvec_new_range( idx_t N ) {
  idx_t i;
  bvec v;
  v = bvec_new( N );
  for( i = 0 ; i < N ; i++ )
    v[ i ] = i;
  return v;
}

cvec cvec_new_range( idx_t N ) {
  idx_t i;
  cvec v;
  v = cvec_new( N );
  for( i = 0 ; i < N ; i++ ) {
    creal( v[ i ] ) = i;
    cimag( v[ i ] ) = 0;
  }
  return v;
}


/*------------------------------------------------------------------------------*/
vec vec_new_arithm( double start, double incr, idx_t N ) {
  idx_t i;
  double value = start;
  vec v;
  v = vec_new( N );
  for( i = 0 ; i < N ; i++, value += incr )
    v[ i ] = value;
  return v;
}


ivec ivec_new_arithm( int start, int incr, idx_t N ) {
  idx_t i;
  int value = start;
  ivec v;
  v = ivec_new( N );
  for( i = 0 ; i < N ; i++, value += incr )
    v[ i ] = value;
  return v;
}


bvec bvec_new_arithm( byte start, byte incr, idx_t N ) {
  idx_t i;
  int value = start;
  bvec v;
  v = bvec_new( N );
  for( i = 0 ; i < N ; i++, value += incr )
    v[ i ] = value;
  return v;
}

cvec cvec_new_arithm( cplx start, cplx incr, idx_t N ) {
  idx_t i;
  cplx value = start;
  cvec v;
  v = cvec_new( N );
  for( i = 0 ; i < N ; i++) {
    v[ i ] = value;
    value = cadd(value, incr);
  }
  return v;
}


/*------------------------------------------------------------------------------*/
vec vec_new_geom( double start, double r, idx_t N ) {
  idx_t i;
  double value = start;
  vec v;
  it_assert( N >= 0, "Sequence should not be of negative length" );
  v = vec_new( N );
  for( i = 0 ; i < N ; i++, value *= r )
    v[ i ] = value;
  return v;
}


ivec ivec_new_geom( int start, int r, idx_t N ) {
  idx_t i;
  int value = start;
  ivec v;
  it_assert( N >= 0, "Sequence should not be of negative length" );
  v = ivec_new( N );
  for( i = 0 ; i < N ; i++, value *= r )
    v[ i ] = value;
  return v;
}


bvec bvec_new_geom( byte start, byte r, idx_t N ) {
  idx_t i;
  byte value = start;
  bvec v;
  it_assert( N >= 0, "Sequence should not be of negative length" );
  v = bvec_new( N );
  for( i = 0 ; i < N ; i++, value *= r )
    v[ i ] = value;
  return v;
}

cvec cvec_new_geom( cplx start, cplx r, idx_t N ) {
  idx_t i;
  cplx value = start;
  cvec v;
  v = cvec_new( N );
  for( i = 0 ; i < N ; i++) {
    v[ i ] = value;
    value = cmul(value, r);
  }
  return v;
}

/*------------------------------------------------------------------------------*/
cvec cvec_new_unit_roots(idx_t N)
{
  idx_t k;
  cvec v;

  v = cvec_new(N);

  /* generate e^{2i\pi k / N} = cos(2 k \pi / N) + i sin(2 k \pi / N) */
  for(k = 0; k < N; k++) {
    creal(v[k]) = cos(2 * k * M_PI / N);
    cimag(v[k]) = sin(2 * k * M_PI / N);
  }

  return(v);
}


/*------------------------------------------------------------------------------*/
vec vec_conv( vec v1, vec v2 )
{
  int i1, i2;
  vec v = vec_new_zeros( vec_length( v1 ) + vec_length( v2 ) - 1 );

  for( i1 = 0 ; i1 < vec_length( v1 ) ; i1++ )
    for( i2 = 0 ; i2 < vec_length( v2 ) ; i2++ )
      v[ i1 + i2 ] += v1[ i1 ] * v2[ i2 ];

  return v;
}


/*------------------------------------------------------------------------------*/
ivec ivec_conv( ivec v1, ivec v2 )
{
  int i1, i2;
  ivec v = ivec_new_zeros( ivec_length( v1 ) + ivec_length( v2 ) - 1 );

  for( i1 = 0 ; i1 < ivec_length( v1 ) ; i1++ )
    for( i2 = 0 ; i2 < ivec_length( v2 ) ; i2++ )
      v[ i1 + i2 ] += v1[ i1 ] * v2[ i2 ];

  return v;
}


/*---------------------------------------------------------------------------*/
void vec_rand( vec v )
{
  int i;
  for( i = 0 ; i < vec_length(v) ; i++ )
    v[i] = it_rand();
}

 
void vec_randn( vec v )
{
  int i;
  for( i = 0 ; i < vec_length(v) ; i++ )
    v[i] = it_randn();
}


/*---------------------------------------------------------------------------*/
vec vec_new_rand( idx_t n )
{
  vec v = vec_new( n );
  vec_rand(v);
  return v;
}

 
vec vec_new_randn( idx_t n )
{
  vec v = vec_new( n );
  vec_randn(v);
  return v;
}

/*

  Knuth (or Fisher-Yates) shuffle.

  See: R. A. Fisher and F. Yates, Example 12,
       Statistical Tables, London, 1938.

  Generates uniformly random permutations.

  Input  : - len  : size of vector
           - seed : to initialize the PRNG
  Output : a len-long ivec containing a shuffle
           between 0 and len-1.

  Allocate memory : YES (ivec[len])

 */

ivec ivec_new_perm( size_t len, unsigned int seed )
{

  ivec  perm;
  idx_t i = 0;
  idx_t p = 0;
  int   n = 0;

  it_assert( len>1, "Permutation of a scalar is crazy" );

  it_seed( seed );
  perm = ivec_new_arithm( 0, 1, len );

  for ( i= 0; i< len; i++ )
    {
      p = i + (unsigned int)(it_rand()*(len-i));
      n = perm[p];
      perm[p] = perm[i];
      perm[i] = n;
    }

  return( perm );

}

