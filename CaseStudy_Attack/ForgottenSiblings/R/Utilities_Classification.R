######## Standardization Functions ##########
# compute preproc = c( mean(xtrain), sd(xtrain) ) before
# then simply use doprep (xtrain, preproc) or invprep(xtrain, preproc)...
# much faster than some packages I've found to do such preprocessing and its inversion...

getpreprocessor <- function(xdata) {
  list(colmean = colMeans(xdata), colsd = apply(xdata, 2, sd))
}

doprep <- function(xdata, preproc) {
  if(class(xdata)=="numeric"){
    return( (xdata - preproc$colmean) /  preproc$colsd )
  } else if(class(xdata) == "matrix"){
    return( t(apply(xdata, 1, function(r) { (r - preproc$colmean) / preproc$colsd } )) )
  } else {
    stop("Wrong input")
  }
}

invprep <- function(xdata, preproc) {
  if(class(xdata)=="numeric"){
    return( xdata * preproc$colsd + preproc$colmean )
  } else if(class(xdata) == "matrix"){
    return( t(apply(xdata, 1, function(r) { (r * preproc$colsd) + preproc$colmean } )) )
  } else {
    stop("Wrong input")
  }
}
