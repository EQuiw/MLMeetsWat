########## Utilities to load and check data ##############

# Loads the full dataset and returns it as list without further processing
just_load_fulldataset <- function(outputdir, database, suffix) {
  ### A. Load

  load(file = file.path(outputdir, paste0("imgs_wavs_", database, suffix, "_orig_ll.RData"))) # xtrain_ll
  load(file = file.path(outputdir, paste0("imgs_wavs_", database, suffix, "_orig_hf.RData"))) # xtrain_orig_hf
  load(file = file.path(outputdir, paste0("imgs_wavs_", database, suffix, "_wat_hf.RData"))) # xtrain_wat_hf

  ### B. Merge
  xtrain = rbind(xtrain_wat_hf, xtrain_orig_hf)
  xtrainll = rbind(xtrain_ll, xtrain_ll)
  ytrain = to_categorical( c(rep(0,nrow(xtrain_wat_hf)), rep(1, nrow(xtrain_orig_hf))) , 2)

  rm(xtrain_ll, xtrain_orig_hf, xtrain_wat_hf)
  gc()
  ret_list = list(xtrainll = xtrainll, xtrain = xtrain, ytrain = ytrain)
}


# Loads the full dataset, splits it into train and val. set, shuffles the train set and if wanted, removes the pairs.
# If pairs are removed, we will not! have for each marked image its unmarked version.
get_my_datasets <- function(outputdir, database, suffix, splitratio = 0.2, randomseed = 41, shuffletrain = T, removepairstrain = F) {

  ### A. Load

  load(file = file.path(outputdir, paste0("imgs_wavs_", database, suffix, "_orig_ll.RData"))) # xtrain_ll
  load(file = file.path(outputdir, paste0("imgs_wavs_", database, suffix, "_orig_hf.RData"))) # xtrain_orig_hf
  load(file = file.path(outputdir, paste0("imgs_wavs_", database, suffix, "_wat_hf.RData"))) # xtrain_wat_hf


  ### B. Merge wat & no-wat and then split into train-test

  testsetsize = round(NROW(xtrain_wat_hf) * splitratio)
  test_range = (NROW(xtrain_wat_hf) - testsetsize) : NROW(xtrain_wat_hf)
  train_range = 1 : (NROW(xtrain_wat_hf) - testsetsize - 1)

  xtest = rbind(xtrain_wat_hf[ test_range , ], xtrain_orig_hf[ test_range , ])
  xtrain = rbind(xtrain_wat_hf[ train_range , ], xtrain_orig_hf[ train_range , ])

  xtestll = rbind(xtrain_ll[ test_range , ], xtrain_ll[ test_range , ])
  xtrainll = rbind(xtrain_ll[ train_range , ], xtrain_ll[ train_range , ])

  ytrain = to_categorical( c(rep(0,length(train_range)), rep(1, length(train_range))) , 2) # 1nd column = wat presence = 1, 2nd column = absence = 1
  ytest = to_categorical( c(rep(0,length(test_range)), rep(1, length(test_range))) , 2)

  rm(xtrain_ll, xtrain_orig_hf, xtrain_wat_hf)
  gc()

  # D. Remove pairs
  if(removepairstrain == TRUE){
    mi = sum(ytrain[,1]==1)
    ytrain = rbind( ytrain[1:floor(0.5*mi), ], ytrain[ceiling(1.5*mi+1):nrow(ytrain),] )
    xtrain = rbind( xtrain[1:floor(0.5*mi),], xtrain[ceiling(1.5*mi+1):nrow(xtrain),] )
    xtrainll = rbind( xtrainll[1:floor(0.5*mi),], xtrainll[ceiling(1.5*mi+1):nrow(xtrainll),] )
    gc()
  }

  ## C. shuffle training data
  if(shuffletrain) {
    set.seed(randomseed)
    newindicestrain = sample(nrow(xtrain))
    xtrain = xtrain[newindicestrain, ]
    xtrainll = xtrainll[newindicestrain, ]
    ytrain = ytrain[newindicestrain, ]
  }

  return(list(xtrainll = xtrainll, xtrain = xtrain, ytrain = ytrain, xtestll = xtestll, xtest = xtest, ytest = ytest))
}




# Checks if label of the input data corresponds to watermark detector
# Moreover, it saves the cos values for watermarked and no-watermarked images.
datalabel_check <- function(datall, datahf, ydata, mysizex, mysizey) {

  stats_waterm = numeric( sum(ydata[, 1] == 1) )
  stats_nowat = numeric( sum(ydata[, 2] == 1) )
  j = 1 # we have an own index for each vector
  k = 1

  for(i in 1:nrow(ydata)){

    rimg = revertWaveletCoefficients(llcoeffs = datall[i, ], hfcoeffs = datahf[i, ], height=mysizex, width=mysizey, verbose=0)
    detect_result = detect_watermark(pixmap = rimg, verbose = F)
    if( detect_result$flag_detect != ydata[i, 1] ){
      cat("mismatch at ", i, "with cos value (maybe in random margin?):", detect_result$cos, "\n")
    }
    if( ydata[i, 1] == 1 ) {
      stats_waterm[j] = detect_result$cos
      j = j + 1
    } else {
      stats_nowat[k] = detect_result$cos
      k = k + 1
    }

  }
  return(list(waterm=stats_waterm, nowat=stats_nowat))
}
