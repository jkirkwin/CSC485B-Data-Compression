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

decode::CompressedPFrame getPFrame(YUVFrame420& inputFrame, YUVFrame420& previousFrame, dct::QualityLevel qualityLevel) {
    auto height = inputFrame.getYPlane().rows;
    auto width = inputFrame.getYPlane().cols;
    auto scaledHeight = inputFrame.getCbPlane().rows;
    auto scaledWidth = inputFrame.getCbPlane().cols;

    auto y = inputFrame.getYPlane();
    auto cb = inputFrame.getCbPlane();
    auto cr = inputFrame.getCrPlane();

    // todo add back the diff-ing once macroblocks/motion vectors are added to the bitstream.
//    matrix::Matrix<unsigned char> y(height, width);
//    matrix::Matrix<unsigned char> cb(scaledHeight, scaledWidth);
//    matrix::Matrix<unsigned char> cr(scaledHeight, scaledWidth);
//    for (u32 row = 0; row < height; ++row) {
//        for (u32 col = 0; col < width; ++col) {
//            y.set(row, col) = inputFrame.Y(col, row) - previousFrame.Y(col, row);
//            y.set(row, col) = inputFrame.Y(col, row) - previousFrame.Y(col, row);
//        }
//    }
//    for (u32 row = 0; row < scaledHeight; ++row) {
//        for (u32 col = 0; col < scaledWidth; ++col) {
//            cb.set(row, col) = inputFrame.Cb(col, row) - previousFrame.Cb(col, row);
//            cr.set(row, col) = inputFrame.Cr(col, row) - previousFrame.Cr(col, row);
//        }
//    }

    auto encodedYPlane = dct::transform(y, dct::quantize::luminance(), qualityLevel);
    auto encodedCbPlane = dct::transform(cb, dct::quantize::chromanance(), qualityLevel);
    auto encodedCrPlane = dct::transform(cr, dct::quantize::chromanance(), qualityLevel);

    // We aren't doing any diff-ing, so we generate un-predicted macroblock headers
    decode::MacroblockHeader unpredictedHeader {false, 0, 0};
    std::vector<decode::MacroblockHeader> macroblockHeaders(encodedCbPlane.size(), unpredictedHeader);

    return decode::CompressedPFrame(height, width, encodedYPlane, encodedCbPlane, encodedCrPlane, macroblockHeaders);
}

void pushMacroblock(OutputBitStream& outputBitStream, decode::MacroblockHeader header) {
    if (header.predicted) {
        // Signal that the macroblock is predicted
        outputBitStream.push_bit(1);

        // Push a variable-bit encoding for the motion vector components
        auto encodedX = delta::encode(header.motionVectorX);
        auto encodedY = delta::encode(header.motionVectorY);
        outputBitStream.push_bits_msb_first(encodedX);
        outputBitStream.push_bits_msb_first(encodedY);
    }
    else {
        // Signal the the macroblock is not predicted
        outputBitStream.push_bit(0);
    }
}

void pushPFrame(OutputBitStream& outputBitStream, decode::CompressedPFrame& pFrame) {
    // Push the header for each macroblock first, then push the planes
    for (const auto& header : pFrame.macroblockHeaders) {
        pushMacroblock(outputBitStream, header);
    }

    writeBlocks(pFrame.y, outputBitStream);
    writeBlocks(pFrame.cb, outputBitStream);
    writeBlocks(pFrame.cr, outputBitStream);
}


void compress(u32 width, u32 height, dct::QualityLevel qualityLevel, YUVStreamReader& reader, OutputBitStream& outputBitStream){
    // Push metadata
    outputBitStream.push_u32(height);
    outputBitStream.push_u32(width);
    sendQualityLevel(qualityLevel, outputBitStream);

    // Read the first frame and encode it as an I-Frame
    assert (reader.read_next_frame());
    outputBitStream.push_byte(1); // Continuation flag
    auto iFrame = getIFrame(reader.frame(), qualityLevel);
    pushIFrame(outputBitStream, iFrame);

    // Continually store the __decoded__ version of each frame in order to
    // generate P-Frames
    auto previous = decode::IFrameToYCbCr(iFrame, qualityLevel);

    // Read each frame of video, encode it, and push the encoding.
    while (reader.read_next_frame()){
        // Push a one byte flag to indicate there is another frame of video
        // todo consider reducing the size of the continuation flag.
        //  We could probably just use a single bit
        outputBitStream.push_byte(1);

        // Read in the next frame, encode it, and push the encoding
        YUVFrame420& nextFrame = reader.frame();
        auto pFrame = getPFrame(nextFrame, previous, qualityLevel);
        pushPFrame(outputBitStream, pFrame);

        // Cache the decoded version of the last frame for future P-Frames
        auto decoded = decode::PFrameToYCbCr(pFrame, previous, qualityLevel);
        previous = decoded;
    }

    outputBitStream.push_byte(0); // Flag to indicate end of data
    outputBitStream.flush_to_byte();
}

//int NOT_MAIN(int argc, char** argv) {
int main(int argc, char** argv) {
    // Read arguments
    if (argc < 4){
        std::cerr << "Usage: " << argv[0] << " <width> <height> <low/medium/high>" << std::endl;
        return 1;
    }
    u32 width = std::stoi(argv[1]);
    u32 height = std::stoi(argv[2]);
    dct::QualityLevel qualityLevel = getQualityLevel(argv[3]);

    YUVStreamReader reader {std::cin, width, height};
    OutputBitStream outputBitStream {std::cout};

    compress(width, height, qualityLevel, reader, outputBitStream);

    return 0;
}


// A dummy main() for debugging
//int main(int argc, char** argv) {
int dummy(int argc, char** argv) {
    u32 width = 352, height = 288;
    auto qualityLevel = dct::med;

    auto filepath = "/home/jamie/csc485/CSC485B-Data-Compression/uvid/raw_videos/flower_352x288.raw";
    std::fstream infile(filepath);

    YUVStreamReader reader {infile, width, height};
    OutputBitStream outputBitStream {std::cout};

    compress(width, height, qualityLevel, reader, outputBitStream);

    return 0;
}