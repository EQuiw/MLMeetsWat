#include "rcppMyUtils.h"

mat convertSexpIntoMat(SEXP inputimageassexp) {
  Rcpp::NumericMatrix inputimg(inputimageassexp);
  int width, height, max_val; //, i, j;
  height = inputimg.nrow();
  width = inputimg.ncol();
  mat m;

  m = mat_new( height, width );
  for( int i = 0 ; i < height ; i++ )
    for( int j = 0 ; j < width ; j++ )
      m[i][j] = (double) inputimg(i,j);

  return m;
}



