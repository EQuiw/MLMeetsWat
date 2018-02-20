#!/bin/bash

if [ $# -ne 4 ];
then
  echo "usage: $0 filepath filename target cropsize"
  exit 1
fi

IMG=$1
IMGNAME=$2
TARGET=$3
NEWSIZE=$4

# Define OFFSETsX, OFFSETsY

OFFSETsX=( 256 768 1024 1568 )
OFFSETsY=( 256 768 1024 1568 )


cp ${IMG} "${TARGET}/${IMGNAME}"
IMGSUFFIX=${IMGNAME##*.}
IMGBASE=${IMGNAME%.*}

echo ${IMG}

for OFFSETX in "${OFFSETsX[@]}"
do
    for OFFSETY in "${OFFSETsY[@]}"
    do
    echo "Offsets:"$OFFSETX":"$OFFSETY":"

    origimg=${TARGET}/${IMGBASE}"_"$OFFSETX"_"$OFFSETY"."${IMGSUFFIX}
    
    # crop
    convert "${TARGET}/${IMGNAME}" -crop ${NEWSIZE}x${NEWSIZE}+$OFFSETX+$OFFSETY $origimg

    done
done

# clean
rm "${TARGET}/${IMGNAME}"



