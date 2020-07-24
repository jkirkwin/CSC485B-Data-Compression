#!/bin/bash

QUALITY=${1:-med}
RESULT_DIR=${2:-result_images}

echo Using quality level $QUALITY
echo Saving images to $RESULT_DIR

mkdir -p $RESULT_DIR
rm -f $RESULT_DIR/*

for filename in `find ./test_images -type f`
do
    echo Encoding and decoding $filename
    fname=`basename -s .bmp $filename`
    echo $fname.bmp
    tempfile=/tmp/$fname.bin
    # Todo change to use make and current dir once we get that far
    cmake-build-debug/uvg_compress $QUALITY $filename $tempfile
    cmake-build-debug/uvg_decompress $tempfile $RESULT_DIR/$fname.bmp
    echo "" # Newline
done
