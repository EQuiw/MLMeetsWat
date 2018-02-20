\name{embed_watermark}
\alias{embed_watermark}
%- Also NEED an '\alias' for EACH other topic documented here.
\title{
Watermark Embedding via Broken Arrows
}
\description{
Embed a watermark via Broken Arrows in a given image.
}
\usage{
embed_watermark(pixmap, verbose=0, watermarkkey = c(0,0,0,0))
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{pixmap}{ a 2-dim matrix that contains the pixel values in a range from [0,255].}
  \item{verbose}{ set 1 if status updates are wanted.}
  \item{watermarkkey}{ the watermark key, 4 dim vector with integer values from 0 to 2^31}
  }
\details{
%%  ~~ If necessary, more details than the description above ~~
}
\value{
Returns a list with currently only one element, in future, we may return more information via the list.
\item{watermarkedimage }{The watermarked image from pixmap, a matrix.}
%%  \item{comp2 }{Description of 'comp2'}
%% ...
}
\references{
We use the publicly available Broken Arrows implementation from the Broken Arrows contest.
T. Furon and P. Bas, “Broken arrows,” EURASIP Journal on Information Security, vol. 2008, pp. 1–13, 2008
}
\author{
Erwin Quiring
}
\note{
%%  ~~further notes~~
}

%% ~Make other sections like Warning with \section{Warning }{....} ~

\seealso{
%% ~~objects to See Also as \code{\link{help}}, ~~~
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

# detect watermark
detect_watermark(img)
detect_watermark(watimg$watermarkedimage)
}

\keyword{ embedding }% use one of  RShowDoc("KEYWORDS")
\keyword{ Broken Arrows }% __ONLY ONE__ keyword per line
