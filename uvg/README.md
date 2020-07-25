# Assignment 4 - Imagae Compression (`uvg`)
Jamie Kirkwin

## Description

The build outputs of this directory are `uvg_compress` and `uvg_decompress`: 
a JPEG-esque compressor and decompressor for .bmp images.

The encoding process consists of
- Converting the .bmp input from RGB to YCbCr format
- Scaling (sub-sampling) the Cb and Cr colour planes down to one quarter of their original size
- Performing a block-based Discrete Cosine Transform (DCT) on each of the three planes to produce sets of real-valued coefficients
- Quantizing each block of the DCT results using the recommended coefficient matrixes from the JPEG standard
- Using a delta compression algorithm to compress each block of quantized coefficients

## Bitstream

The bitstream is quite simple.

There is an 8-byte-and-2-bit header for the entire file, consisting of
- a height field (4 bytes)
- a width field (4 bytes)
- a quality parameter  (2 bits)

The height and width give the dimensions of the original image.
The quality parameter can be used to select high, medium, or low quality. 
Higher quality means a larger compressed file size, but higher fidelity to 
the original image after decompression.

After the header come a sequence of blocks. Each block represents a chunk of
one plane of the YCbCr image. Recall that the Cb and Cr planes have been 
sub-sampled to a quarter of their original size.

Each block is of size 64, representing an 8x8 square from one of the planes. 
The file format for images which do not divide evenly into such blocks may be 
slightly inflated as dummy data is added and all blocks sent are of size 64.

Each encoded block consists of a DC coefficient in 4 bytes, followed by an AC 
coefficient in 4 bytes, followed by the other 62 AC coefficients encoded as deltas
using a variable bit encoding scheme.

The coefficients are ordered in a "Zig-Zag" pattern from the top left of the quantized
DCT result matrix, alternating along the diagonals, ending with the bottom right coefficient.
This is the same pattern used by JPEG.

## Sources
- [Matrix Multiplication](https://mathworld.wolfram.com/MatrixMultiplication.html)
- [zig zag pattern](https://medium.com/100-days-of-algorithms/day-63-zig-zag-51a41127f31)
