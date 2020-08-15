# Assignment 5 - Video Compression (`uvid`)
Jamie Kirkwin
CSC 485B: Data Compression

## Description

The build outputs of this directory are `uvid_compress` and `uvid_decompress`: a simple codec for raw video files.
`scripts/` holds useful bash scripts for converting .y4m video files to the raw format used, playing videos in either 
format, running the codec, and mimicking streaming by running the codec with video piped in from and output to `ffplay`

The compression pipeline is fairly simple. A block-based DCT identical to the one used in `uvg` is used to compress 
each frame (see `dct/dct.h`). The quantization matrix used can be scaled using the quality parameters. `low` means more 
aggressive quantization, while `high` means less quantization and a higher fidelity result. This library also re-orders 
the coefficients in a zig-zag pattern similar to JPEG to improve delta compression performance.

A variable-bit encoding scheme is also used in the pipeline. For details, see the documentation in `delta/delta.h`. This
scheme encodes a signed value in a number of bits proportional to the magnitude of the number.

I-Frames are encoded using these two techniques and some simple delta compression to decrease the size of the AC 
coefficients.

P-Frames are encoded using the same mechanisms as I-Frames, with the addition of a header section for each 16x16 section 
("macroblock") of the original image (corresponding to a 16x16 pixel block of the Y plane and 8x8 chunks of the 
sub-sampled Cb and Cr planes) indicating:
    (a) if the macroblock is encoded without modification or if it is predicted
    (b) if the macroblock is predicted, then a vector from the top left corner of the macroblock to the top-left corner 
    of the portion of the previous frame used for prediction is stored.
Predicted macroblocks in P-frames are just the result of subtracting the value of each pixel in some section of the 
previous frame from the macroblock in the current frame. Macroblocks on the edge of the image may be smaller than 16x16/
8x8.

To find the motion vector to be used, a local search is performed. Average Absolute Difference (AAD) is the metric used 
to determine the quality of a possible predictive match in the previous frame. First, the same location in the previous 
frame is checked. Then, at increasing intervals around the top-left corner of the current macroblock, matches in the 8 
cardinal directions are checked.
If, at any point a prospective match gives a very low AAD score, the algorithm stops and that match is used. If such a 
low AAD match is not found, then the best one out of all those found is saved.
If the resulting match does not have an AAD of less than half of the average magnitude of the original pixels in the 
macroblock, then prediction is not used. If the AAD does satisfy this inequality, then the values from the previous 
image are subtracted from the macroblock's pixels before performing the DCT and delta compression as described for 
I-Frames.

A single I-Frame is sent to begin the stream. All subsequent frames are P-Frames.

## Bitstream

### File Header
The height and width of the frames are encoded in 32 bits each, least-significant-bit first.
Then, there is a 2-bit encoding of the quality setting used by the compressor (00 for low, 10 for medium, and 01 for 
high)

### Frame header/trailer
Before every frame in the video, a single 1 bit is inserted to indicate that there is still at least one more frame to 
decode. After the final frame, a single 0 bit must be written.

### Frame Blocks
The DCT blocks for I- and P-Frames are encoded identically (recall that a P-frame may just have different values as 
inputs to the DCT process). Each block is a sequence of <= 64 coefficients.

The DC and first AC coefficient are sent as 32-bit literals (lsb to msb)
Each of the remaining AC coefficients (up to 62 of them) is encoded as variable-bit encoding of the difference between 
it and the preceding AC coefficient. See `delta/delta.h` for a description of this scheme.

### I-Frames
I-Frames are encoded simply as a sequence of these frame blocks (preceded by a single continuation bit).
The blocks are ordered from top-left to bottom-right in row-major order for each of the three planes.
The Y plane is sent first, followed by the Cb plane, followed by the Cr plane.

### P-Frames
P-Frames are encoded similarly to I-Frames, but with a sequence of macroblock headers before the encoded blocks.
Each macro-block header corresponds to a 16x16 chunk of the initial image, which is a 16x16 block of the Y plane and 
an 8x8 block of the sub-sampled Cb and Cr planes.

For each macroblock in row-major order, its header is encoded.

After the sequence of macroblock headers (see below), the contents of the 3 planes (some of which may be the original 
data and some predicted) are encoded as DCT blocks as described above for I-Frames. 

#### Macroblock header
The macroblock header has the following structure:
1. A one-bit flag indicating if the macroblock is predicted using the previous frame or not.
2. If the prediction flag is set to 1, then a motion vector field is included as well. The x-component of the motion
vector is sent first, followed by the y-component. Each of these is encoded using the same variable-bit scheme discussed 
earlier. See `delta/delta.h` for details. 

The motion vector is stored as the difference between the top-left position of the macroblock and the top-left position 
of the chunk of the previous frame used to take the diff. __It is measured in the units of the colour planes__ to save 
space, and can be multiplied by two to find the Y-Plane equivalent. 
