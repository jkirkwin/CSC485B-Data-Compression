# Assignment 2 - GZip Compressor (`uvgz`)

## Sources
Almost all of the code here is my own. I used a couple of StackOverflow posts, mostly for finding out the most 
C-plus-plus way to do things with iterators and vectors. These places are linked in the code where I used them.

I also made heavy use of the code Bill provided, but its pretty split up throughout `prefix.cpp` and `gzip.cpp`. 
As with the SO references, there are comments in the functions that were adapted from the starter code.

Finally, I used Boost and the provided CRC library.

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
* The output stream implementation provided by Bill
* A GZip library which supplies an implementation of the afformentioned GZip Encoder and uses the other two libraries
and the output stream based on the GZip spec.

## Dir Structure
Concerns are roughly broken up into:
* GZip file, member, and block structure -> `gzip.cpp/h`
* LZSS Encoding -> `lzss.cpp/h`, `lzss_backref.h`
* Prefix coding -> `prefix.cpp/h`
* Running the code -> `main.cpp`
* Provided bitstream -> `shared/`

I wanted to have a separate CMake build and subdirectory for each library but I was wasting too much time fighting with
it so please forgive the monolith project root folder.

## Deflate Compression Decisions
There are a crazy amount of different decisions that a DEFLATE compressor can make to try to minimize output size.

The initial architecture decisions mean that the LZSS implementation is transparent to the GzipEncoder, and decisions on
the selection of backreferences must be left up to it (i.e. you can't tell the LZSS machinery to optimize backref 
choices for a dynamic prefix code later in the pipeline).

### Choosing LZSS back-references
A very simple, very naive linear search is used by the LzssEncoder in order to find the most recent usable backreference
of sufficient length. I __really__ wanted to have a go at the bastardized hash table approach but I didn't have time - 
maybe something for my assignment 3.

### Choosing block boundaries
Every block is a fixed size, except the last one which is whatever size we have left over.

### Choosing block types
__Summary:__
* Very short input -> type 1
* LZSS ineffective -> type 0
* Short input (LZSS Effective) -> type 1
* All others -> type 2

Block type 2 is preferred for most inputs.

If the input is very short then block 1 is used since it has the least overhead.

Since the LZSS-ing is done in chunks we can determine if there is likely to be expansion from using it by comparing the 
number of LZSS symbols in the encoding to the number of bytes in the original data. The tolerated expansion factor of 
1.2 LZSS items (where backreferences are 4 items) per input byte was determined by benchmarking with provided test data. 
If the LZSS output is larger than this then it is assumed that LZSS will not be of much help and block type 0 is used.

Otherwise, we choose between types 1 and 2. Type 2 seems to perform better in most non-trivial cases so a second 
threshold is set such that any input that is at least that long will use type 2. Otherwise the overhead may be too great 
and type 1 is used instead.

## Notes for the marker
Please excuse the difference in naming conventions etc between Bill's code, mine, and the CRC and boost libraries! 

### Meeting the rubric
* I think I've covered the 4 basic requirements
    * You're reading the README so that should be covered (as long as this is detailed enough). For implementation-level 
    documentation please read the code. 
    * I've implemented all the block types and backreferences are computed using a window of ~5000 bytes.
    * Type 2 blocks use dynamic LL and Distance codes generated using the symbol frequencies (it's done with 
    Package-Merge, not regular Huffman)
* Other items
    * You'll find the package merge implementation in `prefix.cpp` and it should be mostly self-documenting. I used the 
    Sayood book's description and example to build it out, and had to take care of some edge cases that aren't addressed
    there.
    * For type 2 blocks, the CL code RLE functionality is used and the CL Prefix code is generated dynamically. HLIT, 
    HDIST and HCLEN are all used to decrease overhead as well.
    * As discussed above, I do some optimization to choose block types, but I don't do anything clever to choose sizes.
    * I didn't quite do better than `gzip -1` on the provided test files but my encoder's performance is comparable on 
    all of them (and better on 8 of the 34) and runs in under 15 seconds on the reference server.
