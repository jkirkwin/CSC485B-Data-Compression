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
*/

#include <iostream>
#include <string>
#include <cassert>
#include "input_stream.hpp"
#include "bitmap_image.hpp"
#include "uvg_common.hpp"
#include "dct/dct.h"
#include "matrix.h"

// todo remove
void createTestImage() {
    unsigned int width = 8, height = 8;
    bitmap_image output_image {width,height};

    for (int i = 0; i < width; ++i) {
        output_image.set_pixel(i, 0, 128,128,128);
    }
    for (int i = 0; i < width; ++i) {
        output_image.set_pixel(i,1,50,50,200);
        output_image.set_pixel(i,2,50,50,200);
    }
    for (int i = 0; i < width; ++i) {
        output_image.set_pixel(i,3,50,200,100);
        output_image.set_pixel(i,4,50,200,100);
    }
    for (int i = 0; i < width; ++i) {
        output_image.set_pixel(i,5,250, 10, 10);
        output_image.set_pixel(i,6,250, 10, 10);
        output_image.set_pixel(i,7,250, 10, 10);
    }

    output_image.save_image("/home/jamie/csc485/CSC485B-Data-Compression/uvg/testinput.bmp");
}


//std::vector<dct::encoded_block_t> getBlocks(unsigned int n, InputBitStream& inStream) {
//    std::vector<dct::encoded_block_t> container;
//    container.reserve(n);
//    for (int i = 0; i < n; ++i) {
//        dct::encoded_block_t block;
//        for (int j = 0; j < dct::BLOCK_CAPACITY; ++j) {
//            block.push_back((int)inStream.read_u16());
//        }
//        container.push_back(block);
//    }
//    return container;
//}

//void decompress(const std::string& input_filename, const std::string& output_filename) {
//    std::cout << "Decompressing " << input_filename << " to " << output_filename << std::endl;
//
//    std::ifstream input_file{input_filename,std::ios::binary};
//    InputBitStream input_stream {input_file};
//
//    unsigned int height = input_stream.read_u32();
//    unsigned int width = input_stream.read_u32();
//
//    // todo This is where we will need to add delta decompression and DCT inversion.
//    //      For the DCT we'll need the quality measure from the bit stream
//
//
//    // Read in the DCT blocks.
//    // todo add scaling back in
//    auto yBlocks = getBlocks(height * width, input_stream);
////    auto scaledHeight = (height+1)/2;
////    auto scaledWidth = (width+1)/2;
////    auto scaledSize = scaledHeight * scaledWidth;
////    auto cbBlocks = getBlocks(scaledSize, input_stream);
//    auto cbBlocks = getBlocks(height * width, input_stream);
////    auto crBlocks = getBlocks(scaledSize, input_stream);
//    auto crBlocks = getBlocks(height * width, input_stream);
//
//    // Decode the DCT blocks
//    // todo use the appropriate quantizers
////    auto yCtx = dct::luminanceContext(height, width);
//    dct::inversionContext yCtx {height, width, dct::quantize::none()};
//    auto yMatrix = dct::invert(yBlocks, yCtx);
//    dct::inversionContext cCtx {height, width, dct::quantize::none()};
////    auto cCtx = dct::chromananceContext(height, width);
////    auto scaledCbMatrix = dct::invert(cbBlocks, cCtx);
//    auto cbMatrix = dct::invert(cbBlocks, cCtx);
////    auto scaledCrMatrix = dct::invert(crBlocks, cCtx);
//    auto crMatrix = dct::invert(crBlocks, cCtx);
//
//   // ----------old code------------
////    auto Y = create_2d_vector<unsigned char>(height,width);
////    auto Cb_scaled = create_2d_vector<unsigned char>((height+1)/2,(width+1)/2);
////    auto Cr_scaled = create_2d_vector<unsigned char>((height+1)/2,(width+1)/2);
//
////    for (unsigned int y = 0; y < height; y++) {
////        for (unsigned int x = 0; x < width; x++) {
////            Y.at(y).at(x) = input_stream.read_byte();
////        }
////    }
////
////    for (unsigned int y = 0; y < (height+1)/2; y++) {
////        for (unsigned int x = 0; x < (width+1)/2; x++) {
////            Cb_scaled.at(y).at(x) = input_stream.read_byte();
////        }
////    }
////
////    for (unsigned int y = 0; y < (height+1)/2; y++) {
////        for (unsigned int x = 0; x < (width+1)/2; x++) {
////            Cr_scaled.at(y).at(x) = input_stream.read_byte();
////        }
////    }
//
//    auto imageYCbCr = create_2d_vector<PixelYCbCr>(height,width);
//    for (unsigned int y = 0; y < height; y++){
//        for (unsigned int x = 0; x < width; x++){
//            imageYCbCr.at(y).at(x) = {
//
//                yMatrix(y,x),
//                cbMatrix(y,x),
//                crMatrix(y,x)
////                scaledCbMatrix(y/2,x/2),
////                scaledCrMatrix(y/2,x/2)
//
////                Y.at(y).at(x),
////                Cb_scaled.at(y/2).at(x/2),
////                Cr_scaled.at(y/2).at(x/2)
//
//
////                128, 128
//            };
//        }
//    }
//
//    input_stream.flush_to_byte();
//    input_file.close();
//
//    bitmap_image output_image {width,height};
//
//    for (unsigned int y = 0; y < height; y++){
//        for (unsigned int x = 0; x < width; x++){
//            auto pixel_rgb = imageYCbCr.at(y).at(x).to_rgb();
//            auto [r,g,b] = pixel_rgb;
//            output_image.set_pixel(x,y,r,g,b);
//        }
//    }
//
//    output_image.save_image(output_filename);
//}

void fillMatrixWithLiterals(matrix::Matrix<unsigned char>& m, InputBitStream& inputBitStream) {
    for (int i = 0; i < m.capacity(); ++i) {
        m.data.at(i) = inputBitStream.read_byte();
    }
}

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

void decompressNew(const std::string& input_filename, const std::string& output_filename) {
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
// /*
    if (argc < 3){
        std::cerr << "Usage: " << argv[0] << " <input file> <output BMP>" << std::endl;
        return 1;
    }
    std::string input_filename {argv[1]};
    std::string output_filename {argv[2]};
    decompressNew(input_filename, output_filename);
//*/

//    auto infile = "/home/jamie/csc485/CSC485B-Data-Compression/uvg/temp.bin";
//    auto outfile = "/home/jamie/csc485/CSC485B-Data-Compression/uvg/temp.bmp";
//    decompressNew(infile, outfile);

//    createTestImage();
    return 0;
}