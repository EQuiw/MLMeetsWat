/*
Copyright or ? or Copr.:  CNRS and INRIA
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

RcppExport SEXP embedWatermark(
    SEXP inputimageassexp,
    SEXP verbosesexp,
    SEXP watermarkkeysexp
) {
  // try-catch-macros from RCPP: BEGIN_RCPP and END_RCPP
  BEGIN_RCPP

  /* Pixel space */
  unsigned int h_I, w_I;	/* height and width of the image in pixels */
  mat I_X = NULL,		/* Host image */
    I_Y = NULL;	                /* Watermarked image */

  /* Wavelet domain */
  it_wavelet2D_t *wavelet2D = NULL;	/* this defines a wavelet transform in libit*/

  int levels;			/* Levels of the wavelet decomposition */
  mat Wav_X = NULL,		/* Wavelet decomposition of I_X */
    Wav_Y = NULL;		/* Wavelet decomposition of I_Y */

  /* Wavelet subspace */
  int N_s;			/* Dimension of the wavelet subspace */
  vec s_X = NULL,		/* Host sequence (high frequencies wavelet coeff) */
    s_X_abs = NULL,		/* Absolute values of s_X */
    s_W = NULL,			/* sequence of the watermark signal */
    s_Y = NULL;			/* sequence of the watermarked content */

  int N_s_LL;			/* Dimension of the low low band sequence */
  vec s_X_LL = NULL;		/* Host low low band (low frequencies wavelet coeff) */

  /* Secret subspace */
  int N_v;			/* dimension of the secret subspace */
  vec v_X = NULL,		/* secret vector for the host */
    v_X_Abs = NULL,             /* Absolute Value for the host */
    v_C = NULL,			/* carrier/cone vector */
    v_W = NULL,			/* correlation vector of the watermark signal */
    v_e1 = NULL,	        /* vector of the first element of the basis of the MCB plan */
    v_e2 = NULL;		/* vector of the second element of the basis of the MCB plan */

  /* MCB */
  double c_X_1, c_X_2;		/* First and second coordiantes of the host */
  double c_W_1, c_W_2;		/* coordinates of the watermark signal */
  double c_Y_1, c_Y_2;		/* coordinates of the watermaked content */

  /* Misc */
  vec Vtmp;
  int coneIdx;			/* Index of the best cone */
  double angle;			/* angle of the cone */
  double var_X,                 /* variance of the host vector */
    mean_X_Abs,                 /* mean of the absolute value of the host vector */
    norm_s_W;                   /* norm of the watermark vector in the secret subspace*/
  double robustness;            /* Robustness value */


  long i;                       /* increment */
  long sizeAlea;                /* Number of alea drawn by the PRNG */
  unsigned int *alea;           /* 128 bits long key */

  /* Arguments */
  char *inputFile ;		/* Image to be watermarked */

  unsigned int key[4];          /* 128 bits long key */
  unsigned int key1;		/* key used for embedding or detection - belongs to 0 - 2^31. Change sign to use the 32nd bit */
  unsigned int key2;		/* key used for embedding or detection - belongs to 0 - 2^31. Change sign to use the 32nd bit */
  unsigned int key3;		/* key used for embedding or detection - belongs to 0 - 2^31. Change sign to use the 32nd bit */
  unsigned int key4;		/* key used for embedding or detection - belongs to 0 - 2^31. Change sign to use the 32nd bit */
  double psnr;			/* peak signal to noise power ratio in dB - Embedding distortion constraint */
  int N_c;			/* N_c : nb of possible cones */
  int ProportionalFlag;			/* Flag to indicate if the embedding is done using a proprotional or constant embedding */
  int verbose;			/* Flag to indicate if we are in verbose mode */

  std::vector<float> llcoeff_unwat, hfcoeff_wat, hfcoeff_unwat;

  /* Parameters Initialisation */
  levels=3;
  psnr=43;
  N_v=256;
  N_c=30;
  angle = 1.2154; /* pfa = 3.10-7 for 30 cones */
  ProportionalFlag=1;


  /* Load the key */
  Rcpp::NumericVector watermarkkey(watermarkkeysexp);
  key1= watermarkkey[0];
  key2= watermarkkey[1];
  key3= watermarkkey[2];
  key4= watermarkkey[3];

  key[0] = key1;
  key[1] = key2;
  key[2] = key3;
  key[3] = key4;


  /***************************************************
   *  Log files creation                             *
   ***************************************************/
  // FILE *filePSNR = fopen ("OutFiles/filePSNR.dat", "a+");
  // FILE *fileC_X = fopen ("OutFiles/fileC_X.dat", "a+");
  // FILE *fileC_Y = fopen ("OutFiles/fileC_Y.dat", "a+");
  // FILE *fileRob = fopen ("OutFiles/fileRob.dat", "a+");
  // FILE *fileNorm_S_WRho = fopen ("OutFiles/fileNorm_S_WRho.dat", "a+");
  // FILE *fileV_x = fopen ("OutFiles/fileV_x.dat", "a+");
  // FILE *file_mean_X_Abs = fopen("OutFiles/file_mean_X_Abs.dat" , "a+");

  /*************************************************
   * Read the pgm image and obtain an Image I_X    *
   *************************************************/

  verbose = Rcpp::as<int>(verbosesexp);

  // Convert the input image from R to an C++ vector and then convert it to a mat object, used by Broken Arrows
  I_X = convertSexpIntoMat(inputimageassexp);
  // I_X = mat_pgm_read (inputFile);
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
  if (verbose == 1) printf("%d\n",N_s_LL);
  N_s = h_I * w_I - N_s_LL;	/* Dimension of the wavelet space */

  s_X = vec_new_zeros (N_s);	/* This sequence gathers the wavelet coeff that will bear the watermark signal */
  s_X_LL = vec_new_zeros (N_s_LL);	/* This sequence gathers the remaining lowlow freq subband ones. */

  extract (Wav_X, s_X_LL, s_X, levels);	/* fills s_X_LL and S_X */

  s_X_abs = vec_clone (s_X);
  vec_abs (s_X_abs);		/* Absolute values of the wavelet coefficient */

  /* Some statistics to predict the distortion */
  var_X = vec_variance (s_X);
  mean_X_Abs = vec_mean (s_X_abs);

  /* Logs */
  if (verbose == 1)
    printf ("var_X: %f\n", var_X);
  if (verbose == 1)
    printf ("mean_X_Abs: %f\n", mean_X_Abs);
  if (verbose == 1)
    printf ("mean_X_Abs/sqrt(var_X): %f\n", mean_X_Abs / sqrt (var_X));



    // Save the LL & HF Wavelet coefficients
    llcoeff_unwat.resize(N_s_LL);
    hfcoeff_unwat.resize(N_s);
    for(int i=0;i<N_s_LL;i++) {
      llcoeff_unwat[i] = s_X_LL[i];
    }
    for(int i=0;i<N_s;i++) {
      hfcoeff_unwat[i] = s_X[i];
    }

  /***************************************************
   *  Projection on N_v secret carriers going from   *
   *   the wavelet space to the secret subspace      *
   *  (see also section II.c of the paper)           *
   ***************************************************/

  /* PRNG initialisation */
  sizeAlea = (((long) N_v) * ((long) N_s) + 31) / 32;	/* Number of alea drawn by the PRNG */
  alea = (unsigned int*) malloc (sizeAlea * sizeof (unsigned int));
  mt19937_srand_by_array (key, 4);	/*Initializing the PRNG */

  for (i = 0; i < sizeAlea; i++)
    {
      alea[i] = mt19937_rand_int32 ();
    }

  v_X = vec_new_zeros (N_v);
  projectSubspace (s_X, alea, v_X);	/* calculates N_v correlations to project in the secret subspace */

  /* Logs */
  /*
  if (verbose == 1){
    for (i = 0 ; i < N_v ; i++ )
      {
	fprintf( fileV_x , "%f " , v_X[i]);
      }
    fprintf( fileV_x , "\n " );

    fprintf( file_mean_X_Abs , "%f\n " , mean_X_Abs );
  }
  */


  /***************************************************
   *  Compute embedding/detection parameters:        *
   *  - norm_s_W                                     *
   ***************************************************/

  if (ProportionalFlag == 1)
    {
      norm_s_W = sqrt ((double) (h_I * w_I) * 255. * 255. * pow (10, -psnr / 10) / (var_X)); /* see Eq. (25) in paper */
      if (verbose == 1)
	printf ("Proportional Embedding\n");
    }
  else
    {
      norm_s_W = sqrt ((double) (h_I * w_I) * 255. * 255. * pow (10, -psnr / 10) ); /* see Eq. (24) in paper */
      if (verbose == 1)
	printf ("Constant Embedding\n");
    }

  /***************************************************
   *  Find the closest carrier/cone v_C              *
   *  see Eq. (18) in paper                          *
   ***************************************************/

  v_X_Abs = vec_get_subvector (v_X, 0, N_c - 1);	/* Select the N_c first components of v_X */
  vec_abs (v_X_Abs);		/* Taken in absolute value */

  coneIdx = vec_max_index (v_X_Abs);	/* Index of the nearest secret carrier */
  if (verbose == 1)
    printf("coneIdx: %d\n",coneIdx);
  v_C = vec_new_zeros (N_v);

  if (v_X[coneIdx] > 0)
    v_C[coneIdx] = 1;
  else
    v_C[coneIdx] = -1;


  /***************************************************
   *  Project v_X in the MCB          plan           *
   *  from the secret subspace to the MCB plan       *
   *  see also section II.d of the paper             *
   ***************************************************/

  v_e1 = vec_new_zeros (N_v);
  v_e2 = vec_new_zeros (N_v);
  projectMCB (v_X, v_C, v_e1, v_e2, &c_X_1, &c_X_2);

  if (verbose == 1){
    printf ("c_X_1: %f c_X_2: %f\n", c_X_1, c_X_2);
    //fprintf (fileC_X, "%f %f #%s\n", c_X_1, c_X_2, inputFile);
  }

  /***************************************************
   *  Embedding in the MCB plan                      *
   *  see also section III.a of the paper            *
   ***************************************************/
  /* Find the watermark coordinates which maximize the robustness */

  if (ProportionalFlag == 0)
    mean_X_Abs = 1;

  max_robustness (c_X_1, c_X_2, mean_X_Abs, &norm_s_W, angle, &c_W_1, &c_W_2, &robustness);

  /* Embed , Eq (27) of the paper */
  c_Y_1 = c_W_1 * mean_X_Abs + c_X_1;
  c_Y_2 = c_W_2 * mean_X_Abs + c_X_2;

  /* Logs */
  if (verbose == 1)
    printf ("c_W_1: %f c_W_2: %f\n", c_W_1, c_W_2);
  if (verbose == 1)
    printf ("c_Y_1: %f c_Y_2: %f\n", c_Y_1, c_Y_2);
  if (verbose == 1)
    printf ("robustness: %f\n", robustness);
  if (verbose == 1)
    printf ("dist_min: %f\n", sqrt(robustness) );

  /* Logs */
  /*if (verbose == 1){
    fprintf (fileC_Y, "%f %f #%s\n", c_Y_1, c_Y_2, inputFile);
    fprintf (fileRob, "%f #%s\n", robustness, inputFile);
    fprintf (fileNorm_S_WRho, "%f %f #%s\n", norm_s_W, norm_s_W*mean_X_Abs, inputFile);
  } */

  /***************************************************
   *  Map back to the secret subspace                *
   *  see also Eq. (17) of the paper                 *
   ***************************************************/
  v_W = vec_new_zeros (N_v);
  v_W[0] = 1;
  /* reconstruction: v_W = c_W_1 * v_e1 + c_W_2 * v_e2 */
  vec_copy (v_W, v_e1);
  Vtmp = vec_clone (v_e2);
  vec_mul_by (v_W, c_W_1);
  vec_mul_by (Vtmp, c_W_2);
  vec_add (v_W, Vtmp);

  /**************************************************
  * Map back to the wavelet subspace                *
  * using a proportional or constant embedding      *
  ***************************************************/
  s_W = vec_new_zeros (N_s);
  if (ProportionalFlag==1)
    proportionalEmbed (v_W, alea, s_X_abs, s_W); /*Eq. (23) of the paper */
  else
    constantEmbed(v_W, alea, s_W);

  if (verbose == 1)
    printf ("meanW: %f\n", vec_mean (s_W));
  if (verbose == 1)
    printf ("varW: %f\n", vec_variance (s_W));

  /* Add s_W on s_X to obtain s_Y */
  s_Y = vec_clone (s_X);
  vec_add (s_Y, s_W);


    // here just the new wavelet coefficients since s_X_LL is used from above.
    hfcoeff_wat.resize(N_s);
    for(int i=0;i<N_s;i++) {
      hfcoeff_wat[i] = s_Y[i];
    }

    //printf ("A test value: %f\n", s_X[0]);
    //printf ("A test value: %f\n", s_X[10]);

  /***************************************************
  *  Map back to the wavelet space                   *
  ****************************************************/
  Wav_Y = mat_new_zeros (h_I, w_I);
  extractInv (s_X_LL, s_Y, levels, h_I, w_I, Wav_Y);

  /***************************************************
  *  Map back to the spatial image -                 *
  *  by an inverse wavelet transform                 *
  ***************************************************/
  /* Inverse wavelet decomposition of watermarked image */
  I_Y = it_wavelet2D_itransform (wavelet2D, Wav_Y);

  /* Writing */
  Rcpp::NumericMatrix finalimage (h_I, w_I);
  for( int i = 0 ; i < h_I ; i++ )
    for( int j = 0 ; j < w_I ; j++ )
      finalimage(i,j) = I_Y[i][j];

  /***************************************************
  *  Compute: - distortion varW , PSNR              *
  ***************************************************/
  /* Print the PSNR */
  psnr = mat_psnr (I_X, I_Y);
  if (verbose == 1){
    printf ("PSNR=%2.2fdB\n", psnr);
    //fprintf (filePSNR, "%f\n", psnr);
  }

  /***********************
  *  Leave things clean  *
  ***********************/


  mat_delete (I_X);
  mat_delete (I_Y);
  it_delete (wavelet2D);
  mat_delete (Wav_Y);
  mat_delete (Wav_X);

  vec_delete (s_X);
  vec_delete (s_X_abs);
  vec_delete (s_W);
  vec_delete (s_Y);
  vec_delete (s_X_LL);

  vec_delete (v_X);
  vec_delete (v_X_Abs);
  vec_delete (v_C);
  vec_delete (v_W);
  vec_delete (v_e1);
  vec_delete (v_e2);

  vec_delete (Vtmp);

  // fclose (filePSNR);
  // fclose (fileC_Y);
  // fclose (fileC_X);
  // fclose (fileRob);
  // fclose (fileNorm_S_WRho);
  //
  // fclose(fileV_x);
  // fclose(file_mean_X_Abs);

  free (alea);

  return Rcpp::List::create(//Rcpp::Named("llcoeffs_unwat") = llcoeff_unwat,
                            //Rcpp::Named("hfcoeffs_unwat") = hfcoeff_unwat,
                            //Rcpp::Named("hfcoeffs_wat") = hfcoeff_wat,
                            Rcpp::Named("watermarkedimage") = finalimage
                            );
  END_RCPP
}
