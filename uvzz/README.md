# Assignment 3 - Custom Compression Pipeline (`uvzz`)

## Compression Pipeline Design
The pipeline implemented here is based on and aims to out-perform BZip2.

Data is read in blocks of a fixed size. 
Each block is then passed through each stage of the pipeline:

0. A Burrows-Wheeler transform is run on the block to produce a permuted, compressable version of the data and an index needed for decompression.
0. A Move-to-Front transform is performed next to prepare the block for the following specialized RLE step.
0. The RLE component comes next, encoding only runs of zeros. This is effective because the previous Move-To-Front transform results in runs of zeros whenever its input contains runs of any symbol.
0. Finally, entropy coding is done with Arithmetic Coding.

## Implementation

__todo__ Document and justify the algorithms and approaches used for each pipeline component.

### Move-to-Front 
The Move-to-Front transform implementation (see `mtf/mtf.cpp`) is very simple. I opted to use an array-based index table
to increase the locality of the memory needed compared to a `std::list` in the hopes of better cache performance. A list
would have improved the speed of the interior deletions at the cost of increased time to traverse and find the element 
to delete when moving a symbol to the front of the table. The actual implementation is very simple and can be understood
easily from the code and inline documentation.

## Bitstream

__todo__ Design and document a bitstream that carries all necessary metadata and the final encoded data stream.
- Need magic number
- Use CRC?
- Need BWT index
- May need arithmetic coding freq table depending on design choices (e.g. dynamic vs not)

## Sources

__todo__ add all sources used as we go.

### Move-to-Front
https://en.wikipedia.org/wiki/Move-to-front_transform
