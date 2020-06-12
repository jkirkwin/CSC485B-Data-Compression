# Assignmnet 2 - GZip Compressor (`uvgz`)

## Initial strategy
I'm a little scared about the scale of complexity involved here. Most of it comes from the myriad of decisions available
to the compressor and the incentives (i.e. marks available) for performance - both time and compression.

Since I'm not sure if I'll have the time needed to finish if I jump in and try to be clever in the beginning. As such I
am going to aim for a fairly simple implementation to start and if I have any time left over before the due date then 
optimizations can be refactored in.

## Design/Implementation Overview
<!-- TODO -->

## Dir Structure
Concerns are roughly broken up into:
* GZip file, member, and block structure -> `gzip.cpp/h`
* LZSS Encoding -> `lzss.cpp/h`, `lzss_backref.h`
* Prefix coding -> `prefix.h`

## Deflate Compression Decisions
There are a crazy amount of different decisions that a DEFLATE compressor can make to try to minimize output size.

<!-- TODO -->

### Choosing LZSS back-references
<!-- TODO -->

### Choosing block boundaries
Every block will be a fixed size, except the last one which will be whatever size we have left over.

### Choosing block types
Block types will be chosen based on the success of the LZSS scheme for a given chunk of data. If little compression
is expected, then a type 0 block will be used.

Otherwise, if large savings are expected through the use of dynamic codes, then a type 2 block is used.

Otherwise, a type 1 block is used.

## Notes for the marker
Please excuse the difference in naming conventions etc between Bill's code, mine, and the CRC and boost libraries! 

<!-- TODO -->