\name{get_src_imageindex_andpair}
\alias{get_src_imageindex_andpair}
%- Also NEED an '\alias' for EACH other topic documented here.
\title{
Attack. Get test set indices of the marked image and its unmarked counterpart (or vice versa).
}
\description{
If the test set is not sorted/shuffled, then you can specify an index and this method
will return the corresponding unmarked version from the test set.
Internally, if you say iterationc = 3, it will pick the third image from the test set
and the n+3 image where n is the first position where the label is changing.
Thus, it assumes that test set contains of only marked images first, then only unmarked images.
It is just a convenient helper function, you do not have to use it.
}
\usage{
get_src_imageindex_andpair(ytest, iterationc, attack_settings)
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{ytest}{
the label matrix
}
  \item{iterationc}{
the wanted index
}
  \item{attack_settings}{
attack settings list. See 3_attack.R
}
}
