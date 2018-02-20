############ Utilities for attack #############

## get the source image and the equivalent partner (with or without watermark) for debugging
get_src_imageindex_andpair <- function(ytest, iterationc, attack_settings) {
  watindices = seq(1, NROW(ytest))[ytest[, 1] == 1]
  nowatindices = seq(1, NROW(ytest))[ytest[, 2] == 1]
  if(attack_settings$targetclass == 1) {
    # we remove watermark
    sampleind_src = watindices[iterationc]
    sampleind_pair = nowatindices[iterationc]
  } else {
    sampleind_pair = watindices[iterationc]
    sampleind_src = nowatindices[iterationc]
  }
  return(c(sampleind_src, sampleind_pair))
}


# a quick evaluation of the loss after a step of the gradient descent
eval_attack_step <- function(sess, losslist, adv_x, adv_x_ref, x, xref, attack_settings) {

  if(!attack_settings$userefsample){
    r = sess$run(losslist$loss, feed_dict = dict('x'= adv_x))
    r1 = sess$run(losslist$loss1, feed_dict = dict('x'= adv_x))
    r2 = sess$run(losslist$loss2, feed_dict = dict('x'= adv_x))
    return(list(loss=r,loss1=r1,loss2=r2))
  } else {
    r = sess$run(losslist$loss, feed_dict = dict('x'= adv_x, 'xref' = adv_x_ref))
    r1 = sess$run(losslist$loss1, feed_dict = dict('x'= adv_x, 'xref' = adv_x_ref))
    r2 = sess$run(losslist$loss2, feed_dict = dict('x'= adv_x, 'xref' = adv_x_ref))
    return(list(loss=r,loss1=r1,loss2=r2))
  }


}


# perform an update step during the gradient descent
updateadv_x <- function(sess, grad, adv_x, ref_sample, inputshape, eps, x, xref, attack_settings) {
  # Compute Gradient
  if (attack_settings$userefsample){
    cmpgrad = sess$run(grad, feed_dict = dict("x"= array_reshape(adv_x, inputshape),
                                              "xref"= array_reshape(ref_sample, inputshape)
    ))[[1]]
  } else {
    cmpgrad = sess$run(grad, feed_dict = dict("x"= array_reshape(adv_x, inputshape)) )[[1]]
  }

  # reshape to single numeric vector
  cmpgrad = cmpgrad[1,]

  # Refine gradient
  if(attack_settings$ord=="inf"){
    #cmpgrad = sign(cmpgrad)
    # cat("Grad:", cmpgrad[1:20],"\n")
    cmpgrad = cmpgrad / sqrt( sum (cmpgrad^2) )
    cmpgrad_threshold = attack_settings$gradsignthres
    # cat("Grad-N:", cmpgrad[1:20],"\n")
    cmpgrad[cmpgrad>cmpgrad_threshold] = 1
    cmpgrad[cmpgrad<(-cmpgrad_threshold)] = -1
    cmpgrad[cmpgrad>(-cmpgrad_threshold) & cmpgrad<cmpgrad_threshold] = 0
    # cat("Grad-A:", cmpgrad[1:10],"\n Zeros", sum(cmpgrad==0)/length(cmpgrad), "\n")

  } else if(attack_settings$ord==2){
    cmpgrad = cmpgrad / sqrt( sum (cmpgrad^2) )

  } else {
    stop("Unvalid ord choice")
  }

  # Update adv example
  adv_x = adv_x - eps * cmpgrad

  return(adv_x)
}



# get the loss function and the gradient
gradient_loss_fcts <- function(model, x, xref, costloss, attack_settings){

  # A. Distance term
  if(!attack_settings$userefsample){
    # regularize simply the wavelet coefficients, they should not increase so much
    l2dist = tf$sqrt(tf$reduce_sum(tf$square(x), axis=1L))
  } else {
    # use distance from reference point
    l2dist = tf$sqrt(tf$reduce_sum(tf$square(tf$subtract(x, xref)), axis=1L))
  }


  # B. Loss / Function output
  if(attack_settings$optimization_strategy == 1) {
    # B.1 gradient from softmax output directly, corresponds to f_4 from C&W

    preds = model(x)
    loss1 = costloss * preds[, attack_settings$targetclass]
    loss2 = l2dist
    loss = loss1 + loss2
    grad = tf$gradients(loss, x)

  } else if(attack_settings$optimization_strategy == 2) {
    # B.2 use output from layer before softmax, similar to f_6 from C&W
    intermediate_layer_model = keras_model(inputs = model$layers[[1]]$input,
                                           outputs = get_layer(model, name="layer_before_softmax")$output)
    pred_logits = intermediate_layer_model(x)

    if(attack_settings$targetclass==1){
      indexleft = 1
      indexright = 2
    } else if(attack_settings$targetclass==2){
      indexleft = 2
      indexright = 1
    } else {stop("targetclass wrong")}

    loss1 = costloss * (pred_logits[, indexleft] - pred_logits[, indexright])
    loss2 = l2dist
    loss =  loss1 + loss2
    grad = tf$gradients(loss, x)

  } else if(attack_settings$optimization_strategy == 3) {
    # B.3 use output from layer before softmax, but now use max in f(x) to limit loss1, corresponds to f_6 from C&W

    intermediate_layer_model = keras_model(inputs = model$layers[[1]]$input,
                                           outputs = get_layer(model, name="layer_before_softmax")$output)
    pred_logits = intermediate_layer_model(x)

    if(attack_settings$targetclass==1){
      indexleft = 1
      indexright = 2
    } else if(attack_settings$targetclass==2){
      indexleft = 2
      indexright = 1
    } else {stop("targetclass wrong")}

    loss1 = costloss * tf$maximum(0.0, pred_logits[, indexleft] - pred_logits[, indexright] + attack_settings$confidence)
    loss2 = l2dist
    loss =  loss1 + loss2
    grad = tf$gradients(loss, x)

  } else {
    stop("No valid attack strategy")
  }

  return(list(grad=grad, loss=loss, loss1=loss1, loss2=loss2))

}


# prepare an image for sending it to the watermark detector.
get_img_for_oracle_pred <- function(adv_x_ll, adv_x, preproc, height, width) {
  img_4oraclepred = revertWaveletCoefficients(llcoeffs = adv_x_ll, hfcoeffs = invprep(xdata = adv_x, preproc = preproc), height=height, width=width, verbose=0)

  img_4oraclepred[img_4oraclepred>255] = 255
  img_4oraclepred[img_4oraclepred<0] = 0
  return(img_4oraclepred)
}


# the main method of the attack.
do_watermark_attack <- function(sess, model, adv_x, adv_x_ll, nofeatures, attack_settings, eps, costloss, preproc, mysize, ref_sample = NA, verbose = 1) {



  inputshape <- c(1, nofeatures)
  x <- tf$placeholder(tf$float32, shape(NULL, nofeatures))

  if (attack_settings$userefsample){
    xref = tf$placeholder(tf$float32, shape(NULL, nofeatures))
  } else {
    xref = NA
  }

  gradient_loss_fcts_list = gradient_loss_fcts(model = model, x = x, xref = xref, costloss = costloss, attack_settings = attack_settings)
  #grad = gradient_loss_fcts_list$grad


  ############### get current prediction #################

  curattackpred = model %>% predict( array_reshape(adv_x, inputshape) )

  img_4oraclepred = get_img_for_oracle_pred(adv_x_ll = adv_x_ll, adv_x = adv_x, preproc = preproc, height = mysize, width = mysize)
  oraclepred = detect_watermark(pixmap = img_4oraclepred)

  # compare model's prediction with watermark detector's prediction
  watermarkpredicted = (curattackpred[, 1] >= 0.5)

  if( !watermarkpredicted && oraclepred$flag_detect==1){
    watermarkpredicted = TRUE
    if(verbose>=2)
      cat("No wat predicted, but there .. Mismatch occured with cos:", oraclepred$cos, "\n")
  }
  if( watermarkpredicted && oraclepred$flag_detect!=1){
    watermarkpredicted = FALSE
    if(verbose>=2)
      cat("Wat predicted, but not there .. Mismatch occured with cos:", oraclepred$cos, "\n")
  }


  currentloss = eval_attack_step(sess = sess, losslist = gradient_loss_fcts_list,
                                 adv_x = array_reshape(adv_x, inputshape), x=x, xref = xref,
                                 adv_x_ref = array_reshape(ref_sample, inputshape), attack_settings = attack_settings)

  if(verbose>=1)
    cat("---> Start at: ", 0, "\t NN-Output:", curattackpred, "\t loss:", currentloss$loss, "\tloss1:", currentloss$loss1, "\tloss2:", currentloss$loss2,
        "\t Wat-Det:", oraclepred$cos, "; (", oraclepred$c_X_1, ",", oraclepred$c_X_2, ")", "\n")


  ################ Iterate towards target #################
  # Iterate as long as watermark is spotted (if removal attack) or max iterations are not reached #
  itercount = 1
  decisionswapped = FALSE
  earlybreakqueue = list()
  earlybreakmean = Inf

  while(!decisionswapped && itercount < attack_settings$max_iters) {

    # A. Go one step towards gradient's direction
    adv_x <- updateadv_x(sess = sess, grad = gradient_loss_fcts_list$grad, adv_x = adv_x, ref_sample = ref_sample, inputshape = inputshape, eps = eps, x = x, xref = xref, attack_settings = attack_settings)

    # B. Evaluate loss
    currentloss = eval_attack_step(sess = sess, losslist = gradient_loss_fcts_list,
                                   adv_x = array_reshape(adv_x, inputshape), x=x, xref = xref, attack_settings = attack_settings,
                                   adv_x_ref = array_reshape(ref_sample, inputshape) )

    curattackpred = model %>% predict( array_reshape(adv_x, inputshape) )
    watermarkpredicted = (curattackpred[, 1] >= 0.5)

    # Compare with oracle if wanted
    if(itercount %% attack_settings$check_oracle == 0) {

      img_4oraclepred = get_img_for_oracle_pred(adv_x_ll = adv_x_ll, adv_x = adv_x, preproc = preproc, height = mysize, width = mysize)
      oraclepred = detect_watermark(pixmap = img_4oraclepred)

      if(verbose>=3)
        cat("Iteration: ", itercount, " \t NN-Output:", curattackpred, "\t loss:", currentloss$loss, "\tloss1:", currentloss$loss1, "\tloss2:", currentloss$loss2,
            "\t Wat-Det:", oraclepred$cos, "; (", oraclepred$c_X_1, ",", oraclepred$c_X_2, ")", "\n")
    } else {
      if(verbose>=3)
        cat("Iteration: ", itercount, " \t NN-Output:", curattackpred, "\t loss:", currentloss$loss, "\tloss1:", currentloss$loss1, "\tloss2:", currentloss$loss2,
            "\t Wat-Det:", "---------", "\n")
    }

    if(attack_settings$targetclass == 1 && !watermarkpredicted){
      decisionswapped = TRUE
    }
    if(attack_settings$targetclass == 2 && watermarkpredicted){
      decisionswapped = TRUE
    }

    # if decision swapped, then we would assume that attack was successfull, so we check the oracle if we are right.
    # if not, we would continue..
    if (decisionswapped) {

      img_4oraclepred = get_img_for_oracle_pred(adv_x_ll = adv_x_ll, adv_x = adv_x, preproc = preproc, height = mysize, width = mysize)
      oraclepred = detect_watermark(pixmap = img_4oraclepred)

      if(attack_settings$targetclass == 1 && oraclepred$flag_detect==1){
        decisionswapped = FALSE
      }
      if(attack_settings$targetclass == 2 && oraclepred$flag_detect==0){
        decisionswapped = FALSE
      }

    }

    if (decisionswapped && verbose >= 1) {
      cat("---> Success at: ", itercount, " \t NN-Output:", curattackpred, "\t loss:", currentloss$loss, "\t Wat-Det:",  oraclepred$cos, "; (", oraclepred$c_X_1, ",", oraclepred$c_X_2, ")", "<----- \n")
    }

    # early break
    earlybreakqueue[[length(earlybreakqueue)+1]] = curattackpred[, attack_settings$targetclass]
    if(length(earlybreakqueue) == attack_settings$earlybreakminiters){
       if (round(mean(unlist(earlybreakqueue)),8) >= earlybreakmean){
         if(verbose >= 2)
           cat("\t Early break at ", itercount, "\n")
         break
       }
      earlybreakmean = round(mean(unlist(earlybreakqueue)),8)
      earlybreakqueue = earlybreakqueue[-1]
    }


    itercount = itercount + 1
  }

  # round img now
  img_4oraclepred = get_img_for_oracle_pred(adv_x_ll = adv_x_ll, adv_x = adv_x, preproc = preproc, height = mysize, width = mysize)
  adv_x = doprep( getWaveletCoefficients(pixmap = img_4oraclepred)$hfcoeffs , preproc = preproc )

  return(list(adv_x = adv_x, watdetectoroutput = oraclepred, nnoutput = curattackpred))

}



