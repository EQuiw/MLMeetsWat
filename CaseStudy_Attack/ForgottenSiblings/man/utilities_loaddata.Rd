\name{get_my_datasets}
\alias{get_my_datasets}
\alias{just_load_fulldataset}

\title{
Load data.
}
\description{
Functions to load the training and test data (as saved by the Rscript 2_getdata.R).
}
\usage{
get_my_datasets(outputdir, database, suffix, splitratio = 0.2, randomseed = 41, shuffletrain = T, removepairstrain = F)
just_load_fulldataset(outputdir, database, suffix)
}

\arguments{
  \item{outputdir}{the path where the RData files are located}
  \item{database}{the database name}
  \item{suffix}{the database suffix}
  \item{splitratio}{ratio for splitting the training and test set}
  \item{randomseed}{a random seed for shuffling the training set}
  \item{shuffletrain}{set True if the training set should be shuffled. Necessary for learning.}
  \item{removepairstrain}{True if we want to remove the correponding unmarked image for each marked image and vice versa.
  In other words, by using True, we will not have the marked counterpart for each unmarked image. We will have two different sets of images for training.}
}
\value{
Returns a list with the training, test set.
  \item{xtrainll }{the LL coefficients training data}
  \item{xtrain}{the HF coefficients training data}
  \item{ytrain}{the corresponding y labels. Should have two columns. The first indices watermark presence with 1, absence with 0. The 2nd column is the opposite.}
  \item{xtestll}{the LL coefficients test data. Might be empty if splitratio is 0.}
  \item{xtest}{the HF coefficients test data. Might be empty if splitratio is 0.}
  \item{ytest}{the corresponding y labels.}
}
\author{
Erwin Quiring
}
\note{
Note that this function may need some time. Look at 2_getdata.R to learn more what format this function expects and look at 3_attack.R to learn more how
you can use this function.
}
