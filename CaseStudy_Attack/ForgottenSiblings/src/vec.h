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
  Vectors functions
  Copyright (C) 2005 Vivien Chappelier, Herve Jegou
*/


#ifndef __it_vec_h
#define __it_vec_h

#include "cplx.h"
#include "types.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/*        Indexes                                                            */
/*---------------------------------------------------------------------------*/

/* the idx_t type is always equivalent to the int type. It is defined here
   for clarity of the meaning of parameters or iterators inside the lib, but
   can be interchanged for int any time. The first reason is we don't want
   to bother people using the lib with more cryptic typedefs than needed, so
   that they can use 'int' to index vectors without remorse. The second
   reason is we want to be compatible with ivec when returning sets of
   indexes, so that a special Vec(idx_t) need not be defined.
   This has the consequence to limit the maximum vector size to the larger
   signed int on the architecture. On most current architectures this 
   corresponds to 2GB per bvec which Should Be Enough For Everybody (TM).
   Negative values are used for indexes with a special meaning but are
   invalid indexes for dereferencing vectors (e.g. v[end] is invalid).
 */
typedef int idx_t;

/* the end of the vector */
static const idx_t end = (idx_t) INT_MIN;
/* the null index (not found, not existing, ...) */
static const idx_t NULL_INDEX = (idx_t) INT_MIN + 1;

/* remove a warning about unused constants on MacOS X */
static inline idx_t __vec_idx_warnoff__(void) { return(end|NULL_INDEX); }

/*---------------------------------------------------------------------------*/
/*                Vector types                                               */
/*---------------------------------------------------------------------------*/

/* Notes: the following structures are used to store some vectors values. 
   For a given vector v of type type_t (with type_t=int, double, byte or complex), 
   the memory storage is managed as follows (1b=1 byte):

   variable      type          memory pos               purpose

   none          none          _Vec_header[v]->ptr     start of the memory block allocated for the vector.
                                                       This position is not always the one of the vector header, 
                                                       because a word alignment is enforced while a vector is allocated. 
   Vec_header(v) _Vec_header_  v-sizeof(header) bytes  Vector header (see below)
   v[0]         type_t         v                       first element of the vector
   v[1]         type_t         v+sizeof(type_t) bytes  second element...
   v[length-1]  type_t         v+length*sizeof(type_t) last element of the vector
   v[length], ... v[max_length-1] pre-allocated but not currently unused.

   Thus, the amount of memory allocated is greater than the number of element by at least
   sizeof(_Vec_header_) bytes, i.e. about 16 bytes on a 32 bits machine. 
   Usually, vector creation functions allocate vectors such that length=length_max. 
   When the vector length decreases, the already allocated memory remains allocated. 
   When the vector length increases, a geometric reallocation strategy is used. 
*/

typedef struct _Vec_header_ {
  idx_t length;        /* effective length of the vector (<= max_length)     */
  idx_t length_max;    /* amount of memory allocated for the vector elements */
  void *ptr;           /* memory block associated to this vector             */
  size_t element_size; /* size of the stored elements                        */
} Vec_header_t;

/* Return the header associated with a given vector v */
#define Vec_header(v) (*(((Vec_header_t *) (v)) - 1))

/* Return the length of the vector and the number of allocated elements      */
#define Vec_length( v ) ( Vec_header(v).length )
#define Vec_length_max( v ) ( Vec_header(v).length_max )
#define Vec_element_size( v ) ( Vec_header(v).element_size )

typedef void * Vec;
typedef double * vec;
typedef int * ivec;
typedef byte * bvec;
typedef cplx * cvec;

/*---------------------------------------------------------------------------*/
/*        Constant vectors                                                   */
/*---------------------------------------------------------------------------*/

/* the empty vectors */
extern vec const vec_null;
extern ivec const ivec_null;
extern bvec const bvec_null;
extern cvec const cvec_null;

/*---------------------------------------------------------------------------*/
/*                Allocation, initialisation and free                        */
/*---------------------------------------------------------------------------*/

/* The reallocation of memory for vectors is done here according to a geometric rule
   so that the complexity of the overall reallocation proccess remains linear.        
   The following value define the ratio of the realloction procedure.
   Note that the value is a tradeoff between memory overhead and computing cost */
#define DYN_ALLOC_RULE(l) ((l)*3/2+1)

/* Free the vector                                                              */
#define Vec_delete( v ) free( Vec_header(v).ptr ) 

/* We want to make sure the memory is always properly aligned on an 8-byte
   boundary. This is needed to guarantee pointer arithmetic works with double.
   Besides it provides faster memory access in case of MMX/SSE/... optimizations.
*/
#define IT_ALLOC_ALIGN (sizeof(double))

/* Vector allocation functions */
Vec __Vec_new_alloc(size_t elem_size, idx_t length, idx_t length_max);
Vec __Vec_new_realloc(void *V, size_t elem_size, idx_t length, idx_t length_max);

/* Allocate a vector with explicit allocation of the maximum nb of elements     */
#define Vec_new_alloc( type_t, N, length_max ) \
    ((type_t *) __Vec_new_alloc(sizeof(type_t), N, length_max ))

/* Allocate a vector                                                            */
#define Vec_new( type_t, N ) ((type_t *) __Vec_new( sizeof(type_t), N ))
static inline Vec __Vec_new( size_t elem_size, idx_t N )
{
  return(__Vec_new_alloc( elem_size, N, N ));
}

/* Never decrease the size of the vector (call Vec_set_length_max for that purpose) */
#define Vec_set_length( v, N )      do {                                    \
                                      if( N > Vec_length_max( v ) ) {       \
                                        Vec_set_length_max( v, N );         \
                                        Vec_length( v ) = (N); }            \
                                      else {                                \
                                      Vec_length( v ) = (N); }              \
                                     } while(0)
 
/* Warning: the function Vec_set_length_max may change the value of the pointer
   corresponding to vector v. Consequently, the functions calling this function
   must be handled with care, especially when vectors are passed as parameters   */
#define Vec_set_length_max( v, N ) do {                         \
  int L = Vec_length(v);					\
  size_t elem_size = Vec_element_size(v);			\
								\
  assert( N >= Vec_length(v) );					\
  if( N > Vec_length(v) ) {					\
    Vec new_Vec = __Vec_new_realloc(v, elem_size, L, N);	\
    (v) = (v) + ((byte *) new_Vec - (byte *) (v)) / elem_size;	\
  }								\
} while(0)

/* to use the entity end in a vec function, this macro is called */
#define VEC_END_PARAM( v, param ) do { if( param == end ) param = Vec_length( v ) - 1; } while(0)
     
/* Initialize the vector from a buffer of the same type                         */
#define Vec_init( v, buf, N ) memcpy( v, buf, Vec_element_size(v) * (N) );

/* Set all the elements of the vector to value val                              */
#define Vec_set( v, val ) do {                                              \
                            idx_t i;                                       \
                            assert( v );                                    \
                            for( i = 0 ; i < Vec_length( v ) ; i++ )        \
                              v[ i ] = val; } while( 0 )

/* Set all the elements of the vector to value val                              */
#define Vec_set_between( v, i1, i2, val ) do {                            \
                            idx_t ve = i2;                                 \
                            idx_t i;                                       \
                            VEC_END_PARAM( v,ve );                          \
                            assert( v );                                    \
                            for( i = i1 ; i <= ve ; i++ )                   \
                              v[ i ] = val; } while( 0 )

/* Set elements of the vector v at offset index to the value of elements of s */
#define Vec_set_subvector( v, s, index ) do {				   \
                            idx_t i;					   \
			    assert(v);					   \
			    assert(s);					   \
			    assert(index + Vec_length(s) <= Vec_length(v)); \
			    for(i = 0; i < Vec_length(s); i++)		   \
			      (v)[(index) + i] = (s)[i]; } while(0)

#define Vec_copy( v1, v2 ) __Vec_copy((Vec) v1, (Vec) v2)
static inline Vec __Vec_copy(Vec v1, Vec v2)
{
  assert(v1);
  assert(v2);
  assert(Vec_element_size(v1) == Vec_element_size(v2));
  assert(Vec_length(v1) == Vec_length(v2));
  memcpy( v1, v2, Vec_element_size( v1 ) * Vec_length( v1 ) );
  return(v1);
}

#define Vec_eq( v1, v2 ) ( Vec_length( v1 ) == Vec_length( v2 ) &&          \
                         !memcmp( v1, v2, Vec_element_size( v1 ) * Vec_length( v1 ) ) )

#define Vec_del( v, pos ) do {                                              \
                               idx_t l = Vec_length(v)-1;                   \
                               idx_t i;                                     \
                               assert(v);                                   \
                               assert((pos) <= l);                          \
                               for( i = (pos) + 1 ; i <= l ; i++ )          \
                                 (v)[ i - 1 ] = (v)[ i ];                   \
                               Vec_length(v)--;                             \
                             } while( 0 )

/* The following function MAY modify the value of the pointer v                */
/* The vector max length is increased according to a geometric allocation   */
#define Vec_ins( v, pos, elt ) do {                                         \
                               idx_t l = Vec_length(v);                    \
                               idx_t i;                                    \
                               assert(v);                                   \
                               assert((pos) <= l);                          \
                               if( l+1 > Vec_length_max(v) )                \
                                 Vec_set_length_max( v, DYN_ALLOC_RULE( l ));\
                               for( i = l ; i >(pos) ; i-- )                \
                                 (v)[i] = (v)[ i-1 ];                       \
                               (v)[(pos)] = (elt);                          \
                               Vec_length(v)++;                             \
                             } while( 0 )

/* Add an element at the end of the vector (Use vector as a stack)              */
#define Vec_push( v, elt ) Vec_ins( v, Vec_length(v), elt )

/* Retrieve the last element at the end of the vector (Use vector as a stack)   */
#define Vec_pop( v ) Vec_del( v, Vec_length(v)-1 )

/* Return the last element of vector                                            */
#define Vec_head( v ) ((v)[Vec_length(v)-1])

/* Basic vector functions (specialization of previous #define)                  */
static inline vec vec_new( idx_t length ) { return Vec_new( double, length ); }
static inline ivec ivec_new( idx_t length ) { return Vec_new( int, length ); }
static inline bvec bvec_new( idx_t length ) { return Vec_new( byte, length ); }
static inline cvec cvec_new( idx_t length ) { return Vec_new( cplx, length ); }

static inline vec vec_new_alloc( idx_t length, idx_t length_max ) { assert( length_max >= length ); return Vec_new_alloc( double, length, length_max ); }
static inline ivec ivec_new_alloc( idx_t length, idx_t length_max ) { assert( length_max >= length ); return Vec_new_alloc( int, length, length_max ); }
static inline bvec bvec_new_alloc( idx_t length, idx_t length_max ) { assert( length_max >= length ); return Vec_new_alloc( byte, length, length_max ); }
static inline cvec cvec_new_alloc( idx_t length, idx_t length_max ) { assert( length_max >= length ); return Vec_new_alloc( cplx, length, length_max ); }

static inline void vec_delete( vec v ) { Vec_delete( v ); }
static inline void ivec_delete( ivec v ) { Vec_delete( v ); } 
static inline void bvec_delete( bvec v ) { Vec_delete( v ); }
static inline void cvec_delete( cvec v ) { Vec_delete( v ); }

static inline idx_t vec_length( vec v ) { return Vec_length( v ); }
static inline idx_t ivec_length( ivec v ) { return Vec_length( v ); }
static inline idx_t bvec_length( bvec v ) { return Vec_length( v ); }
static inline idx_t cvec_length( cvec v ) { return Vec_length( v ); }

static inline idx_t vec_length_max( vec v ) { return Vec_length_max( v ); }
static inline idx_t ivec_length_max( ivec v ) { return Vec_length_max( v ); }
static inline idx_t bvec_length_max( bvec v ) { return Vec_length_max( v ); }
static inline idx_t cvec_length_max( cvec v ) { return Vec_length_max( v ); }

static inline vec _vec_set_length( vec v, idx_t N ) { Vec_set_length( v, N ); return v; }
static inline ivec _ivec_set_length( ivec v, idx_t N ) { Vec_set_length( v, N ); return v; }
static inline bvec _bvec_set_length( bvec v, idx_t N ) { Vec_set_length( v, N ); return v; }
static inline cvec _cvec_set_length( cvec v, idx_t N ) { Vec_set_length( v, N ); return v; }

#define vec_set_length( v, N )  do { v=_vec_set_length( v, N ); } while (0)
#define ivec_set_length( v, N ) do { v=_ivec_set_length( v, N ); } while (0)
#define bvec_set_length( v, N ) do { v=_bvec_set_length( v, N ); } while (0)
#define cvec_set_length( v, N ) do { v=_cvec_set_length( v, N ); } while (0)

static inline vec  _vec_set_length_max( vec v, idx_t N ) { Vec_set_length_max( v, N ); return v;}
static inline ivec _ivec_set_length_max( ivec v, idx_t N ) { Vec_set_length_max( v, N ); return v; }
static inline bvec _bvec_set_length_max( bvec v, idx_t N ) { Vec_set_length_max( v, N ); return v; }
static inline cvec _cvec_set_length_max( cvec v, idx_t N ) { Vec_set_length_max( v, N ); return v; }

#define  vec_set_length_max( v, N )  do { v=_vec_set_length_max( v, N ); } while (0)
#define ivec_set_length_max( v, N ) do { v=_ivec_set_length_max( v, N ); } while (0)
#define bvec_set_length_max( v, N ) do { v=_bvec_set_length_max( v, N ); } while (0)
#define cvec_set_length_max( v, N ) do { v=_cvec_set_length_max( v, N ); } while (0)

static inline vec vec_init( vec v, double * buf, idx_t N ) { return (vec) Vec_init( v, buf, N ); }
static inline ivec ivec_init( ivec v, int * buf, idx_t N ) { return (ivec) Vec_init( v, buf, N ); }
static inline bvec bvec_init( bvec v, byte * buf, idx_t N ) { return (bvec) Vec_init( v, buf, N ); }
static inline cvec cvec_init( cvec v, cplx * buf, idx_t N ) { return (cvec) Vec_init( v, buf, N ); }

#define _vec(v, i) (*__vec(v, i))
static inline double *__vec(vec v, idx_t i) {
  assert(v);
  VEC_END_PARAM( v, i );
  assert(i >= 0 && i < vec_length(v));
  return(&v[i]);
}

#define _ivec(v, i) (*__ivec(v, i))
static inline int *__ivec(ivec v, idx_t i) {
  assert(v);
  VEC_END_PARAM( v, i );
  assert(i >= 0 && i < ivec_length(v));
  return(&v[i]);
}

#define _bvec(v, i) (*__bvec(v, i))
static inline byte *__bvec(bvec v, idx_t i) {
  assert(v);
  VEC_END_PARAM( v, i );
  assert(i >= 0 && i < bvec_length(v));
  return(&v[i]);
}

#define _cvec(v, i) (*__cvec(v, i))
static inline cplx *__cvec(cvec v, idx_t i) {
  assert(v);
  VEC_END_PARAM( v, i );
  assert(i >= 0 && i < cvec_length(v));
  return(&v[i]);
}

static inline vec  vec_set( vec v, double val ) { Vec_set( v, val ); return v;}
static inline ivec ivec_set( ivec v, int val ) { Vec_set( v, val );  return v;}
static inline bvec bvec_set( bvec v, byte val ) { Vec_set( v, val );  return v;}
static inline cvec cvec_set( cvec v, cplx val ) { Vec_set( v, val );  return v;}

static inline vec  vec_set_between( vec v, idx_t i1, idx_t i2, double val ) { Vec_set_between( v, i1, i2, val ); return v;}
static inline ivec ivec_set_between( ivec v, idx_t i1, idx_t i2, int val ) { Vec_set_between( v, i1, i2, val ); return v;}
static inline bvec bvec_set_between( bvec v, idx_t i1, idx_t i2, byte val ) { Vec_set_between( v, i1, i2, val ); return v;}
static inline cvec cvec_set_between( cvec v, idx_t i1, idx_t i2, cplx val ) { Vec_set_between( v, i1, i2, val ); return v;}

static inline vec vec_get_subvector( vec v, idx_t i1, idx_t i2 )    { vec s;  VEC_END_PARAM( v, i1 ); VEC_END_PARAM( v, i2 ); s = vec_new( i2-i1+1 );  vec_init( s, v+i1, i2-i1+1 ); return s;}
static inline ivec ivec_get_subvector( ivec v, idx_t i1, idx_t i2 ) { ivec s; VEC_END_PARAM( v, i1 ); VEC_END_PARAM( v, i2 ); s = ivec_new( i2-i1+1 ); ivec_init( s, v+i1, i2-i1+1 ); return s;}
static inline bvec bvec_get_subvector( bvec v, idx_t i1, idx_t i2 ) { bvec s; VEC_END_PARAM( v, i1 ); VEC_END_PARAM( v, i2 ); s = bvec_new( i2-i1+1 ); bvec_init( s, v+i1, i2-i1+1 ); return s;}
static inline cvec cvec_get_subvector( cvec v, idx_t i1, idx_t i2 ) { cvec s; VEC_END_PARAM( v, i1 ); VEC_END_PARAM( v, i2 ); s = cvec_new( i2-i1+1 ); cvec_init( s, v+i1, i2-i1+1 ); return s;}

static inline vec vec_set_subvector( vec v, vec s, idx_t idx ) { VEC_END_PARAM( v, idx ); Vec_set_subvector( v, s, idx ); return v; }
static inline ivec ivec_set_subvector( ivec v, ivec s, idx_t idx ) { VEC_END_PARAM( v, idx ); Vec_set_subvector( v, s, idx ); return v; }
static inline bvec bvec_set_subvector( bvec v, bvec s, idx_t idx ) { VEC_END_PARAM( v, idx ); Vec_set_subvector( v, s, idx ); return v; }
static inline cvec cvec_set_subvector( cvec v, cvec s, idx_t idx ) { VEC_END_PARAM( v, idx ); Vec_set_subvector( v, s, idx ); return v; }

/*------------------------------------------------------------------------------*/
/*                Copy and Conversions Functions                                */
/*------------------------------------------------------------------------------*/

/* Copy the content of a vector into another vector                            
   Note that these functions do NOT allocate memory for the destination vectors. 
   Consequently, the amount of allocated elements vec_length_max(v) must be sufficient. 
   The function returns the number of elements that has really been copied      
   If the vectors have not the same size, the function returns an error         */

static inline void vec_copy( vec dest, vec orig ) { Vec_copy(dest, orig); }
static inline void ivec_copy( ivec dest, ivec orig ) { Vec_copy(dest, orig); }
static inline void bvec_copy( bvec dest, bvec orig ) { Vec_copy(dest, orig); }
static inline void cvec_copy( cvec dest, cvec orig ) { Vec_copy(dest, orig); }

/* The same function as copy, but which allocate some memory for the destination vector */
static inline Vec Vec_clone( Vec v ) { assert(v); return(Vec_copy(__Vec_new(Vec_element_size(v), Vec_length(v)), v)); }
static inline vec vec_clone( vec v ) { assert(v); return((vec) Vec_copy(vec_new(vec_length(v)), v)); }
static inline ivec ivec_clone( ivec v ) { assert(v); return((ivec) Vec_copy(ivec_new(ivec_length(v)), v)); }
static inline bvec bvec_clone( bvec v ) { assert(v); return((bvec) Vec_copy(bvec_new(bvec_length(v)), v)); }
static inline cvec cvec_clone( cvec v ) { assert(v); return((cvec) Vec_copy(cvec_new(cvec_length(v)), v)); }

/* Copy the vector into a buffer (which must be a valid allocated pointer)      */
void vec_copy_mem( double * buf, vec v );
void ivec_copy_mem( int * buf, ivec v );
void bvec_copy_mem( byte * buf, bvec v );
void cvec_copy_mem( cplx * buf, cvec v );

/* Pack/unpack all bits of a binary vector together */
void bvec_pack( byte * buf, bvec v );
void bvec_unpack( bvec v, byte * buf );

/* Conversion */
bvec vec_to_bvec( vec v );
ivec vec_to_ivec( vec v );
cvec vec_to_cvec( vec v);

vec  ivec_to_vec( ivec v );
bvec ivec_to_bvec( ivec v );
cvec ivec_to_cvec( ivec v );

ivec bvec_to_ivec( bvec v );
vec  bvec_to_vec( bvec v );
cvec bvec_to_cvec( bvec v );

/*------------------------------------------------------------------------------*/
/*                Comparisons functions                                         */
/*------------------------------------------------------------------------------*/

/* Return true iff all the vectors v1 and v2 are equal component per component  */
int vec_eq( vec v1, vec v2 );
int ivec_eq( ivec v1, ivec v2 );
int bvec_eq( bvec v1, bvec v2 );
int cvec_eq( cvec v1, cvec v2 );

/* Return 1 if vector v1 is lexicographically greater or equal to vector v2, 
   0 otherwise. Note that if v1 is a prefix of v2, v1 is assumed to be smaller  */
int vec_geq( vec v1, vec v2 );
int ivec_geq( ivec v1, ivec v2 );
int bvec_geq( bvec v1, bvec v2 );


/*------------------------------------------------------------------------------*/
/*                Arithmetic functions                                          */
/*------------------------------------------------------------------------------*/

/* Operations with a scalar value                                               */
void vec_incr( vec v, double a );          /* add a to the components              */
void vec_decr( vec v, double a );           /* substract a to the components              */
void vec_mul_by( vec v, double a );         /* multiply the components per a        */
void vec_div_by( vec v, double a );         /* divide the components per a          */

void ivec_incr( ivec v, int a );           /* add a to the components              */
void ivec_decr( ivec v, int a );           /* substract a to the components        */
void ivec_mul_by( ivec v, int a );         /* multiply the components per a        */
void ivec_div_by( ivec v, int a );         /* divide the components per a          */

void cvec_incr( cvec v, cplx a );          /* add a to the components              */
void cvec_decr( cvec v, cplx a );          /* substract a to the components        */
void cvec_mul_by( cvec v, cplx a );        /* multiply the components per a        */
void cvec_div_by( cvec v, cplx a );        /* divide the components per a          */

void cvec_incr_real( cvec v, double a );   /* add a to the components              */
void cvec_decr_real( cvec v, double a );   /* substract a to the components        */
void cvec_mul_by_real( cvec v, double a ); /* multiply the components per a        */
void cvec_div_by_real( cvec v, double a ); /* divide the components per a          */

void cvec_conj( cvec v );                  /* Conjugate                            */

/* Components per components operations (vectors must be of same size)          */
void vec_add( vec v1, vec v2 );      
void vec_sub( vec v1, vec v2 );
void vec_mul( vec v1, vec v2 );
void vec_div( vec v1, vec v2 );
     
void ivec_add( ivec v1, ivec v2 );      
void ivec_sub( ivec v1, ivec v2 );
void ivec_mul( ivec v1, ivec v2 );
void ivec_div( ivec v1, ivec v2 );

vec  vec_new_add( vec v1, vec v2 );      
vec  vec_new_sub( vec v1, vec v2 );
vec  vec_new_mul( vec v1, vec v2 );
vec  vec_new_div( vec v1, vec v2 );

ivec ivec_new_add( ivec v1, ivec v2 );      
ivec ivec_new_sub( ivec v1, ivec v2 );
ivec ivec_new_mul( ivec v1, ivec v2 );
ivec ivec_new_div( ivec v1, ivec v2 );

void cvec_add( cvec v1, cvec v2 );      
void cvec_sub( cvec v1, cvec v2 );
void cvec_mul( cvec v1, cvec v2 );
void cvec_div( cvec v1, cvec v2 );

/* The inner product of two vectors                                             */
double vec_inner_product( vec v1, vec v2 );   
int ivec_inner_product( ivec v1, ivec v2 );   

/* Common functions                                                             */
void vec_neg( vec v );                  /* Negate the vector                    */
void ivec_neg( ivec v );
void cvec_neg( cvec v );

void vec_sqr( vec v );   
void ivec_sqr( ivec v );   

void vec_sqrt( vec v );  
void vec_log( vec v );  
void vec_log10( vec v );  
void vec_exp( vec v );                  /* Apply exponential                     */

void vec_abs( vec v );                  /* Apply absolute value                  */
void ivec_abs( ivec v );

vec  cvec_new_abs( cvec v );

void vec_pow( vec v, double a );        /* set each component c to c^a           */
void vec_normalize( vec v, double nr ); /* so that sum(||v||_n)=1                */

/* Useful operations                                                            */
double vec_sum( vec v );          /* The sum of all the elements          */
int    ivec_sum( ivec v );           /* use with care: may easily under/overflow */
cplx   cvec_sum( cvec v );          /* The sum of all the elements          */

vec  vec_cum_sum( vec v );      
ivec ivec_cum_sum( ivec v );   
cvec cvec_cum_sum( cvec v );   

double vec_sum_sqr( vec v );      /* The energy of the vector             */

double vec_sum_between( vec v, idx_t i1, idx_t i2 );
int    ivec_sum_between( ivec v, idx_t i1, idx_t i2 );
cplx   cvec_sum_between( cvec v, idx_t i1, idx_t i2 );

double vec_min( vec v );
int    ivec_min( ivec v );

double vec_max( vec v );
int    ivec_max( ivec v );

idx_t vec_min_index( vec v );
idx_t ivec_min_index( ivec v );

idx_t vec_max_index( vec v );
idx_t ivec_max_index( ivec v );

double vec_mean( vec v );
double ivec_mean( ivec v );

double vec_median( vec v );       
int    ivec_median( ivec v ); 

double vec_variance( vec v );     /* Unbiaised variance (divide by N-1)   */
double vec_norm( vec v, double nr );   /* compute the norm a of the vector     */


/* General function                                                */
vec vec_apply_function( vec v, it_function_t function, it_args_t args ); 
vec vec_new_apply_function( vec v, it_function_t function, it_args_t args );
#define vec_eval(v, f, a) vec_apply_function(v, f, a)
#define vec_new_eval(v, f, a) vec_new_apply_function(v, f, a)

ivec ivec_apply_function( ivec v, it_ifunction_t function, it_args_t args );
ivec ivec_new_apply_function( ivec v, it_ifunction_t function, it_args_t args );
#define ivec_eval(v, f, a) ivec_apply_function(v, f, a)
#define ivec_new_eval(v, f, a) ivec_new_apply_function(v, f, a)

 
/*-----------------------------------------------------------------*/
/* Set, sorting and finding functions                              */
/*-----------------------------------------------------------------*/

/* Reverse the vector                                              */
void vec_reverse( vec v );
void ivec_reverse( ivec v );
void bvec_reverse( bvec v );

vec vec_new_reverse( vec v );
ivec ivec_new_reverse( ivec v );
bvec bvec_new_reverse( bvec v );
cvec cvec_new_reverse( cvec v );

/* Number of elements equal to value a */
idx_t vec_count( vec v, double a );
idx_t ivec_count( ivec v, int a );
idx_t bvec_count( bvec v, byte a );
idx_t cvec_count( cvec v, cplx a );

/* Return the first position where the value a occurs, NULL_INDEX 
   if the research is not successful                               */
idx_t vec_find_first( vec v, double a );
idx_t ivec_find_first( ivec v, int a );
idx_t bvec_find_first( bvec v, byte a );
idx_t cvec_find_first( cvec v, cplx a );

/* Return the set of positions of value a                                       */
ivec vec_find( vec v, double a );
ivec ivec_find( ivec v, int a );
ivec bvec_find( bvec v, byte a );
ivec cvec_find( cvec v, cplx a );

/* Replace the set of values equal to a by b and return the set of positions 
   for which the modification has been processed.                               */
ivec vec_replace( vec v, double a, double b );
ivec ivec_replace( ivec v, int a, int b );
ivec bvec_replace( bvec v, byte a, byte b );
ivec cvec_replace( cvec v, cplx a, cplx b );


/* Set operations !!! Some of these operations are currently not implemented    */
static inline vec  vec_del( vec v, idx_t pos )   { Vec_del( v, pos ); return v;}
static inline ivec ivec_del( ivec v, idx_t pos ) { Vec_del( v, pos ); return v;}
static inline bvec bvec_del( bvec v, idx_t pos ) { Vec_del( v, pos ); return v;}
static inline cvec cvec_del( cvec v, idx_t pos ) { Vec_del( v, pos ); return v;}

/* To be used as v = ivec_ins( v, pos, elt ) */
static inline vec  _vec_ins( vec v, idx_t pos, double elt ) { Vec_ins( v, pos, elt ); return v; }
static inline ivec _ivec_ins( ivec v, idx_t pos, int elt ) { Vec_ins( v, pos, elt ); return v;  }
static inline bvec _bvec_ins( bvec v, idx_t pos, byte elt ) { Vec_ins( v, pos, elt ); return v;  }
static inline cvec _cvec_ins( cvec v, idx_t pos, cplx elt ) { Vec_ins( v, pos, elt ); return v;  }

#define vec_ins( v, pos, elt )  do { v=_vec_ins( v, pos, elt ); } while (0)
#define ivec_ins( v, pos, elt ) do { v=_ivec_ins( v, pos, elt ); } while (0)
#define bvec_ins( v, pos, elt ) do { v=_bvec_ins( v, pos, elt ); } while (0)
#define cvec_ins( v, pos, elt ) do { v=_cvec_ins( v, pos, elt ); } while (0)

/* Concatenation of two vectors */
vec  vec_concat( vec v1, vec v2 );
ivec ivec_concat( ivec v1, ivec v2 );
bvec bvec_concat( bvec v1, bvec v2 );
cvec cvec_concat( cvec v1, cvec v2 );

/* The following set operations return some vectors which are composed 
   of distinct and sorted elements.  */

vec  vec_unique( vec v );
ivec ivec_unique( ivec v );
bvec bvec_unique( bvec v );
cvec cvec_unique( cvec v );

vec  vec_union( vec v1, vec v2 );
ivec ivec_union( ivec v1, ivec v2 );
bvec bvec_union( bvec v1, bvec v2 );
cvec cvec_union( cvec v1, cvec v2 );

vec  vec_intersection( vec v1, vec v2 );
ivec ivec_intersection( ivec v1, ivec v2 );
bvec bvec_intersection( bvec v1, bvec v2 );
cvec cvec_intersection( cvec v1, cvec v2 );


/* Stack operations: Vector may be used as a stack                              */
static inline vec  _vec_push( vec v, double elt ) { vec_ins(v, vec_length(v), elt); return v;}
static inline ivec _ivec_push( ivec v, int elt )  { ivec_ins(v, ivec_length(v), elt); return v;}
static inline bvec _bvec_push( bvec v, byte elt ) { bvec_ins(v, bvec_length(v), elt); return v;}
static inline cvec _cvec_push( cvec v, cplx elt ) { cvec_ins(v, cvec_length(v), elt); return v;}

#define vec_push( v, elt )  do { v=_vec_push( v, elt ); } while (0)
#define ivec_push( v, elt ) do { v=_ivec_push( v, elt ); } while (0)
#define bvec_push( v, elt ) do { v=_bvec_push( v, elt ); } while (0)
#define cvec_push( v, elt ) do { v=_cvec_push( v, elt ); } while (0)

static inline vec  vec_pop( vec v )   { Vec_pop(v); return v;}
static inline ivec ivec_pop( ivec v ) { Vec_pop(v); return v;}	 
static inline bvec bvec_pop( bvec v ) { Vec_pop(v); return v;}	 
static inline cvec cvec_pop( cvec v ) { Vec_pop(v); return v;}

static inline double vec_head( vec v )    { return Vec_head(v); }
static inline int ivec_head( ivec v )     { return Vec_head(v); }
static inline byte bvec_head( bvec v )    { return Vec_head(v); }
static inline cplx cvec_head( const cvec v ) { return Vec_head(v); }

/* Return the vector composed of the elements of v indexed by idx               */
vec  vec_index_by( vec v, ivec idx );
ivec ivec_index_by( ivec v, ivec idx );
bvec bvec_index_by( bvec v, ivec idx );
cvec cvec_index_by( cvec v, ivec idx );

/* Sort the element of the vector (based on the qsort algorithm of stdlib)      */
void Vec_qsort(Vec v, int (* elem_leq)(const void *, const void *));
void vec_qsort( vec v );
void ivec_qsort( ivec v );
void bvec_qsort( bvec v );

/* Return a vector of index corresponding to increasing values of the vector v  */
ivec Vec_qsort_index(Vec v, int (* elem_leq_idx)(const void *, const void *));
ivec vec_qsort_index( vec v );
ivec ivec_qsort_index( ivec v );
ivec bvec_qsort_index( bvec v );

#define Vec_sort( v, f ) ( Vec_qsort( v, f ) )
#define vec_sort( v ) ( vec_qsort( v ) )
#define ivec_sort( v ) ( ivec_qsort( v ) )
#define bvec_sort( v ) ( bvec_qsort( v ) )

#define Vec_sort_index( v, f ) ( Vec_qsort_index( v, f ) )
#define vec_sort_index( v ) ( vec_qsort_index( v ) )
#define ivec_sort_index( v ) ( ivec_qsort_index( v ) )
#define bvec_sort_index( v ) ( bvec_qsort_index( v ) )


/*-----------------------------------------------------------------*/
/* Special vectors                                                 */
/*-----------------------------------------------------------------*/

/* The following functions modify already allocated vectors        */

#define Vec_void( v ) do {                                                \
  assert( v );                                                            \
  Vec_length( v ) = 0;                                                    \
} while(0)

void vec_void( vec );
void ivec_void( ivec );
void bvec_void( bvec );
void cvec_void( cvec );
 
void vec_zeros( vec );
void ivec_zeros( ivec );
void bvec_zeros( bvec );
void cvec_zeros( cvec );

void vec_ones( vec );
void ivec_ones( ivec );
void bvec_ones( bvec );
void cvec_ones( cvec );

void vec_range( vec );
void ivec_range( ivec );
void bvec_range( bvec );
void cvec_range( cvec );

void vec_1N( vec );
void ivec_1N( ivec );
void bvec_1N( bvec );
void cvec_1N( cvec );

void vec_arithm( vec, double start, double incr );
void ivec_arithm( ivec, int start, int incr );
void bvec_arithm( bvec, byte start, byte incr );
void cvec_arithm( cvec, cplx start, cplx incr );

void vec_geom( vec, double start, double r );
void ivec_geom( ivec, int start, int r );
void bvec_geom( bvec, byte start, byte r );
void cvec_geom( cvec, cplx start, cplx r );

/* Same functions, but which allocate new vectors */

#define Vec_new_void( type_t ) Vec_new( type_t, 0 )

vec  vec_new_void();
ivec ivec_new_void();
bvec bvec_new_void();
cvec cvec_new_void();

/* Constant vector */
vec  vec_new_set( double val, idx_t N );
ivec ivec_new_set( int val, idx_t N );
bvec bvec_new_set( byte val, idx_t N );
cvec cvec_new_set( cplx val, idx_t N );

/* Vectors of zeros */
static inline vec  vec_new_zeros( idx_t N ) { return(vec_new_set( 0., N )); }
static inline ivec ivec_new_zeros( idx_t N ) { return(ivec_new_set( 0, N )); }
static inline bvec bvec_new_zeros( idx_t N ) { return(bvec_new_set( 0, N )); }
static inline cvec cvec_new_zeros( idx_t N ) { return(cvec_new_set( cplx_0, N )); }

/* Vectors of ones */
static inline vec  vec_new_ones( idx_t N ) { return(vec_new_set( 1., N )); }
static inline ivec ivec_new_ones( idx_t N ) { return(ivec_new_set( 1, N )); }
static inline bvec bvec_new_ones( idx_t N ) { return(bvec_new_set( 1, N )); }
static inline cvec cvec_new_ones( idx_t N ) { return(cvec_new_set( cplx_1, N )); }

/* The sequence 0, 1, ... N-1 */
vec  vec_new_range( idx_t N );
ivec ivec_new_range( idx_t N );
bvec bvec_new_range( idx_t N );
cvec cvec_new_range( idx_t N );

/* The sequence 1, 2.. N */
vec  vec_new_1N( idx_t N );
ivec ivec_new_1N( idx_t N );
bvec bvec_new_1N( idx_t N );
cvec cvec_new_1N( idx_t N );

/* Arithmetic sequence starting at start, incremented by incr and containing N elements */
vec  vec_new_arithm( double start, double incr, idx_t N );
ivec ivec_new_arithm( int start, int incr, idx_t N );
bvec bvec_new_arithm( byte start, byte incr, idx_t N );
cvec cvec_new_arithm( cplx start, cplx incr, idx_t N );

/* Same but a geometric sequence of geometric factor r */
vec vec_new_geom( double start, double r, idx_t N );
ivec ivec_new_geom( int start, int r, idx_t N );
bvec bvec_new_geom( byte start, byte r, idx_t N );
cvec cvec_new_geom( cplx start, cplx r, idx_t N );

/* generate e^{2i\pi k / N} = cos(2 k \pi / N) + i sin(2 k \pi / N) */
cvec cvec_new_unit_roots(idx_t N);

/* Generate some random vectors */
void vec_rand( vec v );
void vec_randn( vec v );
vec vec_new_rand( idx_t N );
vec vec_new_randn( idx_t N );
ivec ivec_new_perm( size_t len, unsigned int seed ); 

/*-----------------------------------------------------------------*/
/* Special vectors                                                 */
/*-----------------------------------------------------------------*/

/* Convolution of two vectors */
vec vec_conv( vec v1, vec v2 );
ivec ivec_conv( ivec v1, ivec v2 );

#ifdef __cplusplus
}
#endif /* extern "C" */
#endif











