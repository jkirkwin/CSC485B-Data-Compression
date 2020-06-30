# Assignment 3 - Custom Compression Pipeline (`uvzz`)

## Compression Pipeline Design
The pipeline implemeneted here is based on and aims to out-perform BZip2.

Data is read in blocks of a fixed size. 
Each block is then passed through each stage of the pipeline:

0. A Burrows-Wheeler transform is run on the block to produce a permuted, compressable version of the data and an index needed for decompression.
0. A Move-to-Front transform is performed next to prepare the block for the following specialized RLE step.
0. The RLE component comes next, encoding only runs of zeros. This is effective because the previous Move-To-Front transform results in runs of zeros whenever its input contains runs of any symbol.
0. Finally, entropy coding is done with Arithmetic Coding.

## Implementation

__todo__ Document and justify the algorithms and approaches used for each pipeline component.

## Bitstream

__todo__ Design and document a bitstream that carries all necessary metadata and the final encoded data stream.
- Need magic number
- Use CRC?
- Need BWT index
- May need arithmetic coding freq table depending on design choices (e.g. dynamic vs not)

## Sources

__todo__ add all sources used as we go.
