## embedding and detection of watermark ##

embed_watermark <- function(pixmap, verbose=0, watermarkkey = c(0,0,0,0)) {

  # input must be between [0,255]
  if(max(pixmap) <= 1) {
    stop("Input should be [0,255] range")
  }

  # Call the C++ code to get ll and hf wavelet coefficients of pixmap.
  imgwithwatermarklist = .Call("embedWatermark", pixmap, verbose, watermarkkey)

  imgwithwatermarklist$watermarkedimage = round(imgwithwatermarklist$watermarkedimage)
  imgwithwatermarklist$watermarkedimage[imgwithwatermarklist$watermarkedimage>255] = 255
  imgwithwatermarklist$watermarkedimage[imgwithwatermarklist$watermarkedimage<0] = 0
  return(imgwithwatermarklist)
}


detect_watermark <- function(pixmap, wavsll=NA, wavshf=NA, sizex=NA, sizey=NA, verbose=0, watermarkkey=c(0,0,0,0)) {

  # we can use either pixmap = pixel space, or frequency coefficients wavsll, wavshf.
  if(is.na(pixmap[1])) {
    if(is.na(wavsll[1]) || is.na(wavshf[1]) || is.na(sizex) || is.na(sizey))
      stop("Unvalid function input")

    pixmap = revertWaveletCoefficients(llcoeffs = wavsll, hfcoeffs = wavshf, height=sizex, width=sizey, verbose=verbose)
  }

  # input must be between [0,255]
  if(max(pixmap) <= 1) {
    warning("Input should be [0,255] range")
  }

  decs = .Call("detectWatermark", pixmap, verbose, watermarkkey)

  return(decs)
}
