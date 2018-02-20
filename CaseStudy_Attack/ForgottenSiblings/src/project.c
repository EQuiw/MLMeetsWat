/*
Copyright or © or Copr.:  CNRS and INRIA
name of the program: Broken Arrows
date: December 2007
version: 1.0
contributor(s):
Teddy Furon (INRIA) - teddy(dot)furon(at)inria(dot)fr
Patrick Bas (CNRS) - patrick(dot)bas(at)inpg(dot)fr

This software is a computer program whose purpose is to embed and detect watermark
in still pgm images.

This software is governed by the CeCILL license under French law and
abiding by the rules of distribution of free software.  You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.

*/


#include "vec.h"
#include "mat.h"
#include "random.h"

#include "constants.h"
#include "project.h"


/**********************************************************************
 *  Transform mapping from the wavelet space to the secret subspace   *
 *  secret carriers are antipodal and normalized: +/- 1/sqrt(N_s)     *
 **********************************************************************/
int
projectSubspace (vec s_X, unsigned int *alea, vec v_X)
{
  int N_s = vec_length (s_X);	/* Dimension of the Wavelet space       */
  int N_v = vec_length (v_X);	/* Dimension of the secret subspace     */
  int i, j;
  long k = 0;
  double pos = 1 / sqrt (N_s);

  for (i = 0; i < N_v; i++)
    {
      for (j = 0; j < N_s; j++)
	{
	  /* one alea number is used 32 times, generating 32 carriers binary samples */
	  if ((alea[k / 32] & (1 << (k % 32))) != 0)
	    {
	      v_X[i] += s_X[j];
	    }
	  else
	    {
	      v_X[i] -= s_X[j];
	    }
	  k++;
	}
      v_X[i] *= pos;
    }

  return (1);
}

/*****************************************************************
 * Transform mapping from the secret subspace to the MCB plan    *
 * See also section II.d of the paper                            *
 *****************************************************************/
int
projectMCB (vec v_X, vec v_C, vec v_e1, vec v_e2, double *c_X_1, double *c_X_2)
{
  /* v_X: vector in the correlation space */
  /* v_C: secret vector in the correlation space */
  double normE1, normE2;

  vec_copy (v_e1, v_C);		/* First element of the basis of the MCB plan, Eq (3) of the paper */
  normE1 = vec_norm (v_e1, 2);
  vec_div_by (v_e1, normE1);	/* normalized */

  *c_X_1 = vec_inner_product (v_X, v_e1);	// projection of v_X on v_e1

  /* Graham-Schmidt orthonomalization */
  /* Eq (3) of the paper */

  /* v_e2 = v_X - c_X_1 * v_e1*/
  vec_copy (v_e2, v_e1);
  vec_mul_by (v_e2, *c_X_1);
  vec_sub (v_e2, v_X);
  vec_neg (v_e2);

  normE2 = vec_norm (v_e2, 2);
  vec_div_by (v_e2, normE2);	/* normalized it */

  *c_X_2 = vec_inner_product (v_X, v_e2);	/* projection of v_X on v_e2 */

  return (1);
}
