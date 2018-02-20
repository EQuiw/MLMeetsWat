\name{getWaveletCoefficients}
\alias{getWaveletCoefficients}
%- Also NEED an '\alias' for EACH other topic documented here.
\title{
Get wavelet coefficients of a signal
}
\description{
Get the 2D wavelet transformation of the input signal (Daubechies 9/7) on three levels of decomposition.
Output is separated into two vectors.
HF is the output from all wavelet coefficients except the low-frequency LL-band.
LL is the rest.
}
\usage{
getWaveletCoefficients(pixmap)
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{pixmap}{ a 2-dim matrix that contains the pixel values in a range from [0,255].}
  }

\details{
}
\value{
A list with LL and HF coefficients.
\item{llcoeffs }{the LL wavelet coefficients}
\item{hlcoeffs }{All wavelet coefficients except the LL band}
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

}

\keyword{ wavelet }% use one of  RShowDoc("KEYWORDS")
\keyword{ Broken Arrows }% __ONLY ONE__ keyword per line
