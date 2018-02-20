\name{do_watermark_attack}
\alias{do_watermark_attack}

\title{
Attack. Crafts adversarial sample.
}
\description{
Conducts the gradient descent on the given model towards the wanted class. Various settings are possible.
}
\usage{
do_watermark_attack(sess, model, adv_x, adv_x_ll, nofeatures, attack_settings, eps, costloss, preproc, mysize, ref_sample = NA, verbose = 1)
}
%- maybe also 'usage' for other objects documented here.
\arguments{
  \item{sess}{
The current tensorflow session.
}
  \item{model}{
The Keras model.
}
  \item{adv_x}{
The current sample where we want to remove the watermark. Pass the HF wavelet coefficients.
}
  \item{adv_x_ll}{
The current sample where we want to remove the watermark. Pass the LL wavelet coefficients.
}
  \item{nofeatures}{
The number of features.
}
  \item{attack_settings}{
A list with various parameters that define how we conduct the attack, e.g. the attack strategy, ...
}
  \item{eps}{
The gradient descent step width.
}
  \item{costloss}{
The c parameter that balances the distance penalty and the prediction score.
}
  \item{preproc}{
preprocessor object. see doprep...
}
  \item{mysize}{
the height/width of the image. necessary to revert the wavelet coefficients into the correct image format.
}
  \item{ref_sample}{
the reference sample for the distance term.
}
  \item{verbose}{
console output. Use verbose = 1, or = 2, or = 3 to control the number of outputs. A higher number leads to detailed output.
}
}
\details{
Look at the 3_attack.R script to get a better intuition how to use this method. This function is the result of some experiments and could be implemented in an easier way.
}
\value{
Will return a list with the final adversarial sample, its detector output and its neural network prediction.
%%  ~Describe the value returned
%%  If it is a LIST, use
%%  \item{comp1 }{Description of 'comp1'}
%%  \item{comp2 }{Description of 'comp2'}
%% ...
}

\author{
Erwin Quiring
}

\seealso{
The best thing you can do is to read our paper, the source code and to look at our 3_attack.R script.
}

