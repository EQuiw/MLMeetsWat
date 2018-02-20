/*
Adapted version to get wavelet coefficients of broken arrows file embed.c
*/

#include<stdio.h>
#include<string.h>
#include "mat.h"
#include "vec.h"
#include "parser.h"
#include "separable2D.h"
#include "wavelet.h"
#include "transform2D.h"
#include "wavelet2D.h"
#include "io.h"
#include "random.h"
#include "distance.h"

#include "extract.h"
#include "project.h"
#include "utils.h"
#include "constants.h"

#include <Rcpp.h>
#include <stdlib.h>
#include "rcppMyUtils.h"
using namespace std;

RcppExport SEXP getWaveletCoefficients(
		SEXP inputimageassexp
		) {

	// try-catch-macros from RCPP: BEGIN_RCPP and END_RCPP
	BEGIN_RCPP

  /* Pixel space */
  unsigned int h_I, w_I;	/* height and width of the image in pixels */
  mat I_X = NULL;           /* Host image */

  /* Wavelet domain */
  it_wavelet2D_t *wavelet2D = NULL;	/* this defines a wavelet transform in libit*/

  int levels;			/* Levels of the wavelet decomposition */
  mat Wav_X = NULL;	/* Wavelet decomposition of I_X */

  /* Wavelet subspace */
  int N_s;			/* Dimension of the wavelet subspace */
  vec s_X = NULL;		/* Host sequence (high frequencies wavelet coeff) */

  int N_s_LL;			/* Dimension of the low low band sequence */
  vec s_X_LL = NULL;		/* Host low low band (low frequencies wavelet coeff) */

  /* Arguments */
  vector<float> llcoeff, hfcoeff;

  /* Parameters Initialisation */
  levels=3;

  // Convert the input image from R to an C++ vector and then convert it to a mat object, used by Broken Arrows
  I_X = convertSexpIntoMat(inputimageassexp);

  /*************************************************
   * Read the pgm image and obtain an Image I_X    *
   *************************************************/

   // I_X = mat_pgm_read (inputFile); // not necessary anymore.
   h_I = mat_height (I_X);
   w_I = mat_width (I_X);

   /***************************************************
    *  Wavelet decomposition of I_X to obtain Wav_X   *
    ***************************************************/

   wavelet2D = it_wavelet2D_new (it_wavelet_lifting_97, levels+1);
   Wav_X = it_wavelet2D_transform (wavelet2D, I_X);

   /***************************************************
    *  Host sequence extraction s_X                   *
    *  Host core signal extraction s_X_LL             *
    *  (see also section II.b of the paper)           *
    ***************************************************/

   N_s_LL = h_I * w_I / ((int) pow (2, (2 * levels)));	/* Number of remaining untouched coefficients */

   N_s = h_I * w_I - N_s_LL;	/* Dimension of the wavelet space */

   s_X = vec_new_zeros (N_s);	/* This sequence gathers the wavelet coeff that will bear the watermark signal */
   s_X_LL = vec_new_zeros (N_s_LL);	/* This sequence gathers the remaining lowlow freq subband ones. */

   extract (Wav_X, s_X_LL, s_X, levels);	/* fills s_X_LL and S_X */

  //s_X_abs = vec_clone (s_X);
  //vec_abs (s_X_abs);		/* Absolute values of the wavelet coefficient */

  /* Some statistics to predict the distortion */
  //var_X = vec_variance (s_X);
  //mean_X_Abs = vec_mean (s_X_abs);

  llcoeff.resize(N_s_LL);
  hfcoeff.resize(N_s);
  for(int i=0;i<N_s_LL;i++) {
    llcoeff[i] = s_X_LL[i];
  }
  for(int i=0;i<N_s;i++) {
    hfcoeff[i] = s_X[i];
  }
  /***********************
   *  Leave things clean  *
   ***********************/

   mat_delete (I_X);

  it_delete (wavelet2D);
  mat_delete (Wav_X);

  vec_delete (s_X);
  vec_delete (s_X_LL);

  return Rcpp::List::create(Rcpp::Named("llcoeffs") = llcoeff,
                            Rcpp::Named("hfcoeffs") = hfcoeff);

  END_RCPP
}

