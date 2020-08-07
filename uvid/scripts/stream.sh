#/bin/bash

scripts_dir=`dirname $0`

if [ $# -ne 4 ]
then 
    >&2 echo "Provide input .y4m file, dimensions, and quality setting"
    exit 1 
fi

infile=$1
height=$2
width=$3
quality=$4

$scripts_dir/y4m_to_raw.sh $infile | $scripts_dir/codec.sh $height $width $quality | $scripts_dir/play_raw.sh $height $width 
