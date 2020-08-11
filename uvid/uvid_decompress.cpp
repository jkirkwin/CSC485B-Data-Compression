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
#include "uvid_decode.h"

dct::QualityLevel getQualityLevel(InputBitStream &inputBitStream) {
    auto encoded = inputBitStream.read_bits(2);
    assert (encoded != 3);
    if (encoded == 0) {
        return dct::low;
    }
    else if (encoded == 1) {
        return dct::med;
    }
    else {
        return dct::high;
    }
}

dct::encoded_block_t readEncodedBlock(InputBitStream& inputBitStream) {
    dct::encoded_block_t block;

    // The DC and first AC coefficients are sent as literals in 4 bytes.
    const int dc = inputBitStream.read_u32();
    block.push_back(dc);
    const int ac0 = inputBitStream.read_u32();
    block.push_back(ac0);

    // The rest are encoded as deltas
    int prev = ac0;
    for (int i = 2; i < dct::BLOCK_CAPACITY; ++i) {
        int diff = delta::decodeFromBitStream(inputBitStream);
        int cur = prev + diff;
        block.push_back(cur);

        prev = cur;
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

decode::CompressedIFrame readIFrame(InputBitStream& inputBitStream, u32 height, u32 width) {
    // Read in the quantized coefficients for the Y plane
    auto luminanceBlockCount = numBlocksToRead(height, width);
    auto yBlocks = readEncodedBlocks(luminanceBlockCount, inputBitStream);

    // Read in the quantized coefficients for the Cb and Cr planes
    auto colourBlockCount = numBlocksToRead(height/2, width/2);
    auto cbBlocks = readEncodedBlocks(colourBlockCount, inputBitStream);
    auto crBlocks = readEncodedBlocks(colourBlockCount, inputBitStream);

    // Package up and return the result
    return decode::CompressedIFrame(height, width, yBlocks, cbBlocks, crBlocks);
}

int main(int argc, char** argv) {
    InputBitStream input_stream {std::cin};

    // Read header fields
    u32 height {input_stream.read_u32()};
    u32 width {input_stream.read_u32()};
    auto qualityLevel = getQualityLevel(input_stream);

    YUVStreamWriter writer {std::cout, width, height};

    // Read each frame, decode it, and write it out
    while (input_stream.read_byte()){ // Check the single-byte continuation flag
        auto iFrame = readIFrame(input_stream, height, width);
        auto& resultFrame = writer.frame();
        decode::IFrameToYCbCr(iFrame, resultFrame, qualityLevel);

        writer.write_frame();
    }

    return 0;
}