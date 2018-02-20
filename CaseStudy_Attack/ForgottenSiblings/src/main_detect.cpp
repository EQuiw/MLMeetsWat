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




#include <time.h>

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

RcppExport SEXP detectWatermark(
    SEXP inputimageassexp,
    SEXP verbosesexp,
    SEXP watermarkkeysexp
) {

  // try-catch-macros from RCPP: BEGIN_RCPP and END_RCPP
  BEGIN_RCPP

  /* Spatial domain */
  unsigned int h_I, w_I;	/* height and width of the image in pixels */
  mat I_X = NULL;		/* Image under scrutiny */

  /* Wavelet domain */
  it_wavelet2D_t *wavelet2D = NULL;	/* this defines a wavelet transform in libit */
  int levels;			/* Levels of the wavelet decomposition */
  mat Wav_X = NULL;		/* Wavelet decomposition of I_X */

  /* Wavelet Space */
  int N_s;			/* Dimension of the wavelet subspace */
  vec s_X = NULL;		/* Host sequence (high frequencies wavelet coeff) */
  int N_s_LL;			/* Dimension of the low low band sequence */
  vec s_X_LL = NULL;		/* Host low low band (low frequencies wavelet coeff) */

  /* Secret subspace */
  int N_v;        	       	/* dimension of the secret subspace */
  vec v_X = NULL,		/* secret vector for the host */
    v_C = NULL,			/* carrier/cone vector */
    v_X_Abs = NULL,             /* Absolute Value for the host */
    v_e1 = NULL,	        /* vector of the first element of the basis of the MCB plan */
    v_e2 = NULL;		/* vector of the second element of the basis of the MCB plan */
  int N_c;			/* N_c : nb of possible cones */

  /* MCB plan */
  double c_X_1, c_X_2;		/* First and second coordiantes of the host */

  // Misc.
  // char *inputFile = argv[1];  	/* Image under scrutiny */

  unsigned int key[4];          /* 128 bits long key */
  unsigned int key1;		/* key used for embedding or detection - belongs to 0 - 2^31. Change sign to use the 32nd bit */
  unsigned int key2;		/* key used for embedding or detection - belongs to 0 - 2^31. Change sign to use the 32nd bit */
  unsigned int key3;		/* key used for embedding or detection - belongs to 0 - 2^31. Change sign to use the 32nd bit */
  unsigned int key4;		/* key used for embedding or detection - belongs to 0 - 2^31. Change sign to use the 32nd bit */

  int coneIdx;			/* Index of the best cone */

  long i;                       /* increment */
  long sizeAlea;                /* Number of alea drawn by the PRNG */
  unsigned int *alea;           /* 128 bits long key */

  double Cos_angle_min ; /* cos of the thiniest cone */
  double Cos_angle_max ; /* cos of the largest cone */
  double Cos_angle_int ; /* cos of the internal cone */

  double norm_c_min ; /* parameter to create a truncated cone */
  double period_saw ; /* period of a saw */
  double ratio_saw ; /* ratio width of a teeth / period */

  double Cos_angle_ext ; /* cos of the external cone */
  double norm_c_X; /* norm of the tested image in the MCB plane */
  double Cos_angle; /* angle of the tested image */
  double r; /* random value */

  int decision; /* decision value */
  int verbose; /* verbose flag */



  // FILE *fileC_Z = fopen ("OutFiles/fileC_Z.dat", "a+");


  int flag_detect = 0; /* Flag for detection */


  /* Parameters Initialisation */
  levels=3;
  N_v=256;
  N_c=30;

  Cos_angle_min = 0.3479;   /* pfa = 3.10-7 for 30 cones, cos of the internal cone */
  Cos_angle_max = 0.3248;   /* pfa = 3.10-6 for 30 cones, cos of the external cone */
  Cos_angle_int = Cos_angle_min;
  norm_c_min = 10;
  period_saw = 30;
  ratio_saw = 0.15;

  /*****************
   * Load the key  *
   ******************/
  Rcpp::NumericVector watermarkkey(watermarkkeysexp);
  key1= watermarkkey[0];
  key2= watermarkkey[1];
  key3= watermarkkey[2];
  key4= watermarkkey[3];

  key[0] = key1;
  key[1] = key2;
  key[2] = key3;
  key[3] = key4;

  /*************************************************
   * Read the pgm image and obtain an Image I_X    *
   *************************************************/

  verbose = Rcpp::as<int>(verbosesexp);
  // Convert the input image from R to an C++ vector and then convert it to a mat object, used by Broken Arrows
  I_X = convertSexpIntoMat(inputimageassexp);

  h_I = mat_height (I_X);
  w_I = mat_width (I_X);

  /***************************************************
   *  Wavelet decomposition of I_X to obtain Wav_X  *
   ***************************************************/

  /* Transform the image */
  wavelet2D = it_wavelet2D_new (it_wavelet_lifting_97, levels+1);
  Wav_X = it_wavelet2D_transform (wavelet2D, I_X);

  /***************************************************
   *  Sequence  extraction s_X                       *
   *  (see also section II.b of the paper)           *
   ***************************************************/

  N_s_LL = h_I * w_I / ((int) pow (2, (2 * levels)));
  N_s = h_I * w_I - N_s_LL;	/* Dimension of the wavelet space */

  s_X = vec_new_zeros (N_s);	/* This sequence gathers the wavelet coeff that potentially bear the watermark signal */
  s_X_LL = vec_new_zeros (N_s_LL);	/* (Useless) This sequence gathers the remaining lowlow freq subband ones. */

  extract (Wav_X, s_X_LL, s_X, levels); /* fills s_X_LL and S_X */


  /***********************************************************
   * Note: for security purposes, here we should threshold   *
   * the wavelet coefficients in order to avoid attacks done *
   * saturating the wavelet coefficients and removing the    *
   * watermark (not implemented)                             *
   ***********************************************************/

  /***************************************************
   *  Projection on N_v secret carriers going from   *
   *   the wavelet space to the correlation space    *
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
  projectSubspace (s_X, alea, v_X);	// calculates N_v correlations

  /***************************************************
   *  Find the closest carrier/cone v_C              *
   *  see Eq. (18) in paper                          *
   ***************************************************/

  v_X_Abs = vec_get_subvector (v_X, 0, N_c - 1);	// Select the N_c first components of v_X
  vec_abs (v_X_Abs);		// Taken in absolute value

  coneIdx = vec_max_index (v_X_Abs);	// Index of the nearest secret carrier

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

  /***************************************************
   *  Compute the angle of the test signal v_X       *
   *  see also Eq. (5) of the paper                  *
   ***************************************************/
  norm_c_X = sqrt(c_X_1*c_X_1+c_X_2*c_X_2);
  Cos_angle= c_X_1/norm_c_X;
  if(verbose == 1) printf("Cos_angle: %f\n",Cos_angle);
  if(verbose == 1) printf("c_X_1: %f c_X_2: %f\n", c_X_1 , c_X_2 );

  /***************************************************
   *  Decide if the watermark is present according   *
   *  to a sculpted cone                             *
   *  see also section V of the paper                *
   ***************************************************/

  /***************************************************
   *           Random border parameter               *
   ***************************************************/
  it_randomize();
  r = it_rand(); /* between 0 and 1 ; different each time */
  Cos_angle_ext = Cos_angle_min - (Cos_angle_min-Cos_angle_max)*(1 + r)/2.;   /* See sec V.a of the paper */

  /***************************************************
   *  Decide if the watermark is present according   *
   *  to a basic or a scupted cone                   *
   ***************************************************/

  /* d = 1  => watermark detected;
     d < 0  => watermark not present;
     d = -1 => c_X near the origin;
     d = -2 => c_X in a saw tooth;
     d = -3 => c_X is not in the external cone */


  if (Cos_angle > Cos_angle_ext) /* We are in the external (random) cone */
    {
      if (norm_c_X < norm_c_min) /* See sec V.c of the paper */
	{
	  decision=-1; /* we are near the origin; */
	}
      else
	{
	  double saw = c_X_1/period_saw;
	  saw = saw - floor(saw);
	  if (saw < 0) {saw = -saw;}
	  if (saw < ratio_saw){
	    if (Cos_angle < Cos_angle_int) /* See sec V.b of the paper */
	      {
		decision = -2; /* we are in a saw, between cos_angle_max and cos_angle_min */
	      }
	    else /* See sec III.a of the paper */
	      {
		decision = 1; /* we are in the internal cone */
	      }
	  }
	  else /* See sec V.a of the paper */
	    {
	      decision = 1; /* we are in the external cone and not in a saw */
	    }
	}
    }
  else
    {
      decision=-3; /* We are not in the external (random) cone */
    }

  if (decision > 0)
    {
      flag_detect = 1; /* Watermark detected ! */
    }


  if (flag_detect > 0)
    {
     if(verbose == 1) printf ("YES: A watermark has been detected\n");
    }
  else
    {
     if(verbose == 1) printf ("NO: No watermark has been detected\n");
    }

  //if(verbose == 1) fprintf (fileC_Z, "%f %f #%s\n", c_X_1, c_X_2, inputFile);
  if (verbose == 1)
    printf("coneIdx: %d\n",coneIdx);


  // fclose(fileC_Z);

  /* Leave things clean */
  mat_delete (I_X);
  it_delete (wavelet2D);
  mat_delete (Wav_X);
  vec_delete (s_X);
  vec_delete (s_X_LL);
  vec_delete (v_X);
  vec_delete (v_C);
  vec_delete (v_X_Abs);
  vec_delete (v_e1);
  vec_delete (v_e2);
  free (alea);



  return Rcpp::List::create(Rcpp::Named("flag_detect") = flag_detect,
                            Rcpp::Named("coneIdx") = coneIdx,
                            Rcpp::Named("cos") = Cos_angle,
                            Rcpp::Named("c_X_1") = c_X_1,
                            Rcpp::Named("c_X_2") = c_X_2
  );

  END_RCPP
}
