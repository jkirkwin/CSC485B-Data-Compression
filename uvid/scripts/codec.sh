#!/bin/bash

# This script takes the raw file to be compressed and decompressed via stdin. 

if [ -f ./uvid_compress -a -f ./uvid_decompress ]
then
    if [ "$#" -ne 3 ]
    then
        >&2 echo "Provide dimensions and quality setting" 
        exit 1
    fi

    width=$1
    height=$2
    quality=$3

    cat - | ./uvid_compress $width $height $quality | ./uvid_decompress

else
    >&2 echo "Cannot locate codec executables. Did you build?" 
    exit 2
fi


