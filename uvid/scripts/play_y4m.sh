#!/bin/bash

scripts_dir=`dirname $0`

if [ "$#" -ne 3 ]
then
    echo "Provide input file and video resolution."
    exit 1
fi

file=$1
width=$2
height=$3

$scripts_dir/y4m_to_raw.sh $file | $scripts_dir/play_raw.sh $width $height
