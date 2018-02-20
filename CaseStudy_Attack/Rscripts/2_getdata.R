#################### Get Data #######################
library(pixmap, quietly=TRUE) # necessary for pgm files; you don't need it if you don't have any pgm files.
library(png)
library(jpeg)

library(ForgottenSiblings)

## specify path, and other parameters, get all images

database = "Manual"
suffix = ""
filepath = "pathwhereimagesarelocated"
outputdir = "path-where-we-save-the-RData"
pattern = NA
verbose = F

maxnoimages = 40000
min_threshold_forwatermark = 0.45

if(!is.na(pattern)){
  files = grep(pattern, list.files(filepath), perl=T, value=T)
} else {
  # Read all PNG or JPG images from Source dir if pattern is NA
  files = list.files(filepath, pattern = "(.*\\.png)|(.*\\.jpg)|(.*\\.pgm)")
}


cat("Found",length(files)," images in specified path\n")

if(length(files) > maxnoimages) {
  files = files[1:maxnoimages]
  cat("I'll limit the number of files to:", length(files), "\n")
  suffix = maxnoimages
}



## Preallocate the matrix and then fill it continously
temp_img = readImage(files[1], filepath, resizev = NA, resizeh = NA) * 255
temp_img_wavs = getWaveletCoefficients(pixmap = temp_img)
colsexpectedll = length(temp_img_wavs$llcoeffs)
colsexpectedhf = length(temp_img_wavs$hfcoeffs)
cat("Dimension is HF:", colsexpectedhf, ", LF:", colsexpectedll, "\n")

xtrain_ll <- matrix(data=NA_real_, nrow = length(files), ncol = colsexpectedll)
xtrain_orig_hf <- matrix(data=NA_real_, nrow = length(files), ncol = colsexpectedhf)
xtrain_wat_hf <- matrix(data=NA_real_, nrow = length(files), ncol = colsexpectedhf)

error_count = 0
## Run through all images
for(i in 1:length(files)){
  if(i %% 250 == 0)
    cat("Row: ", i, "\n")

  # Read image, depending on file type, and resize it immediately
  img = readImage(files[i], filepath, resizev = NA, resizeh = NA, offsetv, offseth) * 255
  if(max(img) <= 10 || mean(img)<10 || mean(img)>250) {
    cat("Image:", i, " is too dark or bright..\n")
    error_count = error_count + 1
    next
  }
  # plotGreymap(img)

  # get wavelet coefficients for original image
  img_wavs = getWaveletCoefficients(pixmap = img)

  # embed watermark
  imgwithwatermark = embed_watermark(pixmap = img, verbose = verbose)
  # plotGreymap(imgwithwatermark$watermarkedimage)

  # get frequency coefficients
  imgwithwatermark_wavs = getWaveletCoefficients(pixmap = imgwithwatermark$watermarkedimage)

  # fill
  detect_result = detect_watermark(pixmap = imgwithwatermark$watermarkedimage, verbose = verbose)
  if( detect_result$flag_detect==1 && detect_result$cos > min_threshold_forwatermark ){
    xtrain_ll[i, ] = img_wavs$llcoeffs
    xtrain_orig_hf[i, ] = img_wavs$hfcoeffs
    xtrain_wat_hf[i, ] = imgwithwatermark_wavs$hfcoeffs
  } else {
    cat("Image:", i, " is not used, no succ. watermark embedding, delete it later; Cos:", detect_result$cos, "\n")
    error_count = error_count + 1
  }

}

# remove NA rows where watermark embedding was not successfull
if(error_count > 0){
  cat("Handling NA rows\n")
  ind = apply(xtrain_ll, 1, function(x) all(is.na(x)))
  xtrain_ll = xtrain_ll[!ind, ]

  ind2 = apply(xtrain_orig_hf, 1, function(x) all(is.na(x)))
  xtrain_orig_hf = xtrain_orig_hf[!ind2, ]

  ind3 = apply(xtrain_wat_hf, 1, function(x) all(is.na(x)))
  xtrain_wat_hf = xtrain_wat_hf[!ind3, ]

  assertthat::are_equal(ind, ind2, ind3)
}
assertthat::are_equal(NROW(xtrain_ll), NROW(xtrain_orig_hf), NROW(xtrain_wat_hf))

## Save data
print(xtrain_orig_hf[1:5,1:5])
print(xtrain_wat_hf[1:5,1:5])
print(xtrain_ll[1:5,1:5])

save(xtrain_ll, file = file.path(outputdir, paste0("imgs_wavs_", database, suffix, "_orig_ll.RData")))
save(xtrain_orig_hf, file = file.path(outputdir, paste0("imgs_wavs_", database, suffix, "_orig_hf.RData")))
save(xtrain_wat_hf, file = file.path(outputdir, paste0("imgs_wavs_", database, suffix, "_wat_hf.RData")))



