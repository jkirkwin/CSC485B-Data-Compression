/* uvid_compress.cpp
   CSC 485B/578B/SENG 480B - Data Compression - Summer 2020

   todo update this docstring
   Starter code for Assignment 5

   Reads video data from stdin in uncompresed YCbCr (YUV) format 
   (With 4:2:0 subsampling). To produce this format from 
   arbitrary video data in a popular format, use the ffmpeg
   tool and a command like 

     ffmpeg -i videofile.mp4 -f rawvideo -pixel_format yuv420p - 2>/dev/null | ./this_program <width> height>

   Note that since the width/height of each frame is not encoded into the raw
   video stream, those values must be provided to the program as arguments.

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
#include "output_stream.hpp"
#include "yuv_stream.hpp"
#include "dct/dct.h"
#include "delta/delta.h"
#include <stdexcept>
#include "uvid_decode.h"

dct::QualityLevel getQualityLevel(const std::string& qualityString) {
    if (qualityString == "low") {
        return dct::low;
    }
    else if (qualityString == "medium") {
        return dct::med;
    }
    else if (qualityString == "high") {
        return dct::high;
    }
    else {
        std::cerr << "Bad quality level provided: '" << qualityString << "'. Acceptable values are 'low', 'medium', and 'high'" << std::endl;
        throw std::invalid_argument("Bad quality level");
    }
}

void sendQualityLevel(dct::QualityLevel qualityLevel, OutputBitStream& outputBitStream) {
    auto numBits = 2;
    if (qualityLevel == dct::low) {
        outputBitStream.push_bits(0, numBits);
    }
    else if(qualityLevel == dct::med) {
        outputBitStream.push_bits(1, numBits);
    }
    else {
        assert (qualityLevel == dct::high);
        outputBitStream.push_bits(2, numBits);
    }
}

void writeBlocks(const std::vector<dct::encoded_block_t>& blocks, OutputBitStream& outputBitStream) {
    for (const auto& block : blocks) {
        assert (block.size() == dct::BLOCK_CAPACITY);

        // Push the DC coefficient and the first AC coefficient as literals.
        auto dc = block.front();
        outputBitStream.push_u32(dc);
        auto ac0 = block.at(1);
        outputBitStream.push_u32(ac0);

        // Push the remaining AC coefficients as deltas
        auto prev = ac0;
        for (int i = 2; i < block.size(); ++i) {
            int cur = block.at(i);
            int diff = cur - prev;
            auto encodedDiff = delta::encode(diff);
            outputBitStream.push_bits_msb_first(encodedDiff);

            prev = cur;
        }
    }
}

decode::CompressedIFrame getIFrame(YUVFrame420& inputFrame, dct::QualityLevel qualityLevel) {
    auto yPlane = inputFrame.getYPlane();
    auto cbPlane = inputFrame.getCbPlane();
    auto crPlane = inputFrame.getCrPlane();

    auto encodedYPlane = dct::transform(yPlane, dct::quantize::luminance(), qualityLevel);
    auto encodedCbPlane = dct::transform(cbPlane, dct::quantize::chromanance(), qualityLevel);
    auto encodedCrPlane = dct::transform(crPlane, dct::quantize::chromanance(), qualityLevel);

    auto height = yPlane.rows;
    auto width = yPlane.cols;
    return decode::CompressedIFrame(height, width, encodedYPlane, encodedCbPlane, encodedCrPlane);
}

void pushIFrame(OutputBitStream& outputBitStream, decode::CompressedIFrame& iFrame) {
    writeBlocks(iFrame.y, outputBitStream);
    writeBlocks(iFrame.cb, outputBitStream);
    writeBlocks(iFrame.cr, outputBitStream);
}

void compress(u32 width, u32 height, dct::QualityLevel qualityLevel){
    // Initialize input and output streams
    YUVStreamReader reader {std::cin, width, height};
    OutputBitStream output_stream {std::cout};

    // Push metadata
    output_stream.push_u32(height);
    output_stream.push_u32(width);
    sendQualityLevel(qualityLevel, output_stream);

    // Read each frame of video, encode it, and push the encoding.
    while (reader.read_next_frame()){
        // Push a one byte flag to indicate there is another frame of video
        // todo consider reducing the size of the continuation flag.
        //  We could probably just use a single bit
        output_stream.push_byte(1);

        // Read in the next frame, encode it, and push the encoding
        YUVFrame420& frame = reader.frame();
        auto iFrame = getIFrame(frame, qualityLevel);
        pushIFrame(output_stream, iFrame);
    }

    output_stream.push_byte(0); //Flag to indicate end of data
    output_stream.flush_to_byte();
}

int main(int argc, char** argv) {
    if (argc < 4){
        std::cerr << "Usage: " << argv[0] << " <width> <height> <low/medium/high>" << std::endl;
        return 1;
    }

    u32 width = std::stoi(argv[1]);
    u32 height = std::stoi(argv[2]);
    dct::QualityLevel qualityLevel = getQualityLevel(argv[3]);
    compress(width, height, qualityLevel);
}