/*
Adapted version to get inverse wavelet version of broken arrows file embed.c
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
using namespace std;

RcppExport SEXP revertWaveletCoefficients(
    SEXP llcoeffssexp,
    SEXP hfcoeffssexp,
    SEXP imagesizeheight,
    SEXP imagesizewidth,
    SEXP verbosesexp
) {

  // try-catch-macros from RCPP: BEGIN_RCPP and END_RCPP
  BEGIN_RCPP

  /* Pixel space */
  unsigned int h_I, w_I;	/* height and width of the image in pixels */
  mat I_Y = NULL;	                /* Watermarked image */

  const char *inputFileLL = NULL;
  const char *inputFileHH = NULL;	/* the name of the two files that contain the wavelet coeff. */

  /* Wavelet domain */
  it_wavelet2D_t *wavelet2D = NULL;	/* this defines a wavelet transform in libit*/

  int levels;			/* Levels of the wavelet decomposition */
  mat Wav_Y = NULL;		/* Wavelet decomposition of I_Y */

  /* Wavelet subspace */
  int N_s;			/* Dimension of the wavelet subspace */
  vec s_X = NULL;		/* Host sequence (high frequencies wavelet coeff) */
  //  s_Y = NULL;			/* sequence of the watermarked content */

  int N_s_LL;			/* Dimension of the low low band sequence */
  vec s_X_LL = NULL;		/* Host low low band (low frequencies wavelet coeff) */

  //long i;                       /* increment */
  int verbose;			/* Flag to indicate if we are in verbose mode */


  /****************************************************/
  /* Parameters Initialisation */
  levels=3;
  verbose = Rcpp::as<int>(verbosesexp);
  h_I = Rcpp::as<int>(imagesizeheight);
  w_I = Rcpp::as<int>(imagesizewidth);

  N_s_LL = h_I * w_I / ((int) pow (2, (2 * levels)));	/* Number of remaining untouched coefficients */
  // if (verbose == 1) printf("%d\n",N_s_LL);
  N_s = h_I * w_I - N_s_LL;	/* Dimension of the wavelet space */
  // if (verbose == 1) printf("%d\n",N_s);

  s_X = vec_new_zeros (N_s);
  s_X_LL = vec_new_zeros (N_s_LL);

  Rcpp::NumericVector llcoeffs(llcoeffssexp);
  Rcpp::NumericVector hfcoeffs(hfcoeffssexp);

  Rcpp::NumericMatrix finalimage (h_I, w_I);
  /****************************************************/
  /* read LL & HF input */
  for(int i=0;i<N_s_LL;i++) {
    s_X_LL[i] = llcoeffs[i];
  }
  for(int i=0;i<N_s;i++) {
    s_X[i] = hfcoeffs[i];
  }

  /***************************************************
  *  Map back to the wavelet space                   *
  ****************************************************/
  wavelet2D = it_wavelet2D_new (it_wavelet_lifting_97, levels+1);
  Wav_Y = mat_new_zeros (h_I, w_I);
  extractInv (s_X_LL, s_X, levels, h_I, w_I, Wav_Y);

  /***************************************************
  *  Map back to the spatial image -                 *
  *  by an inverse wavelet transform                 *
  ***************************************************/
  /* Inverse wavelet decomposition of watermarked image */
  I_Y = it_wavelet2D_itransform (wavelet2D, Wav_Y);

  /* Writing */
  // mat_pgm_write (argv[5], I_Y);
  for( int i = 0 ; i < h_I ; i++ )
    for( int j = 0 ; j < w_I ; j++ )
      finalimage(i,j) = I_Y[i][j];

  /* Cleaning */
  it_delete (wavelet2D);
  mat_delete (Wav_Y);
  mat_delete (I_Y);

  vec_delete (s_X);

  vec_delete (s_X_LL);


  return finalimage;
  // return Rcpp::List::create(Rcpp::Named("llcoeffs") = llcoeff,
  //                           Rcpp::Named("hfcoeffs") = hfcoeff);

  END_RCPP
}
