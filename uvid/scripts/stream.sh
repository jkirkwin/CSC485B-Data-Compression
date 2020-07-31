#/bin/bash

scripts_dir=`dirname $0`

if [ $# -ne 4 ]
then 
    >&2 echo "Provide input .y4m file, dimensions, and quality setting"
    exit 1 
fi

infile=$1
width=$2
height=$3
quality=$4

$scripts_dir/y4m_to_raw.sh $infile | $scripts_dir/codec.sh $width $height $quality | $scripts_dir/play_raw.sh $width $height
