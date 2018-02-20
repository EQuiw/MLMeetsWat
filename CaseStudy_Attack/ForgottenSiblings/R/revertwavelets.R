#### get image from wavelet coefficients ####

revertWaveletCoefficients <- function(llcoeffs, hfcoeffs, height, width, verbose=0) {

  # Call the C++ code to get ll and hf wavelet coefficients of pixmap.
  revertedimage = .Call("revertWaveletCoefficients", llcoeffs, hfcoeffs, height, width, verbose)

  # ouput must be between [0,255]
  if(max(revertedimage) <= 1) {
    warning("Output not in [0,255] range")
  }
  return(round(revertedimage))
}
