#library(steganal)
library(ForgottenSiblings)
context("Broken Arrows test")

test_that("Broken Arrows test", {
  #skip_on_cran()

  # check embedding is correct
  lenawat = embed_watermark(pixmap = lenaimg, verbose = 0)
  load("./lena_watermarked.RData") # loads lenawithwatermark
  m1 = sum(lenawat$watermarkedimage != lenawithwatermark)
  expect_equal(m1, 0)
  expect_equal(lenawithwatermark[1], 194) # check first pixel, just to be sure it is the correct image


  # check detection is correct
  detec = detect_watermark(pixmap = lenaimg, verbose = 0)
  expect_equal(detec$flag_detect, 0)
  detec = detect_watermark(pixmap = lenawithwatermark, verbose = 0)
  expect_equal(detec$flag_detect, 1)

  expect_equal(detec$coneIdx, 28)
  expect_equal(detec$cos, 0.72756, tolerance = 0.01)
})
