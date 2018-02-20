############# Do Attack ##############
library(keras)
library(tensorflow)
library(ForgottenSiblings)

classihelperdir <- "set/path/to/Rscripts" # SET PATH where this file is located
source(file.path(classihelperdir,"/Classification_Helper.R"))

####################### Set Parameters #######################

outputdir = "-where-the-RData-were-saved"   # SET PATH
databaseTrain = "Raise" # Raise"
databaseTest = "Manual"   # "Dresden"
suffixTrain = "40000" # 40000"
suffixTest = "2"
mysize = 128

# select the batch we want to process (I choose 50 images) or set attackindices manually below.
# You can modify this script to be called via bash, then you can easily process multiple batches in parallel:
catt = 1
attackindices = seq((catt-1)*50 + 1, catt*50)
print(attackindices)
# attackindices = 280:320 # set the indices of the images where we want to remove the watermark

saveresultsdir = "-where-the-experimental-results-will-be-saved"  # SET PATH
saveresultssuffix = paste0("linf_removal_eusipco", catt, databaseTest, suffixTest) # suffix that will be appended on the Rdata that contain the results and are stored on the disk in saveresultsdir.
kerasmodelsuffix = "pairsremoved_eusipco"



sess = tf$Session()
k_set_session(sess)


###################### Load Datasets #######################

## I. Load ##
dataset_watnowat = get_my_datasets(outputdir = outputdir, database = databaseTrain, suffix = suffixTrain, splitratio = 0, randomseed = 41, shuffletrain = TRUE, removepairstrain=TRUE)
xtrainll = dataset_watnowat$xtrainll
xtrain = dataset_watnowat$xtrain
ytrain = dataset_watnowat$ytrain
rm(dataset_watnowat)
gc()

# do not sort the test set
dataset_watnowat = get_my_datasets(outputdir = outputdir, database = databaseTest, suffix = suffixTest, splitratio = 0, randomseed = 41, shuffletrain = FALSE)
xtestll = dataset_watnowat$xtrainll
xtest = dataset_watnowat$xtrain
ytest = dataset_watnowat$ytrain
rm(dataset_watnowat)
gc()


## preprocessing

preproc = getpreprocessor(xdata = xtrain)
xtrain = doprep(xdata = xtrain, preproc = preproc)
xtest = doprep(xdata = xtest, preproc = preproc)
gc()

#xtestinv = invprep(xdata = xtestsc, preproc = preproc)


###################### Train Model #######################

if(!file.exists(file.path(saveresultsdir,paste0("keras_model",kerasmodelsuffix,".hdf5")))){
  # model = get_keras_model(input_dim = dim(xtrain)[2], l2_regulariz = 0.22, dropoutrate = 0.02, neurons = c(90,60,30), activationparam = "relu")

  # multiple runs to obtain a good one...
  best_model = NA
  best_model_score = -Inf
  for(i in 1:5){
    model = get_keras_model(input_dim = dim(xtrain)[2], l2_regulariz = 0.20, dropoutrate = 0.0, neurons = c(40,60,40), activationparam = "relu")
    summary(model)


    opt <- optimizer_rmsprop(lr = 0.0001, decay = 1e-6)
    model %>% compile(
      loss = "categorical_crossentropy",
      optimizer = opt,
      metrics = "accuracy"
    )

    early_stop = callback_early_stopping(monitor = "val_acc", min_delta = 0.05, patience = 5)

    history <- model %>% fit(
      xtrain, ytrain,
      epochs = 50, batch_size = 128,
      validation_split = 0.3,
      callbacks = list(early_stop)
    )

    score <- (model %>% evaluate(xtest, ytest))
    val_acc <- history$metrics$val_acc[length(history$metrics$val_acc)]
    cat("Test-Acc:",i,":",score$acc,"\t Val-Acc:", val_acc, "\n")

    if(val_acc > best_model_score){
      best_model = model
      best_model_score = val_acc
    }

    gc()
  }

  model = best_model
  save_model_hdf5(model, filepath = file.path(saveresultsdir,paste0("keras_model",kerasmodelsuffix,".hdf5")), overwrite = F)
} else {
  model <- load_model_hdf5(filepath = file.path(saveresultsdir,paste0("keras_model",kerasmodelsuffix,".hdf5")))
  model %>% evaluate(xtest, ytest)
}

score <- (model %>% evaluate(xtest, ytest))
cat("Final Model - Test-Acc:",":",score$acc,"\n")

###################### Attack Preparation #######################

attack_settings = list(
  max_iters = 100,   # max number of iterations during gradient descent
  ord = "inf",      # use sign(gradient), set "inf", or use grad, set 2.
  gradsignthres = NA, # only used if ord = "inf". Set by attack_avoidlocalminima_settings.
  check_oracle = 1,    # at each x call, check if oracle has same output
  targetclass = 1,     # 1 = remove watermark, 2 = add watermark
  confidence = 7,     # we stop the attack at what prediciton score -> confidence, since we believe we succeeded.
  optimization_strategy = 2,
  closs_range = c(7.5,10,40,50,75,300),   # the c parameter from paper
  userefsample = T,    # if True, we compute the distance penalty by ||x - adv_x|| or if False, we compute || adv_x ||
  earlybreakminiters = 30  # early break after x iterations
)

attack_avoidlocalminima_settings = list(
  eps_range = c(0.05,0.15, 0.2, 0.4, 0.75, 2.5),
  varyinput = c(0, 0.03),
  gradsignthres_range = c(0.0001, 0.00001)
)

myverbose = 2

###################### Do Attack #######################

k_set_learning_phase(0)


final_results = data.frame( attack_index = numeric(length(attackindices)), psnr = numeric(length(attackindices)), nnoutputtargetclass = numeric(length(attackindices)),
                            flagdetected = numeric(length(attackindices)), cosdetected = numeric(length(attackindices)), cosbefore = numeric(length(attackindices)),
                            closs = numeric(length(attackindices)), eps = numeric(length(attackindices)), varyinput = numeric(length(attackindices)),
                            cgradsignthres = numeric(length(attackindices)),
                            l0 = numeric(length(attackindices)), linf = numeric(length(attackindices)), l2 = numeric(length(attackindices)))
final_results_adv_x = list()

iterationc = 1
for(iterationc in 1:length(attackindices)){
  cat("\n --------------------->", iterationc,"<-----------------\n", rep("*",20), "\n")

  ## get the source image and the equivalent partner (with or without watermark) for debugging
  sampleinds = get_src_imageindex_andpair(ytest = ytest, iterationc = attackindices[iterationc], attack_settings = attack_settings)
  sampleind_src = sampleinds[1]; sampleind_pair = sampleinds[2]

  # Some debugging. Show the detection results from detector.
  res_src = detect_watermark(pixmap = NA, wavsll = xtestll[sampleind_src, ], wavshf = invprep(xdata = xtest[sampleind_src, ], preproc = preproc), sizex = mysize, sizey = mysize, verbose = F)
  res_pair = detect_watermark(pixmap = NA, wavsll = xtestll[sampleind_pair, ], wavshf = invprep(xdata = xtest[sampleind_pair, ], preproc = preproc), sizex = mysize, sizey = mysize, verbose = F)

  if(myverbose >= 2){
    cat(unlist(res_src),"\n")
    cat(unlist(res_pair),"\n")
  }

  adv_x = xtest[sampleind_src, ]
  adv_x_ll = xtestll[sampleind_src, ]
  nofeatures = dim(xtrain)[2]
  ref_sample =  adv_x - 1e-10 # for numerical stability

  # Short check that we have chosen an image with (or without) a watermark if we want to remove it (or add it).
  if(attack_settings$targetclass==1 && !assertthat::are_equal(res_src$flag_detect, 1)){
    stop("unvalid input")
  }
  if(attack_settings$targetclass==2 && !assertthat::are_equal(res_src$flag_detect, 0)){
    stop("unvalid input")
  }


  best_adv_x_psnrresult = -Inf

  ## We iterate here over each combination of parameters that may influence the attack.
  ## We try various step sizes, various cost values, slightly change the starting position, ...
  for(closs in attack_settings$closs_range) {
    for(cgradsignthres in attack_avoidlocalminima_settings$gradsignthres_range){
      attack_settings$gradsignthres = cgradsignthres

      for( curvaryinput in attack_avoidlocalminima_settings$varyinput) {
        noise <- rnorm(n = nofeatures, mean = 0, sd = curvaryinput)
        mod_adv_x <- adv_x + noise

        for(eps in attack_avoidlocalminima_settings$eps_range){

          cat(rep("*",20),"\n","C-loss:", closs, ", VaryInput:", curvaryinput, ", eps:", eps, ", grad sign thr:", cgradsignthres, "\n")

          adv_x_final_list = do_watermark_attack(sess = sess, model = model, adv_x = mod_adv_x, adv_x_ll = adv_x_ll,
                                                 nofeatures = nofeatures, attack_settings = attack_settings, eps = eps, costloss = closs, mysize = mysize,
                                                 preproc = preproc, ref_sample = ref_sample, verbose = myverbose)

          if( adv_x_final_list$watdetectoroutput$flag_detect == (attack_settings$targetclass-1) ){
            adv_x_rev = revertWaveletCoefficients(llcoeffs = adv_x_ll, hfcoeffs = invprep(xdata = adv_x_final_list$adv_x, preproc = preproc), height=mysize, width=mysize, verbose=1)
            adv_x_orig = revertWaveletCoefficients(llcoeffs = adv_x_ll, hfcoeffs = invprep(xdata = adv_x, preproc = preproc), height=mysize, width=mysize, verbose=1)
            # plotGreymap(adv_x_rev)

            # distances:
            l0_dist = sum(round(adv_x,2) != round(adv_x_final_list$adv_x,2)) / length(adv_x)
            linf_dist = max(abs(round(adv_x,2) - round(adv_x_final_list$adv_x,2)))
            curdist = sqrt(sum((adv_x - adv_x_final_list$adv_x)^2))
            curpsnr = computePSNR(x = adv_x_rev, y = adv_x_orig)
            cat("PSNR:", curpsnr, "\t Freq L0:", l0_dist, "\t L-inf:", linf_dist, "\t L-2:", curdist,"\n")

            # check if better, if so, update results for current image.
            if( curpsnr > best_adv_x_psnrresult ) {
              final_results$attack_index[iterationc] = attackindices[iterationc]
              final_results$psnr[iterationc] = curpsnr
              final_results$nnoutputtargetclass[iterationc] = adv_x_final_list$nnoutput[attack_settings$targetclass]
              final_results$flagdetected[iterationc] = adv_x_final_list$watdetectoroutput$flag_detect
              final_results$cosdetected[iterationc] =   adv_x_final_list$watdetectoroutput$cos
              final_results$cosbefore[iterationc] = res_src$cos
              final_results$closs[iterationc] = closs
              final_results$eps[iterationc] = eps
              final_results$varyinput[iterationc] = curvaryinput
              final_results$l0[iterationc] = l0_dist
              final_results$l2[iterationc] = curdist
              final_results$linf[iterationc] = linf_dist
              final_results$cgradsignthres[iterationc] = cgradsignthres

              final_results_adv_x[[iterationc]] = adv_x_rev
              best_adv_x_psnrresult = curpsnr
            }

          }
          cat("\n", rep("*",20), "\n")

        }
      }
    }
  }

} # end loop with iterationc


save(final_results, file=file.path(saveresultsdir, paste0("final_results", saveresultssuffix, ".RData")))
save(final_results_adv_x, file=file.path(saveresultsdir, paste0("final_results_adv_x", saveresultssuffix, ".RData")))

