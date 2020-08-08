/* uvid_decompress.cpp
   CSC 485B/578B/SENG 480B - Data Compression - Summer 2020

    todo update this docstring
   Starter code for Assignment 5

   This placeholder code reads the (basically uncompressed) data produced by
   the uvid_compress starter code and outputs it in the uncompressed 
   YCbCr (YUV) format used for the sample video input files. To play the 
   the decompressed data stream directly, you can pipe the output of this
   program to the ffplay program, with a command like 

     ffplay -f rawvideo -pixel_format yuv420p -framerate 30 -video_size 352x288 - 2>/dev/null
   (where the resolution is explicitly given as an argument to ffplay).

   B. Bird - 07/15/2020
   J. Kirkwin - 08/07/2020
*/

#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <cassert>
#include <cstdint>
#include <tuple>
#include "input_stream.hpp"
#include "yuv_stream.hpp"
#include "dct/dct.h"
#include "delta/delta.h"

dct::encoded_block_t readEncodedBlock(InputBitStream& inputBitStream) {
    dct::encoded_block_t block;
    // todo decode delta encoding
    for (u32 i = 0; i < dct::BLOCK_CAPACITY; ++i) {
        block.push_back(inputBitStream.read_u32());
    }

    return block;
}

std::vector<dct::encoded_block_t> readEncodedBlocks(unsigned int n, InputBitStream& inputBitStream) {
    std::vector<dct::encoded_block_t> blocks(n);
    for (int i = 0; i < n; ++i) {
        blocks.at(i) = readEncodedBlock(inputBitStream);
    }
    return blocks;
}

unsigned int numBlocksToRead(unsigned int height, unsigned int width) {
    //https://stackoverflow.com/questions/2745074/fast-ceiling-of-an-integer-division-in-c-c
    assert (height > 0 && width > 0);
    auto verticalPartitions = width / dct::BLOCK_DIMENSION + (width % dct::BLOCK_DIMENSION != 0);
    auto horizontalPartitions = height / dct::BLOCK_DIMENSION + (height % dct::BLOCK_DIMENSION != 0);
    auto result = verticalPartitions * horizontalPartitions;
    return result;
}

int main(int argc, char** argv){

    //Note: This program must not take any command line arguments. (Anything
    //      it needs to know about the data must be encoded into the bitstream)
    
    InputBitStream input_stream {std::cin};

    u32 height {input_stream.read_u32()};
    u32 width {input_stream.read_u32()};

    u32 scaledHeight = height/2; // todo used +1 in uvg?
    u32 scaledWidth = width/2;

    YUVStreamWriter writer {std::cout, width, height};

    while (input_stream.read_byte()){ // Reading the single-byte continuation flag
        YUVFrame420& frame = writer.frame();

        // Read in the quantized coefficients for each plane
        auto luminanceBlockCount = numBlocksToRead(height, width);
        auto yBlocks = readEncodedBlocks(luminanceBlockCount, input_stream);

        auto colourBlockCount = numBlocksToRead(scaledHeight, scaledWidth);
        auto cbBlocks = readEncodedBlocks(colourBlockCount, input_stream);
        auto crBlocks = readEncodedBlocks(colourBlockCount, input_stream);

        // Invert the DCT/quantization
        const auto qualityLevel = dct::med; // todo pull this from the bitstream
        auto yContext = dct::luminanceContext(height, width);
        auto yPlane = dct::invert(yBlocks, yContext, qualityLevel);
        auto colourContext = dct::chromananceContext(scaledHeight, scaledWidth);
        auto cbPlane = dct::invert(cbBlocks, colourContext, qualityLevel);
        auto crPlane = dct::invert(crBlocks, colourContext, qualityLevel);

        // Construct and write the frame
        for (u32 y = 0; y < height; y++) {
            for (u32 x = 0; x < width; x++) {
                frame.Y(x,y) = yPlane.at(y,x);
            }
        }
        for (u32 y = 0; y < scaledHeight; y++) {
            for (u32 x = 0; x < scaledWidth; x++) {
                frame.Cb(x,y) = cbPlane.at(y,x);
                frame.Cr(x,y) = crPlane.at(y,x);
            }
        }
        writer.write_frame();
    }


    return 0;
}