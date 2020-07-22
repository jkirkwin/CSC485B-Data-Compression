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
#include "Eigen/Dense"
#include "dct/dct.h"

//A simple downscaling algorithm using averaging.
Eigen::MatrixX<unsigned char> scale_down(Eigen::MatrixX<unsigned char> source_image, int factor){
    const auto source_height = source_image.rows();
    assert (source_height > 0);
    const auto source_width = source_image.cols();
    assert (source_width > 0);

    unsigned int scaled_height = (source_height+factor-1)/factor;
    unsigned int scaled_width = (source_width+factor-1)/factor;

    //Note that create_2d_vector automatically initializes the array to all-zero
    auto sums = create_2d_vector<unsigned int>(scaled_height,scaled_width);
    auto counts = create_2d_vector<unsigned int>(scaled_height,scaled_width);

    for(unsigned int y = 0; y < source_height; y++) {
        for (unsigned int x = 0; x < source_width; x++){
            sums.at(y/factor).at(x/factor) += source_image(y,x);
            counts.at(y/factor).at(x/factor)++;
        }
    }

    Eigen::MatrixX<unsigned char> resultMatrix(scaled_height, scaled_width);
    for(unsigned int y = 0; y < scaled_height; y++) {
        for (unsigned int x = 0; x < scaled_width; x++) {
            resultMatrix(y, x) = (unsigned char)((sums.at(y).at(x)+0.5)/counts.at(y).at(x));
        }
    }
    return resultMatrix;
}

/**
 * Represents a Y-CB-CR image as a set of the three independent planes.
 */
struct YCbCrImage {
    typedef Eigen::MatrixX<unsigned char> matrix_t;

    unsigned int height;
    unsigned int width;
    matrix_t yPlane;
    matrix_t cbPlane;
    matrix_t crPlane;

    YCbCrImage(int h, int w, matrix_t y, matrix_t cb, matrix_t cr) :
            height(h), width(w), yPlane(y), cbPlane(cb), crPlane(cr) {
    }
};

YCbCrImage getYCbCrImage(const bitmap_image& input_image) {
    unsigned int height = input_image.height();
    unsigned int width = input_image.width();

    YCbCrImage::matrix_t yMatrix(height, width), cbMatrix(height, width), crMatrix(height, width);

    for(unsigned int y = 0; y < height; y++){
        for (unsigned int x = 0; x < width; x++) {
            auto [r,g,b] = input_image.get_pixel(x,y);
            PixelRGB rgb_pixel {r,g,b};

            auto yCbCrPixel = rgb_pixel.to_ycbcr();
            auto row = y, col = x;
            yMatrix(row, col) = yCbCrPixel.Y;
            cbMatrix(row, col) = yCbCrPixel.Cb;
            crMatrix(row, col) = yCbCrPixel.Cr;
        }
    }

    return YCbCrImage(height, width, yMatrix, cbMatrix, crMatrix);
}

void compress(const std::string& input_filename, const std::string& output_filename) {
    bitmap_image input_image {input_filename};
    auto imageYCbCr = getYCbCrImage(input_image);

    std::ofstream output_file{output_filename, std::ios::binary};
    OutputBitStream output_stream {output_file};

    //Placeholder: Use a simple bitstream containing the height/width (in 32 bits each)
    //followed by the entire set of values in each colour plane (in row major order).

    // todo improve the bitstream's efficiency for encoding width and height. Variable byte encoding?

    unsigned int height = input_image.height();
    unsigned int width = input_image.width();
    output_stream.push_u32(height);
    output_stream.push_u32(width);


    auto yCbCrImage = getYCbCrImage(input_image);

    // Scale the two colour planes
    auto scaledCb = scale_down(yCbCrImage.cbPlane, 2);
    auto scaledCr = scale_down(yCbCrImage.crPlane, 2);

    // todo DCT each plane
//    auto dctYBlocks = dct::transform(yCbCrImage.yPlane);
//    auto dctCbBlocks = dct::transform(scaledCb);
//    auto dctCrBlocks = dct::transform(scaledCr);

    // todo run delta compression on each plane
    // todo send results to output file.

    // Temp: Write the matrixes to verify
    for (int row = 0; row < yCbCrImage.yPlane.rows(); ++row) {
        for (int col = 0; col < yCbCrImage.yPlane.cols(); ++col) {
            output_stream.push_byte(yCbCrImage.yPlane(row, col));
        }
    }
    for (int row = 0; row < scaledCb.rows(); ++row) {
        for (int col = 0; col < scaledCb.cols(); ++col) {
            output_stream.push_byte(scaledCb(row, col));
        }
    }
    for (int row = 0; row < scaledCr.rows(); ++row) {
        for (int col = 0; col < scaledCr.cols(); ++col) {
            output_stream.push_byte(scaledCr(row, col));
        }
    }

    output_stream.flush_to_byte();
    output_file.close();
}

int main(int argc, char ** argv) {
    if (argc < 4){
        std::cerr << "Usage: " << argv[0] << " <low/medium/high> <input BMP> <output file>" << std::endl;
        return 1;
    }

    std::string quality{argv[1]};
    std::string input_filename {argv[2]};
    std::string output_filename {argv[3]};
    compress(input_filename, output_filename);

//    compress("/home/jamie/csc485/CSC485B-Data-Compression/uvg/test_images/Flag_of_Canada_medium.bmp", "/tmp/foo");

    return 0;
}