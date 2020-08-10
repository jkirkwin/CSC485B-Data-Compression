#!/bin/bash

# Convert a .y4m video file to the raw format used by the codec via ffmpeg

if [ -z "$1" ]
then
    echo "Provide a file name."
    exit 1
fi

file=$1
>&2 echo "Converting $file to .raw" 

if [ -f "$file" ]
then
    ffmpeg -i $file -f rawvideo -pixel_format yuv420p - 2>/dev/null

    if [ $? -ne 0 ]
    then
        >&2 echo "COMMAND FAILED - unable to convert y4m to raw"
        exit 3
    fi
else 
    >&2 echo "NO SUCH FILE"
    exit 2
fi
