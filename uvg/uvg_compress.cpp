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

//A simple downscaling algorithm using averaging.
std::vector<std::vector<unsigned char> > scale_down(std::vector<std::vector<unsigned char> > source_image, unsigned int source_width, unsigned int source_height, int factor){

    unsigned int scaled_height = (source_height+factor-1)/factor;
    unsigned int scaled_width = (source_width+factor-1)/factor;

    //Note that create_2d_vector automatically initializes the array to all-zero
    auto sums = create_2d_vector<unsigned int>(scaled_height,scaled_width);
    auto counts = create_2d_vector<unsigned int>(scaled_height,scaled_width);

    for(unsigned int y = 0; y < source_height; y++)
        for (unsigned int x = 0; x < source_width; x++){
            sums.at(y/factor).at(x/factor) += source_image.at(y).at(x);
            counts.at(y/factor).at(x/factor)++;
        }

    auto result = create_2d_vector<unsigned char>(scaled_height,scaled_width);
    for(unsigned int y = 0; y < scaled_height; y++)
        for (unsigned int x = 0; x < scaled_width; x++)
            result.at(y).at(x) = (unsigned char)((sums.at(y).at(x)+0.5)/counts.at(y).at(x));
    return result;
}

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



/**
 * Represents a Y-CB-CR image as a set of the three independent planes.
 */
 /*
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

void compressOld(const std::string& input_filename, const std::string& output_filename) {
    std::cout << "Compressing OLD" << input_filename << " to " << output_filename << std::endl;

    bitmap_image input_image {input_filename};

    std::ofstream output_file{output_filename, std::ios::binary};
    OutputBitStream output_stream {output_file};

    // todo improve the bitstream's efficiency for encoding width and height. Variable byte encoding?

    unsigned int height = input_image.height();
    unsigned int width = input_image.width();
    output_stream.push_u32(height);
    output_stream.push_u32(width);

    auto yCbCrImage = getYCbCrImage(input_image);

    // todo Scale the two colour planes
//    auto scaledCb = scale_down(yCbCrImage.cbPlane, 2);
//    auto scaledCr = scale_down(yCbCrImage.crPlane, 2);

    // DCT each plane
    // todo use appropriate quantizers
    auto dctYBlocks = dct::transform(yCbCrImage.yPlane, dct::quantize::none());
    auto dctCbBlocks = dct::transform(yCbCrImage.cbPlane, dct::quantize::none());
//    auto dctCbBlocks = dct::transform(scaledCb, dct::quantize::chromanance());
    auto dctCrBlocks = dct::transform(yCbCrImage.crPlane, dct::quantize::none());
//    auto dctCrBlocks = dct::transform(scaledCr, dct::quantize::chromanance());

    // todo run delta compression on each plane
    // todo send results to output file.

    // TEMP: write the blocks to verify
    for (const auto& block: dctYBlocks) {
        for (const int coef : block) {
            output_stream.push_u16(coef);
        }
    }

    for (const auto& block: dctCbBlocks) {
          for (const int coef : block) {
              output_stream.push_u16(coef);
          }
    }
    for (const auto& block: dctCrBlocks) {
        for (const int coef : block) {
            output_stream.push_u16(coef);
        }
    }


    // TEMP: Write the matrixes to verify
//    for (int row = 0; row < yCbCrImage.yPlane.rows(); ++row) {
//        for (int col = 0; col < yCbCrImage.yPlane.cols(); ++col) {
//            output_stream.push_byte(yCbCrImage.yPlane(row, col));
//        }
//    }
//    for (int row = 0; row < scaledCb.rows(); ++row) {
//        for (int col = 0; col < scaledCb.cols(); ++col) {
//            output_stream.push_byte(scaledCb(row, col));
//        }
//    }
//    for (int row = 0; row < scaledCr.rows(); ++row) {
//        for (int col = 0; col < scaledCr.cols(); ++col) {
//            output_stream.push_byte(scaledCr(row, col));
//        }
//    }

    output_stream.flush_to_byte();
    output_file.close();
}
*/

void writeLiterals(const matrix::Matrix<unsigned char>& m, OutputBitStream& outputBitStream) {
    for (auto b : m.data) {
        outputBitStream.push_byte(b);
    }
}

void writeBlockLiterals(const std::vector<dct::encoded_block_t>& blocks, OutputBitStream& outputBitStream) {
    for (const auto& block : blocks) {
        for (auto i : block) {
            outputBitStream.push_u32(i);
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

void compressNew(const std::string& input_filename, const std::string& output_filename, dct::QualityLevel qualityLevel) {
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

    // todo add delta compression here.

    // Write the blocks as-is for testing
    writeBlockLiterals(encodedYPlane, output_stream);
    writeBlockLiterals(encodedCbPlane, output_stream);
    writeBlockLiterals(encodedCrPlane, output_stream);

    // Write the literal values for testing
//    writeLiterals(yPlane, output_stream);
//    writeLiterals(scaledCbPlane, output_stream);
//    writeLiterals(scaledCrPlane, output_stream);

    output_stream.flush_to_byte();
    output_file.close();
}


//void compress(const std::string& input_filename, const std::string& output_filename) {
//    std::cout << "Compressing " << input_filename << " to " << output_filename << std::endl;
//
//    bitmap_image input_image{input_filename};
//
//    unsigned int height = input_image.height();
//    unsigned int width = input_image.width();
//
//    //Read the entire image into a 2d array of PixelRGB objects
//    //(Notice that height is the outer dimension, so the pixel at coordinates (x,y)
//    // must be accessed as imageRGB.at(y).at(x)).
//    std::vector<std::vector<PixelYCbCr>> imageYCbCr = create_2d_vector<PixelYCbCr>(height, width);
//
//
//    for (unsigned int y = 0; y < height; y++) {
//        for (unsigned int x = 0; x < width; x++) {
//            auto[r, g, b] = input_image.get_pixel(x, y);
//            PixelRGB rgb_pixel{r, g, b};
//            imageYCbCr.at(y).at(x) = rgb_pixel.to_ycbcr();
//        }
//    }
//
//    std::ofstream output_file{output_filename, std::ios::binary};
//    OutputBitStream output_stream{output_file};
//
//    //Placeholder: Use a simple bitstream containing the height/width (in 32 bits each)
//    //followed by the entire set of values in each colour plane (in row major order).
//
//    output_stream.push_u32(height);
//    output_stream.push_u32(width);
//
//    //Write the Y values
//    for (unsigned int y = 0; y < height; y++)
//        for (unsigned int x = 0; x < width; x++)
//            output_stream.push_byte(imageYCbCr.at(y).at(x).Y);
//
//    //Extract the Cb plane into its own array
//    auto Cb = create_2d_vector<unsigned char>(height, width);
//    for (unsigned int y = 0; y < height; y++)
//        for (unsigned int x = 0; x < width; x++)
//            Cb.at(y).at(x) = imageYCbCr.at(y).at(x).Cb;
//    auto Cb_scaled = scale_down(Cb, width, height, 2);
//
//    //Extract the Cr plane into its own array
//    auto Cr = create_2d_vector<unsigned char>(height, width);
//    for (unsigned int y = 0; y < height; y++)
//        for (unsigned int x = 0; x < width; x++)
//            Cr.at(y).at(x) = imageYCbCr.at(y).at(x).Cr;
//    auto Cr_scaled = scale_down(Cr, width, height, 2);
//
//    //Write the Cb values
//    for (unsigned int y = 0; y < (height + 1) / 2; y++)
//        for (unsigned int x = 0; x < (width + 1) / 2; x++)
//            output_stream.push_byte(Cb_scaled.at(y).at(x));
//
//    //Write the Cr values
//    for (unsigned int y = 0; y < (height + 1) / 2; y++)
//        for (unsigned int x = 0; x < (width + 1) / 2; x++)
//            output_stream.push_byte(Cr_scaled.at(y).at(x));
//
//
//    output_stream.flush_to_byte();
//    output_file.close();
//}




int main(int argc, char ** argv) {
//    /*
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
    compressNew(input_filename, output_filename, qualityLevel);

//     */
//    std::string infile = "/home/jamie/csc485/CSC485B-Data-Compression/uvg/test_images/grapefruit.bmp";
//    std::string outfile = "/home/jamie/csc485/CSC485B-Data-Compression/uvg/temp.bin";
//    compress(infile, outfile);

    return 0;
}