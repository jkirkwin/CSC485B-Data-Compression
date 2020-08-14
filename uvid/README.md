# Assignment 5 - Video Compression (`uvid`)
Jamie Kirkwin

## Description

The build outputs of this directory are `uvid_compress` and `uvid_decompress`: 
a simple codec for raw video files.

`scripts/` holds useful bash scripts for converting .y4m video files to the raw format, playing video in either format, 
running the codec, and mimicking streaming by running the codec with video piped in and output piped to `ffplay`

The compression pipeline is fairly simple. A block-based DCT identical to the one used in `uvg` is used to compress 
each frame.

The first frame is encoded using only this technique. All other frames are encoded as simple P-Frames predicted based on 
the __decoded__ version of the previous frame sent/received.

P-Frames consist of the normal sequence of blocks, prefixed by a sequence of block headers which indicated whether each 
block was predicted, and if so, how.

## Bitstream

Each block is encoded the same as in `uvg`:
- A DCT is performed, followed by quantization
- The DC and AC coefficients are sent as 32-bit literals, while the remaining 62 coefficients are encoded as variable-bit 
encodings of the difference between consecutive coefficients. See `delta/delta.h` for more information.

I-Frames are encoded simply as a sequence of these blocks.

P-Frames are encoded the same as I-Frames, but with a sequence of "macro-block" headers before the encoded blocks.
Each macro-block header corresponds to a 16x16 chunk of the initial image, which is a 16x16 block of the Y plane and 
an 8x8 block of the sub-sampled Cb and Cr planes.
The macroblock header has the following structure:
- A one-bit flag indicating if the macroblock is predicted using the previous frame.
- If the prediction flag is set, then there are two more fields which indicate the section of the previous image which was
used to predict the macroblock. The first field is the x-component of the motion vector and the second is the y-component.
Each of these is encoded using the same variable-bit scheme as the DCT coefficients (after the first AC coefficient). 
See `delta/delta.h` for details. 
The motion vector is computed to be inside the subsampled cb/cr planes so that it may be kept as small as possible. 
It must be scaled to be used to decode the Y-component of a predicted macro-block.

After each frame there is a single bit which indicates whether there are further video frames to be decoded. This is 
only set to 0 after the final frame of video.
