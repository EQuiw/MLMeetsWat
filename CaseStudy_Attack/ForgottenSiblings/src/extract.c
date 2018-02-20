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
#include "wavelet2D.h"

#include "extract.h"


int
extract (mat Wav_X, vec s_X_LL, vec s_X, int levels)
{
  int i, j, k, l, m;

  /* Image subbands */
  mat *subx = it_wavelet2D_split (Wav_X, levels);

  k = 0;
  for (l = levels; l > 0; l--)
    for (m = 2; m >= 0; m--)
      for (i = 0; i < mat_height (subx[l * 3 - m]); i++)
	for (j = 0; j < mat_width (subx[l * 3 - m]); j++)
	  {
	    s_X[k] = subx[l * 3 - m][i][j];
	    k++;
	  }
  k = 0;

  for (i = 0; i < mat_height (subx[0]); i++)
    for (j = 0; j < mat_width (subx[0]); j++)
      {
	s_X_LL[k] = subx[0][i][j];
	k++;
      }

  /* Cleaning */
  for (l = levels; l > 0; l--)
    {
      mat_delete (subx[3 * l - 2]);
      mat_delete (subx[3 * l - 1]);
      mat_delete (subx[3 * l]);
    }
  mat_delete (subx[0]);
  free(subx);
  return (1);
}

int
extractInv (vec s_X_LL, vec s_X, int levels, int h_I, int w_I, mat Mwav)
{
  int i, j, k, l, m;
  mat *subx = NULL;
  mat MwavRet;
  int mid_row, mid_col;

  /* Image subbands */
  subx = (mat *) malloc (sizeof (mat) * (3 * levels + 1));
  mid_row = (w_I) / 2;
  mid_col = (h_I) / 2;

  for (l = levels; l > 0; l--)
    {
      subx[l * 3 - 2] = mat_new_zeros (mid_col, mid_row);
      subx[l * 3 - 1] = mat_new_zeros (mid_col, mid_row);
      subx[l * 3] = mat_new_zeros (mid_col, mid_row);
      h_I = mid_row;
      w_I = mid_col;
      mid_row = (h_I) / 2;
      mid_col = (w_I) / 2;
    }

  subx[0] = mat_new_zeros (w_I, h_I);

  k = 0;
  for (l = levels; l > 0; l--)
    for (m = 2; m >= 0; m--)
      for (i = 0; i < mat_height (subx[l * 3 - m]); i++)
	for (j = 0; j < mat_width (subx[l * 3 - m]); j++)
	  {
	    subx[l * 3 - m][i][j] = s_X[k];
	    k++;
	  }

  k = 0;
  for (i = 0; i < mat_height (subx[0]); i++)
    for (j = 0; j < mat_width (subx[0]); j++)
      {
	subx[0][i][j] = s_X_LL[k];
	k++;
      }

  /* Merge subx */
  MwavRet = it_wavelet2D_merge (subx, levels);

  mat_copy (Mwav, MwavRet);

  /* Cleaning */
  mat_delete (MwavRet);
  for (l = levels; l > 0; l--)
    {
      mat_delete (subx[3 * l - 2]);
      mat_delete (subx[3 * l - 1]);
      mat_delete (subx[3 * l]);
    }
  mat_delete (subx[0]);
  free(subx);

  return (1);
}

int
extractBF (vec s_X_LL, vec V_BF)
{
  int N_s_LL = vec_length (s_X_LL);
  int i;

  for (i = 0; i < 3 * N_s_LL / 4; i++)
    V_BF[i] = s_X_LL[i + N_s_LL / 4];

  return (1);
}
