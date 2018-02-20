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
#include "math.h"
#include "random.h"

#include "constants.h"
#include "utils.h"


/*************************************
 *  Proportional embedding           *
 *  See also section IV of the paper *
 *************************************/

int
proportionalEmbed (vec v_W, unsigned int *alea, vec s_X_abs, vec s_W)
{
  int N_s = vec_length (s_W);
  int N_v = vec_length (v_W);
  double pos = 1 / sqrt (N_s);
  int i, j, k = 0;

  for (i = 0; i < N_v; i++)
    for (j = 0; j < N_s; j++)
      {
	if ((alea[k / 32] & (1 << (k % 32))) != 0)
	  {
	    s_W[j] += v_W[i];
	  }
	else
	  {
	    s_W[j] -= v_W[i];
	  }
	k++;
      }

  for (j = 0; j < N_s; j++)
    s_W[j] = s_W[j] * s_X_abs[j] * pos; /* Eq. (23) of the paper */

  return (1);

}

/*************************************
 *  Proportional embedding           *
 *  See also section III of the paper*
 *************************************/

int
constantEmbed (vec v_W, unsigned int *alea, vec s_W)
{
  int N_s = vec_length (s_W);
  int N_v = vec_length (v_W);
  int i, j, k = 0;
  double pos = 1 / sqrt (N_s);


  for (i = 0; i < N_v; i++)
    for (j = 0; j < N_s; j++)
      {
	if ((alea[k / 32] & (1 << (k % 32))) != 0)
	  {
	    s_W[j] += v_W[i];
	  }
	else
	  {
	    s_W[j] -= v_W[i];
	  }
	k++;
      }

  for (j = 0; j < N_s; j++)
    s_W[j] = s_W[j] * pos;

  return (1);
}


/*********************************************
 * Computation of the maximum robustness     *
 * by maximization of the nearest border     * 
 * point distance (see III.a.2 of the paper) *
 *********************************************/    
int
max_robustness (double c_X_1, double c_X_2, double mean_X_Abs,
		double *norm_s_W, double angle, double *c_W_1,
		double *c_W_2, double *robustness)
{
  double c_Y_1;
  double c_Y_2;
  double wE1, wE2;
  double rho = (*norm_s_W) * mean_X_Abs;

  wE1 = rho * sin(angle);
  wE2 = -1* rho * cos(angle);
  c_Y_1 = c_X_1 + wE1; /* Eq (13) of the paper */
  c_Y_2 = c_X_2 + wE2; /* Eq (13) of the paper */

  *robustness = (c_Y_1*sin(angle)-c_Y_2*cos(angle))*(c_Y_1*sin(angle)-c_Y_2*cos(angle)); /* cf Eq (12) of the paper */

  if (c_Y_2 < 0){
    c_Y_1 = c_X_1 + sqrt(rho*rho-c_X_2*c_X_2); /* Eq (10) of the paper */
    c_Y_2 = 0; /* Eq (10) of the paper */
    *robustness = c_Y_1*c_Y_1*sin(angle)*sin(angle); /* Eq (11) of the paper */
    wE1 = c_Y_1 - c_X_1;
    wE2 = c_Y_2 - c_X_2;
  }

  /*********************************************************
   *   Computation of the coordinates to be projected back *
   *   to the secret subspace(see IV.a.2 of the paper)     *
   *********************************************************/

  *c_W_1 = wE1 / mean_X_Abs;
  *c_W_2 = wE2 / mean_X_Abs;

  return (1);

}


/******************************************
 * PSNR between 2 gray level images       *
 ******************************************/

double
mat_psnr (mat x, mat y)
{

  unsigned int i = 0, j = 0;
  double psnr = 0., mean = 0.;

  mat d = NULL;

  d = mat_clone (x);

  for (i = 0; i < mat_height (x); i++)
    for (j = 0; j < mat_width (x); j++)
      d[i][j] -= y[i][j];

  mean = mat_mean (d);


  for (i = 0; i < mat_height (x); i++)
    for (j = 0; j < mat_width (x); j++)
      psnr += d[i][j] * d[i][j];

  mat_delete (d);

  return (10. *
	  log10 (255. * 255. /
		 (psnr / (double) (mat_height (x) * mat_width (x)))));

}
