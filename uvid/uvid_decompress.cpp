/* uvid_decompress.cpp
   CSC 485B/578B/SENG 480B - Data Compression - Summer 2020

   This placeholder code reads the data produced by uvid_compress
   and outputs it in the uncompressed YCbCr (YUV) format used for the
   sample video input files. To play the the decompressed data stream
   directly, you can pipe the output of this program to the ffplay program,
   with a command like

     ffplay -f rawvideo -pixel_format yuv420p -framerate 30 -video_size 352x288 - 2>/dev/null
   (where the resolution is explicitly given as an argument to ffplay).

   B. Bird - 07/15/2020
   J. Kirkwin - 08/07/2020
*/

#include <iostream>
#include <cassert>
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

decode::MacroblockHeader readMacroblockHeader(InputBitStream& inputBitStream) {
    bool predicted = inputBitStream.read_bit() == 1;
    if (predicted) {
        // Read the motion vector components' variable bit encodings
        int motionVecX = (int) delta::decodeFromBitStream(inputBitStream);
        int motionVecY = (int) delta::decodeFromBitStream(inputBitStream);
        return {true, motionVecX, motionVecY};
    }
    else {
        return {false, 0, 0};
    }
}

std::vector<decode::MacroblockHeader> readMacroblockHeaders(InputBitStream& inputBitStream, u32 imageHeight, u32 imageWidth) {
    // There is one macroblock for each block in the CB/CR planes.
    u32 macroblockCount = numBlocksToRead(imageHeight/2, imageWidth/2);
    std::vector<decode::MacroblockHeader> headers(macroblockCount);
    for (u32 i = 0; i < macroblockCount; ++i) {
        headers.at(i) = readMacroblockHeader(inputBitStream);
    }
    return headers;
}

std::vector<dct::encoded_block_t> readPlane(InputBitStream& inputBitStream, u32 height, u32 width) {
    auto blockCount = numBlocksToRead(height, width);
    return readEncodedBlocks(blockCount, inputBitStream);
}

decode::CompressedIFrame readIFrame(InputBitStream& inputBitStream, u32 height, u32 width) {
    // Read in the quantized coefficients for each plane
    auto yBlocks = readPlane(inputBitStream, height, width);
    auto cbBlocks = readPlane(inputBitStream, height/2, width/2);
    auto crBlocks = readPlane(inputBitStream, height/2, width/2);

    // Package up and return the result
    return decode::CompressedIFrame(height, width, yBlocks, cbBlocks, crBlocks);
}

decode::CompressedPFrame readPFrame(InputBitStream& inputBitStream, u32 height, u32 width) {
    // Read the macroblock headers.
    auto macroblockHeaders = readMacroblockHeaders(inputBitStream, height, width);

    // Read in the quantized coefficients for each plane
    auto yBlocks = readPlane(inputBitStream, height, width);
    auto cbBlocks = readPlane(inputBitStream, height/2, width/2);
    auto crBlocks = readPlane(inputBitStream, height/2, width/2);

    // Package up and return the result
    return decode::CompressedPFrame(height, width, yBlocks, cbBlocks, crBlocks, macroblockHeaders);
}

void decompress(InputBitStream& inputBitStream) {
    // Read header fields
    u32 height {inputBitStream.read_u32()};
    u32 width {inputBitStream.read_u32()};
    auto qualityLevel = getQualityLevel(inputBitStream);

    YUVStreamWriter writer {std::cout, width, height};

    // Read the first frame as an I-Frame and output it.
    assert(inputBitStream.read_bit() == 1); // There must be at least one frame of video.
    auto firstFrame = readIFrame(inputBitStream, height, width);
    decode::IFrameToYCbCr(firstFrame, writer.frame(), qualityLevel);
    writer.write_frame();

    // The writer's active frame serves as both the previous frame and as a container
    // to store the next frame. We need to copy it to prevent issues of overwriting
    // parts of the frame that are later used in a prediction via a motion vector.
    auto previous = writer.frame();

    // Read each subsequent P-Frame, decode it, and write it out
    while (inputBitStream.read_bit()){ // Check the single-bit continuation flag
        auto& activeFrame = writer.frame();
        auto pFrame = readPFrame(inputBitStream, height, width);
        decode::PFrameToYCbCr(pFrame, previous, activeFrame, qualityLevel);

        previous = writer.frame(); // Copy the decoded frame
        writer.write_frame();
    }
}

int main() {
    InputBitStream inStream(std::cin);
    decompress(inStream);
    return 0;
}