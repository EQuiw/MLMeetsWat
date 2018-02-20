#library(steganal)
library(ForgottenSiblings)
context("Wavelet Coefficients test")

test_that("wav test", {
  #skip_on_cran()

  # I. First test, check if wavelet coefficients are correct
  wavs = getWaveletCoefficients(pixmap = lenaimg) # lenaimg obtained from data directory as part of the R package

  lenall_reference = scan("lena256.pgm.ll.wavelet")
  lenahf_reference = scan("lena256.pgm.wavelet")
  m1 = sum(round(lenall_reference,0) != round(wavs$llcoeffs,0))
  m2 = sum(round(lenahf_reference,0) != round(wavs$hfcoeffs,0))

  expect_equal(m1,0)
  expect_equal(m2,1) # one time, there is a number x.5 and x.49 , so this explains the mismatch.


  # II. Not check if the image is still the same if we convert the wav. coefficients back to the pixel domain
  rimg = revertWaveletCoefficients(llcoeffs = wavs$llcoeffs, hfcoeffs = wavs$hfcoeffs, height=256, width=256, verbose=0)
  m3 = sum(rimg != lenaimg)
  expect_equal(m3, 0)

})
