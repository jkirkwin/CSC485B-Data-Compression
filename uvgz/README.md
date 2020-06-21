# Assignmnet 2 - GZip Compressor (`uvgz`)

## Design/Implementation Overview
I decided to go with a similar breakdown as for assignment 1 in terms of the design.
There are a handful of static libraries (see below) that allow for some minimal modularity and unit testing.
The LZSS-related functionality and logic lives inside an "Encoder" abstraction (similarly to the encoders in my 
assignment 1 submission) and a similar encoder abstraction wraps the entire GZip/DEFLATE implemenatation (the 
imaginatively name GzipEncoder).

The major chunks that I kept as separate as possible are:
* A prefix coding library which deals with Huffman/Package-Merge and generating codes for an LZSS symbol stream.
* An LZSS library which supplies a transparent LZSS implementation to turn raw data into a stream of literals and 
backreferences
* The output stream implementation provided by bill
* A GZip library which supplies an implementation of the afformentioned GZip Encoder and uses the other two libraries
and the output stream based on the GZip spec.

## Dir Structure
Concerns are roughly broken up into:
* GZip file, member, and block structure -> `gzip.cpp/h`
* LZSS Encoding -> `lzss.cpp/h`, `lzss_backref.h`
* Prefix coding -> `prefix.cpp/h`
* Running the code -> `main.cpp`

I wanted to have a separate CMake build and subdirectory for each library but I was wasting too much time fighting with
it so please forgive the monolith project root folder.

## Deflate Compression Decisions
There are a crazy amount of different decisions that a DEFLATE compressor can make to try to minimize output size.

The initial architecture decisions mean that the LZSS implementation is transparent to the GzipEncoder, and decisions on
the selection of backreferences must be left up to it.

### Choosing LZSS back-references
A very simple, very naive linear search is used by the LzssEncoder in order to find the most recent usable backreference
of sufficient length. 

### Choosing block boundaries
Every block is a fixed size, except the last one which is whatever size we have left over.

### Choosing block types
Block type 2 is always used.

## Notes for the marker
Please excuse the difference in naming conventions etc between Bill's code, mine, and the CRC and boost libraries! 

### Rubric Items
* You're reading the README so that should be covered (as long as this is detailed enough). The code is also fairly 
well documented.
* I've implemented all the block types and compute backreferences using a window of ~5000 bytes.
* For type 2 Dynamic LL and Distance codes are generated based on LZSS symbol frequency, but it's done with 
Package-Merge, not normal Huffman coding.
