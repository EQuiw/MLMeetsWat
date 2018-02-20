# Loads an image from a given path.
readImage = function(x, pathToImages, resizev=NA, resizeh=NA, offsetv=1, offseth=1) {

  filetype = tools::file_ext(x)

  # a. load image
  if(filetype =="jpg") {
    img = readJPEG(paste(pathToImages,x,sep=""))
  } else if (filetype == "png") {
    img = readPNG(paste(pathToImages,x,sep=""))
  } else if(filetype == "pgm"){
    img = pixmap::read.pnm(file = paste(pathToImages,x,sep=""), cellres=1)@grey
  } else {
    stop("Unsupported input format although only jpgs or pngs or pgms should be used. Input:",x)
  }

  # b. determine if color or grey image, since jpg and png package has same output type, we can do the check once
  img = .convertIntoGREYObject(img)

  # c. resize the image
  if(!is.na(resizev) && !is.na(resizeh))
    img = .resizeOp(img, resize=resizev, resize2=resizeh, offsetx = offsetv, offsety = offseth)

  return(img)
}

"
Resize an image.
"
.resizeOp = function(img, resize, resize2, offsetx = 1, offsety = 1){
  img = img[offsetx:(resize+offsetx-1), offsety:(resize2+offsety-1)]
  return(img)
}

"
Converts a color image with 3 channels to grayscale image with 1 channel if necessary.
"
.convertIntoGREYObject = function(image) {
  if(class(image)=="matrix") {
    return(image)
  } else if(class(image)=="array" && dim(image)[3] == 3) {
    cat("Convert color image to grayscale image\n")
    return(convertIntoGrey(image))
  } else {
    stop("not compatible input type by png or jpeg package. Check your version and contact the author")
  }
}


"
Converts rgbmap into greymap if map is rgbmap, otherwise does nothing.
"
convertIntoGrey = function(map) {
  weights=c(0.299,0.587,0.114)
  map <- map$red*weights[1]+map$green*weights[2]+map$blue*weights[3]
  return(map)
}

"
Plots a matrix as image.
"
plotGreymap = function(x,...) {
  matrix = x
  if(min(matrix)<0 || max(matrix)>1){
    # standardize values
    cat("Standardize values... \n")
    matrix = (matrix - min(matrix)) / ( max(matrix) - min(matrix))
  }
  image( (t(matrix)[,nrow(matrix):1] ), col = grey(seq(0, 1, length = 256)),asp=1,...)
}


# Computes PSNR
computePSNR <- function(x, y, norm=255){
  rmse = sqrt(mean((x - y)^2))
  20 * log10(norm/rmse)
}
