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
A C-program for MT19937, with initialization improved 2002/2/10.
Coded by Takuji Nishimura and Makoto Matsumoto.
This is a faster version by taking Shawn Cokus's optimization,
Matthe Bellew's simplification, Isaku Wada's real version.

Before using, initialize the state by using init_genrand(seed) 
or init_by_array(init_key, key_length).

Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
All rights reserved.                          

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. The names of its contributors may not be used to endorse or promote 
products derived from this software without specific prior written 
permission.

The original code is located at: 
http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/MT2002/CODES/MTARCOK/mt19937ar-cok.c 

It was licensed under the terms of the BSD license. 

*/

/*
  Random number generator
  Copyright (C) 2005 Vivien Chappelier
  Copyright (C) 2006 François Cayre 
*/

#ifndef __it_random_h
#define __it_random_h

#include "vec.h"
#include "mat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* MT19937cok-ar related functions: they are here for providing   */ 
/* additional features to whoever might need it                   */
void mt19937_srand(unsigned int seed); 
void mt19937_srand_by_array(unsigned int init_key[], unsigned int key_length);
/* generates a random number on [0,0xffffffff]-interval */
unsigned int mt19937_rand_int32(void); 
/* generates a random number on [0,0x7fffffff]-interval */
int mt19937_rand_int31(void);
/* generates a random number on [0,1]-real-interval */
double mt19937_rand_real1(void);
/* generates a random number on [0,1)-real-interval */
double mt19937_rand_real2(void);
/* generates a random number on (0,1)-real-interval */
double mt19937_rand_real3(void);
/* generates a random number on [0,1) with 53-bit resolution*/
double mt19937_rand_res53(void);


/* initialize the random number generator (with a random seed)    */
/* Note: the seed is taken from the milliseconds of the current   */
/* time, which is not a serious option for security applications. */
/* In this case, always use it_seed with your favorite method     */
/* to obtain a good seed.                                         */
void it_randomize(void);

/* intializes the random generator from a seed */
void it_seed(int seed);

/* generate a value uniformly distributed in [0,1[ */
double it_rand(void);

/* generate a value distributed normally */
double it_randn(void);

/* generate a random variable from its probability
   density function using the acceptance-rejection method.
   the pdf is assumed to be zero outside [a, b].
*/
double it_randpdf(double a, double b, it_function_t pdf, it_args_t args);


/* Random variable that follows a memoryless pdf */
int it_rand_memoryless( vec pdf );

#ifdef __cplusplus
}
#endif /* extern "C" */
#endif
