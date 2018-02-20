\name{readImage}
\alias{readImage}

\title{
Utilities Bitmaps. Load image.
}
\description{
Reads an image from a given source.
}
\usage{
readImage(x, pathToImages, resizev = NA, resizeh = NA, offsetv = 1, offseth = 1)
}

\arguments{
  \item{x}{
the file name
}
  \item{pathToImages}{
the path to the file, should end with /
}
  \item{resizev}{
Used to crop images, if NA, no cropping. Set e.g. 64 to have a new height of 64 px.
}
  \item{resizeh}{
Used to crop images, if NA, no cropping. Set e.g. 64 to have a new width of 64 px.
}
  \item{offsetv}{
the row offset of the upper left corner.
}
  \item{offseth}{
the column offset of the upper left corner.
}
}
\author{
Erwin Quiring
}
