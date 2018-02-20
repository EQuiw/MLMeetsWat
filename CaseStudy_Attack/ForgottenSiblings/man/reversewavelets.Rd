\name{revertWaveletCoefficients}
\alias{revertWaveletCoefficients}
%- Also NEED an '\alias' for EACH other topic documented here.
\title{
Revert wavelet coefficients of a signal
}
\description{
Revert the 2D wavelet transformation of the input signal.
}
\usage{
revertWaveletCoefficients(llcoeffs, hfcoeffs, height, width, verbose=0)
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{llcoeffs}{ LL coefficients}
  \item{hfcoeffs}{ Wavelet coefficients except LL}
  \item{height}{ the height and width of the target image}
  \item{width}{ the height and width of the target image}
  \item{verbose}{ controls console output }
  }

\details{
}
\value{
Returns the image in the pixel space.
}
\references{
We use the publicly available Broken Arrows implementation from the Broken Arrows contest.
T. Furon and P. Bas, “Broken arrows,” EURASIP Journal on Information Security, vol. 2008, pp. 1–13, 2008
}
\author{
Erwin Quiring
}

\examples{

library(ForgottenSiblings)
result = getWaveletCoefficients(lenaimg)
str(result)
lenaimg_recovered = revertWaveletCoefficients(llcoeffs = result$llcoeffs, hfcoeffs = result$hfcoeffs, height=nrow(lenaimg), width=ncol(lenaimg), verbose=0)
plotGreymap(lenaimg_recovered)
}

\keyword{ wavelet }% use one of  RShowDoc("KEYWORDS")
\keyword{ Broken Arrows }% __ONLY ONE__ keyword per line
