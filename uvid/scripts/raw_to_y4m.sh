#!/bin/bash

# Convert from the raw format used by the codec back to .y4m via ffmpeg

if [ "$#" -ne 3 ]
then
    echo "Provide output file name and video resolution."
    exit 1
fi

outfile=$1
width=$1
height=$2

>&2 echo "Converting raw file with dimensions ${width}x${height} to y4m format. Saving as ${outfile}."

ffmpeg -f rawvideo -pixel_format yuv420p -framerate 30 -video_size ${width}x${height} -i - -f yuv4mpegpipe $outfile

if [ $? -ne 0 ]
then
    >&2 echo "COMMAND FAILED" 
    exit 3
fi






