#!/bin/bash

if [ "$#" -ne 2 ]
then
    echo "Provide video resolution."
    exit 1
fi

height=$1
width=$2

ffplay -f rawvideo -pixel_format yuv420p -framerate 30 -video_size ${height}x${width} -
