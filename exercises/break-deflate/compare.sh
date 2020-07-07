#!/bin/bash
FILENAME=$1

gzip -9 < $FILENAME | wc -c
bzip2 -9 < $FILENAME | wc -c