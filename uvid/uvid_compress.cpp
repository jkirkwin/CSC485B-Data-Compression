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

void writeBlock(const dct::encoded_block_t& block, OutputBitStream& outputBitStream) {
    // todo use delta encoding here
    for (const u32 coefficient : block) {
        outputBitStream.push_u32(coefficient);
    }
}

void writeBlocks(const std::vector<dct::encoded_block_t>& blocks, OutputBitStream& outputBitStream) {
    for (const auto& block : blocks) {
        writeBlock(block, outputBitStream);
    }
}

int main(int argc, char** argv){

    if (argc < 4){
        std::cerr << "Usage: " << argv[0] << " <width> <height> <low/medium/high>" << std::endl;
        return 1;
    }
    u32 width = std::stoi(argv[1]);
    u32 height = std::stoi(argv[2]);
    std::string quality{argv[3]};

    u32 scaledWidth = width/2;
    u32 scaledHeight = height/2;

    YUVStreamReader reader {std::cin, width, height};
    OutputBitStream output_stream {std::cout};

    output_stream.push_u32(height);
    output_stream.push_u32(width);

    while (reader.read_next_frame()){
        // todo consider reducing the size of the continuation flag. We could probably just use a single bit?
        output_stream.push_byte(1); //Use a one byte flag to indicate whether there is another frame of video
        YUVFrame420& frame = reader.frame();

        matrix::Matrix<unsigned char> yPlane(height, width);
        matrix::Matrix<unsigned char> cbPlane(scaledHeight, scaledWidth), crPlane(scaledHeight, scaledWidth);

        for (u32 y = 0; y < height; y++) {
            for (u32 x = 0; x < width; x++) {
                yPlane.set(y,x) = frame.Y(x,y);
            }
        }
        for (u32 y = 0; y < scaledHeight; y++) {
            for (u32 x = 0; x < scaledWidth; x++) {
                cbPlane.set(y,x) = frame.Cb(x,y);
            }
        }
        for (u32 y = 0; y < scaledHeight; y++) {
            for (u32 x = 0; x < scaledWidth; x++) {
                crPlane.set(y,x) = frame.Cr(x,y);
            }
        }
        // todo consider changing the frame object to use a matrix internally

        // Run the DCT on each plane in the frame and quantize the coefficients
        const auto qualityLevel = dct::med; // todo parameterize the quality parameter based on argument and add it to the bitstream
        auto encodedYPlane = dct::transform(yPlane, dct::quantize::luminance(), qualityLevel);
        auto encodedCbPlane = dct::transform(cbPlane, dct::quantize::chromanance(), qualityLevel);
        auto encodedCrPlane = dct::transform(crPlane, dct::quantize::chromanance(), qualityLevel);

        // Output the encoded blocks
        writeBlocks(encodedYPlane, output_stream);
        writeBlocks(encodedCbPlane, output_stream);
        writeBlocks(encodedCrPlane, output_stream);
    }

    output_stream.push_byte(0); //Flag to indicate end of data
    output_stream.flush_to_byte();

    return 0;
}