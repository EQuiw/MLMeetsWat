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
  Source definition functions
  Copyright (C) 2005-2006 Vivien Chappelier, Herve Jegou, François Cayre
*/


#ifndef __it_source_h
#define __it_source_h

#include "vec.h"

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------*
 * Source Random number generator                                     *
 *--------------------------------------------------------------------*/

/* generate a vector of random binary values with the given */
/* probability for the 0 symbol */
bvec source_binary( idx_t size, double p0 ); 

/* generate a vector of l-exponentially distributed values */ 
/* Exp pdf: l*exp(-l*x) x>=0 */
vec source_exp( idx_t size, double l );

/* generate a vector of Cauchy-distributed values */ 
/* Cauchy pdf: (1/pi)*b/((x-a)^2+b^2) */
vec source_cauchy( idx_t size, double a, double b );

/* generate a vector of Weibull-distributed values */
vec source_weibull( idx_t size, double alpha, double beta );

/* generate a vector of values uniformly distributed in [a,b[ */
vec source_uniform( idx_t size, double a, double b );

/* generate a vector of independent values drawn from a */
/* gaussian distribution of given mean and standard deviation */
vec source_gaussian( idx_t size, double mean, double std );

/* generate a stationnary random vector from a probability
   density function using the acceptance-rejection method.
   the pdf is assumed to be zero outside [a, b].
*/
vec source_pdf( idx_t size, double a, double b, it_function_t pdf, it_args_t args );

/* Return a vector of size K of real values between 0 and 1           */
#define source_uniform_01( K ) source_uniform( K, 0, 1 )

/* Memoryless discrete source of length size defined by its stationary probabilities pdf */
ivec source_memoryless( idx_t size, vec pdf );

#ifdef __cplusplus
}
#endif /* extern "C" */
#endif
