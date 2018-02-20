\name{plotGreymap}
\alias{plotGreymap}

\title{
Utilities Bitmaps. Shows image.
}
\description{
Plots an image. If the pixel values are not in the proper range, it will scale the pixels to the interval 0,1.
In this way, we can even visualize image residuals such as a camera fingerprint or wavelet frequency coefficients.
If you want to plot an image without scaling, you can use EBImage as Rpackage.
}
\usage{
plotGreymap(x, ...)
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{x}{
the image matrix.
}
  \item{\dots}{
additional parameters that are passed to
}
}
\author{
Erwin Quiring
}
