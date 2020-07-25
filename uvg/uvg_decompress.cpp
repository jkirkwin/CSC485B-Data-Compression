/* uvg_decompress.cpp

   Starter code for Assignment 4 (in C++). This program
    - Reads a height/width value from the input file
    - Reads YCbCr data from the file, with the Y plane
      in full w x h resolution and the other two planes
      in half resolution.
    - Upscales the Cb and Cr planes to full resolution and
      transforms them to RGB.
    - Writes the result as a BMP image
     (Using bitmap_image.hpp, originally from 
      http://partow.net/programming/bitmap/index.html)

   B. Bird - 07/01/2020

   J. Kirkwin - 07/24/2020
*/

#include <iostream>
#include <string>
#include <cassert>
#include "input_stream.hpp"
#include "bitmap_image.hpp"
#include "uvg_common.hpp"
#include "dct/dct.h"
#include "matrix.h"

dct::encoded_block_t readEncodedBlock(InputBitStream& inputBitStream) {
    dct::encoded_block_t block;
    for (int i = 0; i < dct::BLOCK_CAPACITY; ++i) {
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

unsigned int blocksToRead(unsigned int height, unsigned int width) {
    //https://stackoverflow.com/questions/2745074/fast-ceiling-of-an-integer-division-in-c-c
    assert (height > 0 && width > 0);
    auto verticalPartitions = width / dct::BLOCK_DIMENSION + (width % dct::BLOCK_DIMENSION != 0);
    auto horizontalPartitions = height / dct::BLOCK_DIMENSION + (height % dct::BLOCK_DIMENSION != 0);
    auto result = verticalPartitions * horizontalPartitions;
    return result;
}

dct::QualityLevel getQualityLevel(InputBitStream& inputBitStream) {
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

void decompress(const std::string& input_filename, const std::string& output_filename) {
    std::cout << "Decompressing " << input_filename << " to " << output_filename << std::endl;
    std::ifstream input_file{input_filename,std::ios::binary};
    InputBitStream input_stream {input_file};

    unsigned int height = input_stream.read_u32();
    unsigned int width = input_stream.read_u32();
    dct::QualityLevel qualityLevel = getQualityLevel(input_stream);

    unsigned int scaledHeight = (height+1)/2;
    unsigned int scaledWidth = (width+1)/2;

    // get the encoded blocks
    // todo add delta decompression here.
    auto yPlaneBlocks = blocksToRead(height, width);
    auto encodedYPlane = readEncodedBlocks(yPlaneBlocks, input_stream);

    auto colourPlaneBlocks = blocksToRead(scaledHeight, scaledWidth);
    auto encodedCbPlane = readEncodedBlocks(colourPlaneBlocks, input_stream);
    auto encodedCrPlane = readEncodedBlocks(colourPlaneBlocks, input_stream);

    // Invert the DCT to get the decoded Y and colour planes.
    auto yMatrix = dct::invert(encodedYPlane, dct::luminanceContext(height, width), qualityLevel);
    auto scaledCbMatrix = dct::invert(encodedCbPlane, dct::chromananceContext(scaledHeight, scaledWidth), qualityLevel);
    auto scaledCrMatrix = dct::invert(encodedCrPlane, dct::chromananceContext(scaledHeight, scaledWidth), qualityLevel);

    input_stream.flush_to_byte();
    input_file.close();

    // Re-construct the YCbCr image.
    // todo this could be combined with the loop below.
    auto imageYCbCr = create_2d_vector<PixelYCbCr>(height,width);
    for (unsigned int y = 0; y < height; y++){
        for (unsigned int x = 0; x < width; x++){
            imageYCbCr.at(y).at(x) = {
                    yMatrix.at(y, x),
                    scaledCbMatrix.at(y/2, x/2),
                    scaledCrMatrix.at(y/2, x/2)
            };
        }
    }

    bitmap_image output_image {width,height};

    for (unsigned int y = 0; y < height; y++){
        for (unsigned int x = 0; x < width; x++){
            auto pixel_rgb = imageYCbCr.at(y).at(x).to_rgb();
            auto [r,g,b] = pixel_rgb;
            output_image.set_pixel(x,y,r,g,b);
        }
    }

    output_image.save_image(output_filename);
}

int main(int argc, char** argv) {
    if (argc < 3){
        std::cerr << "Usage: " << argv[0] << " <input file> <output BMP>" << std::endl;
        return 1;
    }
    std::string input_filename {argv[1]};
    std::string output_filename {argv[2]};
    decompress(input_filename, output_filename);

    return 0;
}