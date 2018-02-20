############ Utilities to learn a model  ##############
library(keras)
library(tensorflow)
#library(ROCR)

get_keras_model <- function(input_dim, l2_regulariz = 0.1, neurons = c(120,30), dropoutrate=0.1, activationparam = "relu") {

  model <- keras_model_sequential()

  # A. Input Layer
  model %>%
    layer_dense(units = neurons[1], activation = activationparam, input_shape = input_dim, kernel_regularizer = regularizer_l2(l = l2_regulariz)) %>%
    layer_dropout(rate = dropoutrate)

  # B. Hidden Layers
  if(length(neurons)>1){
    for(i in 2:length(neurons)){
      model  %>%
        layer_dense(units = neurons[i], activation = activationparam, kernel_regularizer = regularizer_l2(l = l2_regulariz)) %>%
        layer_dropout(rate = dropoutrate)
    }
  }

  # C. Output Layer
  model %>%
    layer_dense(units = 2, name="layer_before_softmax") %>%
    layer_activation(activation = "softmax")

  model %>% compile(
    loss = 'binary_crossentropy',
    optimizer = optimizer_adadelta(), # optimizer_rmsprop(),
    metrics = c('accuracy')
  )

  return(model)
}


