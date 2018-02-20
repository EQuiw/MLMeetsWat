#!/bin/bash

### This file just creates various smaller images from a given source directory that contains png, jpg or pgm files.
### These are the images we use for our experiments.
### set the src where the images are located.
### set the target where the new cropped images should be saved
### set cropsize to adjust the new image size, in cropimageshelper.sh you can adjust the offsets (the upper left corner of the cropped part)



CMD_DIR=`dirname $(realpath $0)`

# Settings, src and target without ending /
SRC="/pathtoimages"

TARGET="/pathtodirectory_where_we_want_to_save_the_images"

MAX_TIME=3000
MAX_JOBS=8

CROPSIZE=128

# Start...
mkdir -p "${TARGET}"

find "${SRC}/" -type f -regex '\(.*png\|.*jpg\|.*pgm\)' | parallel --bar --timeout ${MAX_TIME} -j ${MAX_JOBS}\
            ${CMD_DIR}/cropimageshelper.sh {} {/} ${TARGET} ${CROPSIZE} > ${CMD_DIR}/parsed_1cropsimages.txt


