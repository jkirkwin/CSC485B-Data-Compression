/* uvg_compress.cpp

   Starter code for Assignment 4 (in C++). This program
    - Reads an input image in BMP format
     (Using bitmap_image.hpp, originally from 
      http://partow.net/programming/bitmap/index.html)
    - Transforms the image from RGB to YCbCr (i.e. "YUV").
    - Downscales the Cb and Cr planes by a factor of two
      (producing the same resolution that would result
       from 4:2:0 subsampling, but using interpolation
       instead of ignoring some samples)
    - Writes each colour plane (Y, then Cb, then Cr)
      in 8 bits per sample to the output file.

   B. Bird - 07/01/2020

   Modified by J. Kirkwin - 07/21/2020
*/

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include "output_stream.hpp"
#include "bitmap_image.hpp"
#include "uvg_common.hpp"
#include "dct/dct.h"
#include "matrix.h"
#include "delta/delta.h"


// An adapted version of the given function that uses matrix::Matrix.
matrix::Matrix<unsigned char> scaleDown(const matrix::Matrix<unsigned char>& inputMatrix, int factor){
    assert (factor != 0);
    assert (inputMatrix.rows > 0 && inputMatrix.cols > 0);

    unsigned int scaled_height = (inputMatrix.rows + factor-1)/factor;
    unsigned int scaled_width = (inputMatrix.cols + factor-1)/factor;

    //Note that create_2d_vector automatically initializes the array to all-zero
    auto sums = create_2d_vector<unsigned int>(scaled_height,scaled_width);
    auto counts = create_2d_vector<unsigned int>(scaled_height,scaled_width);

    for(unsigned int y = 0; y < inputMatrix.rows; y++)
        for (unsigned int x = 0; x < inputMatrix.cols; x++){
            sums.at(y/factor).at(x/factor) += inputMatrix.at(y, x);
            counts.at(y/factor).at(x/factor)++;
        }

    matrix::Matrix<unsigned char> result(scaled_height, scaled_width);
    for(unsigned int y = 0; y < scaled_height; y++) {
        for (unsigned int x = 0; x < scaled_width; x++) {
            auto val = (unsigned char) ((sums.at(y).at(x) + 0.5) / counts.at(y).at(x));
            result.set(y, x, val);
        }
    }
    return result;
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

void compress(const std::string& input_filename, const std::string& output_filename, dct::QualityLevel qualityLevel) {
    std::cout << "Compressing " << input_filename << " to " << output_filename << std::endl;

    bitmap_image input_image{input_filename};

    std::ofstream output_file{output_filename, std::ios::binary};
    OutputBitStream output_stream{output_file};

    unsigned int height = input_image.height();
    unsigned int width = input_image.width();
    output_stream.push_u32(height); // todo use a variable byte encoding here.
    output_stream.push_u32(width);

    sendQualityLevel(qualityLevel, output_stream);

    // Convert the RBG image to Y, Cb, Cr planes
    matrix::Matrix<unsigned char> yPlane(height, width), cbPlane(height, width), crPlane(height, width);
    for (unsigned int y = 0; y < height; y++) {
        for (unsigned int x = 0; x < width; x++) {
            auto[r, g, b] = input_image.get_pixel(x, y);
            PixelRGB rgb_pixel{r, g, b};
            auto yCbCrPixel = rgb_pixel.to_ycbcr();
            yPlane.set(y, x, yCbCrPixel.Y);
            cbPlane.set(y, x, yCbCrPixel.Cb);
            crPlane.set(y, x, yCbCrPixel.Cr);
        }
    }

    // Scale down (sub-sample) the colour planes.
    auto scaledCbPlane = scaleDown(cbPlane, 2);
    auto scaledCrPlane = scaleDown(crPlane, 2);

    // Run DCT on each plane.
    auto encodedYPlane = dct::transform(yPlane, dct::quantize::luminance(), qualityLevel);
    auto encodedCbPlane = dct::transform(scaledCbPlane, dct::quantize::chromanance(), qualityLevel);
    auto encodedCrPlane = dct::transform(scaledCrPlane, dct::quantize::chromanance(), qualityLevel);

    // Do delta compression and send the result.
    writeBlocks(encodedYPlane, output_stream);
    writeBlocks(encodedCbPlane, output_stream);
    writeBlocks(encodedCrPlane, output_stream);

    output_stream.flush_to_byte();
    output_file.close();
}

int main(int argc, char ** argv) {
    if (argc < 4){
        std::cerr << "Usage: " << argv[0] << " <low/medium/high> <input BMP> <output file>" << std::endl;
        return 1;
    }

    dct::QualityLevel qualityLevel;
    std::string quality{argv[1]};
    if (quality == "low") {
        qualityLevel = dct::low;
    }
    else if (quality == "medium") {
        qualityLevel = dct::med;
    }
    else if (quality == "high") {
        qualityLevel = dct::high;
    }
    else {
        std::cerr << "Usage: " << argv[0] << " <low/medium/high> <input BMP> <output file>" << std::endl;
        return 1;
    }

    std::string input_filename {argv[2]};
    std::string output_filename {argv[3]};
    compress(input_filename, output_filename, qualityLevel);

    return 0;
}