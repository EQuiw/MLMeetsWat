#### get wavelet coefficients ####

getWaveletCoefficients <- function(pixmap) {

  # input must be between [0,255]
  if(max(pixmap) <= 1) {
    warning("Input should be [0,255] range")
  }

  # Call the C++ code to get ll and hf wavelet coefficients of pixmap.
  listDenoisedImg = .Call("getWaveletCoefficients", pixmap)
  return(listDenoisedImg)
}
