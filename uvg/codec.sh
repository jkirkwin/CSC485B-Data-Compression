#!/bin/bash

QUALITY=${1:-medium}
RESULT_DIR=${2:-result_images}

make

echo Using quality level $QUALITY
echo Saving images to $RESULT_DIR
echo ""
echo ""

mkdir -p $RESULT_DIR
rm -f $RESULT_DIR/*

for filename in `find ./test_images -type f`
do
    echo $filename
    FNAME=`basename -s .bmp $filename`

    TEMPFILE=/tmp/$FNAME.bin
    ./uvg_compress $QUALITY $filename $TEMPFILE > /dev/null
    ./uvg_decompress $TEMPFILE $RESULT_DIR/$FNAME.bmp > /dev/null

    OLDSIZE=`wc $filename | awk '{print $3}'`
    NEWSIZE=`wc $TEMPFILE | awk '{print $3}'`

    RATIO=$OLDSIZE/$NEWSIZE
    echo Compressed from $OLDSIZE bytes to $NEWSIZE bytes \($[100*$OLDSIZE/$NEWSIZE ]%\)
    echo "" # Newline
done
