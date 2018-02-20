\name{detect_watermark}
\alias{detect_watermark}
%- Also NEED an '\alias' for EACH other topic documented here.
\title{
Watermark Detection via Broken Arrows
}
\description{
Detect a watermark via Broken Arrows in a given image.
}
\usage{
detect_watermark(pixmap, wavsll=NA, wavshf=NA, sizex=NA, sizey=NA, verbose=0, watermarkkey=c(0,0,0,0))
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{pixmap}{ a 2-dim matrix that contains the pixel values in a range from [0,255].}
  \item{wavsll}{ the LL wavelet coefficients returned from the wavelet method}
  \item{wavshf}{ the higher frequencies returned from the wavelet method}
  \item{sizex}{  the height of the image }
  \item{sizey}{ the width of the image}
  \item{verbose}{ set 1 if status updates are wanted.}
  \item{watermarkkey}{ the watermark key, 4 dim vector with integer values from 0 to 2^31}
  }
\details{
You can detect a watermark with two different forms of input. I know, it would be better to have two separate methods for this.
Either you pass the image via the pixmap argument only, or
you pass the frequency coefficients directly. Then, you have to set the ll, hf, and image size arguments.
}
\value{
Returns a list with detection information, such as the flag_detect, cos value and so on.
}
\references{
We use the publicly available Broken Arrows implementation from the Broken Arrows contest.
T. Furon and P. Bas, “Broken arrows,” EURASIP Journal on Information Security, vol. 2008, pp. 1–13, 2008
}
\author{
Erwin Quiring
}
\examples{
# use Lena image that is part of this package
library(ForgottenSiblings)
img = lenaimg

# embed watermark
watimg = embed_watermark(img)

# show images
plotGreymap(img)
plotGreymap(watimg$watermarkedimage)

# detect watermark, here we just use the pixmap...
detect_watermark(img)
detect_watermark(watimg$watermarkedimage)
}

\keyword{ detection }% use one of  RShowDoc("KEYWORDS")
\keyword{ Broken Arrows }% __ONLY ONE__ keyword per line
