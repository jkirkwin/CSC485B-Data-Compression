#!/bin/bash

mkdir -p result_images
rm -f result_images/*

for filename in `find ./test_images -type f`
do
    echo Encoding and decoding $filename
    fname=`basename -s .bmp $filename`
    echo $fname.bmp
    # Todo change to use make and current dir once we get that far
    cmake-build-debug/uvg_compress nill $filename /tmp/$fname.bin
    cmake-build-debug/uvg_decompress /tmp/$fname.bin result_images/$fname.bmp
    echo "" # Newline
done
