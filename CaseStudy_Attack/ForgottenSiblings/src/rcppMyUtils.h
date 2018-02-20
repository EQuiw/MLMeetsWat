#ifndef MYRCPPUTILS
#define MYRCPPUTILS

#include <Rcpp.h>
#include "mat.h"

/**
 * A small helper to convert an SEXP type from R to the mat object that is used by Broken Arrows in C++.
 */
mat convertSexpIntoMat(SEXP inputimageassexp);

#endif /* MYRCPPUTILS */
