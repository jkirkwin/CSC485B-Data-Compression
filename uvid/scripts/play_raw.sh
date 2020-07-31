#!/bin/bash

if [ "$#" -lt 2 ]
then
    >&2 echo "Provide video resolution and optional -s."
    exit 1
fi

width=$1
height=$2

if [ -z "$3" ]
then
  ffplay -f rawvideo -pixel_format yuv420p -framerate 30 -video_size ${width}x${height} -

else if [ "$3" = "-s" ]; then
  ffplay -f rawvideo -pixel_format yuv420p -framerate 30 -video_size ${width}x${height} - 2> /dev/null

else
  >&2 echo "Bad flag given: $3"
  exit 2
fi
fi
