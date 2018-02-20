\name{doprep}
\alias{doprep}
\alias{invprep}
\alias{getpreprocessor}

\title{
Preprocessing Routines that do standardization.
}
\description{
Functions that help to learn standardization params of the training data, to perform this standardization on training data and on test data.
It is important that we apply the mean and sd only from the training data on the test set.
}
\usage{
getpreprocessor(xdata)
doprep(xdata, preproc)
invprep(xdata, preproc)
}

\arguments{
  \item{xdata}{
The training data. Can be a matrix or a vector.
}
  \item{preproc}{
Pass the standardization parameters obtained from calling getpreprocessor function.
}
}
\details{
First, call 'getpreprocessor' on the training data matrix. This will return an list object that contains the colmeans and colsd's.
Second, use 'doprep' on the training data together with the object returned from getpreprocessor.
Third, do the same for the test set.
Later, if we want to get the original values for an object, call 'invprep'...

I've implemented these methods here, since it was faster than the packages I've found that also do some standardization.
}
\author{
Erwin Quiring
}
\examples{
# Create arbitrary matrix with 4 rows and 32 columns (would correspond to 4 train samples and 32 features).
mtrain = matrix(sample.int(n = 100, size = 4 * 32, replace = TRUE), nrow = 4, ncol = 32, byrow = TRUE)
# and create a 2nd test set matrix
mtest = matrix(sample.int(n = 100, size = 4 * 32, replace = TRUE), nrow = 4, ncol = 32, byrow = TRUE)

# Learn standardization params on train set
preproc = getpreprocessor(xdata = mtrain)

# Apply the standardization params from train set on both sets.
mtrain_stand = doprep(mtrain, preproc)
mtest_stand = doprep(mtest, preproc)

# Compare
colMeans(mtrain)
colMeans(mtrain_stand)

# Invert
mtest_inv = invprep(mtest_stand, preproc)
}

