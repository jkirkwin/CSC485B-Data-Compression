#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include "dct.h"
#include <algorithm>

namespace dct {

    // Computed with gen_dct_matrix.cpp
    const std::vector<float> dctMatrixData {
        0.353553, 0.353553, 0.353553, 0.353553, 0.353553, 0.353553, 0.353553, 0.353553,
        0.490393, 0.415735, 0.277785, 0.0975452, -0.0975452, -0.277785, -0.415735, -0.490393,
        0.46194, 0.191342, -0.191342, -0.46194, -0.46194, -0.191342, 0.191342, 0.46194,
        0.415735, -0.0975452, -0.490393, -0.277785, 0.277785, 0.490393, 0.0975452, -0.415735,
        0.353553, -0.353553, -0.353553, 0.353553, 0.353553, -0.353553, -0.353553, 0.353553,
        0.277785, -0.490393, 0.0975452, 0.415735, -0.415735, -0.0975452, 0.490393, -0.277785,
        0.191342, -0.46194, 0.46194, -0.191342, -0.191342, 0.46194, -0.46194, 0.191342,
        0.0975452, -0.277785, 0.415735, -0.490393, 0.490393, -0.415735, 0.277785, -0.0975452
    };
    const matrix::Matrix<float> cMatrix (8, 8, dctMatrixData);
    const matrix::Matrix<float> cMatrixTranspose = matrix::transpose(cMatrix);

    /*
     * Convert a normal block to one that uses a float representation.
     */
    matrix::Matrix<float> getFloatBlock(const raw_block_t& block) {
        // https://stackoverflow.com/questions/6399090/c-convert-vectorint-to-vectordouble
        std::vector<float> floatData(block.data.begin(), block.data.end());
        return matrix::Matrix<float> (block.rows, block.cols, floatData);
    }

    /*
     * Convert a float block to one that uses a byte representation.
     */
    raw_block_t getRawBlockFromFloat(const matrix::Matrix<float>& block) {
        // https://stackoverflow.com/questions/6399090/c-convert-vectorint-to-vectordouble

        std::vector<unsigned char> byteData;
        for (auto f : block.data) {
            // ensure that we don't wrap around on out-of-range values
            if ( f <= 0) {
                byteData.push_back(0);
            }
            else if (f > 255) {
                byteData.push_back(255);
            }
            else {
                byteData.push_back(std::round(f));
            }
        }

        return raw_block_t(block.rows, block.cols, byteData);
    }


    encoded_block_t encodeBlock(const raw_block_t& rawBlock, const quantize::quantizer_t& quantizer) {
        const auto rawFloatBlock = getFloatBlock(rawBlock);
        const auto intermediate = matrix::multiply(cMatrix, rawFloatBlock);
        const auto dctResult = matrix::multiply(intermediate, cMatrixTranspose);

        // quantize the block
        encoded_block_t quantized;
        quantized.reserve(BLOCK_CAPACITY);
        for (int row = 0; row < BLOCK_DIMENSION; ++row) {
            for (int col = 0; col < BLOCK_DIMENSION; ++col) {
                auto dctElem = dctResult.at(row, col);
                auto quantElem = (float) quantizer.at(row, col);
                quantized.push_back(std::round(dctElem / quantElem));
            }
        }
        assert(quantized.size() == BLOCK_CAPACITY);

        // todo linearize the block in a zigzag pattern
        // for now just emit the blocks in row-major order.
        return quantized;
    }

    std::vector<encoded_block_t> transform(
            const matrix::Matrix<unsigned char>& inputMatrix,
            const quantize::quantizer_t& quantizer) {
        assert (inputMatrix.rows > 0 && inputMatrix.cols > 0);

        std::vector<encoded_block_t> result;
        result.reserve(inputMatrix.capacity() / (BLOCK_CAPACITY-1)); // This should hopefully prevent resizing

        for (int row = 0; row < inputMatrix.rows; row += BLOCK_DIMENSION) {
            for (int col = 0; col < inputMatrix.cols; col += BLOCK_DIMENSION) {
                // pull out an 8x8 block.
                const auto numRows = std::min(inputMatrix.rows - row,  BLOCK_DIMENSION);
                const auto numCols = std::min(inputMatrix.cols - col,  BLOCK_DIMENSION);

                // ensure the block is big enough. If we have a decreased number of rows or columns
                // then the missing entries are filled with zeros.
                raw_block_t rawBlock(BLOCK_DIMENSION, BLOCK_DIMENSION, 0);

                // Copy into only the numRows*numCols section of the raw block. The rest remains 0s.
                for (int i = 0; i < numRows; ++i) {
                    for (int j = 0; j < numCols; ++j) {
                        auto inputValue = inputMatrix.at(row + i, col + j);
                        rawBlock.set(i, j, inputValue);
                    }
                }

                auto encodedBlock = encodeBlock(rawBlock, quantizer);
                result.push_back(encodedBlock);
            }
        }
        return result;
    }

    matrix::Matrix<unsigned char> invert(const std::vector<encoded_block_t>& blocks, const inversionContext& context) {
        //https://stackoverflow.com/questions/2745074/fast-ceiling-of-an-integer-division-in-c-c
        auto blocksPerRow = context.width / BLOCK_DIMENSION + (context.width % BLOCK_DIMENSION != 0);

        // create and populate a result matrix
        matrix::Matrix<unsigned char> resultMatrix(context.height, context.width);
        for (int row = 0; row < resultMatrix.rows; row += BLOCK_DIMENSION) {
            for (int col = 0; col < resultMatrix.cols; col += BLOCK_DIMENSION) {
                // (row, col) is the position of the top left coefficient in the next block of the matrix.
                //  Map this to an index in the block array.
                auto colPos = col / BLOCK_DIMENSION;
                auto rowPos = row / BLOCK_DIMENSION;
                auto flattenedIndex = colPos + rowPos * blocksPerRow;
                auto encodedBlock = blocks.at(flattenedIndex);

                auto decodedBlock = decodeBlock(encodedBlock, context.quantizer);

                // Determine if the block should be inserted in full, or if some columns
                // or rows run off the edge of the image.
                const auto numRows = std::min(resultMatrix.rows - row, BLOCK_DIMENSION);
                const auto numCols = std::min(resultMatrix.cols - col, BLOCK_DIMENSION);
                if (numRows == BLOCK_DIMENSION && numCols == BLOCK_DIMENSION) {
                    // no need to trim, so we avoid copying to a new matrix.
                    resultMatrix.insertBlock(row, col, decodedBlock);
                }
                else {
                    // Get only the section of bthe block that was in the original image.
                    auto trimmedDecodedBlock = decodedBlock.getBlock(0, 0, numRows, numCols);
                    resultMatrix.insertBlock(row, col, trimmedDecodedBlock);
                }
            }
        }

        return resultMatrix;
    }

    raw_block_t decodeBlock(const encoded_block_t& block, const quantize::quantizer_t& quantizer) {
        assert (block.size() == BLOCK_CAPACITY); // todo should really just use std::array<64>

        // construct the de-quantized DCT matrix
        matrix::Matrix<float> dctEncoded(BLOCK_DIMENSION, BLOCK_DIMENSION);
        for (int row = 0; row < BLOCK_DIMENSION; ++row) {
            for (int col = 0; col < BLOCK_DIMENSION; ++col) {
                auto flattenedIndex = row * BLOCK_DIMENSION + col;
                auto quantized = block.at(flattenedIndex); // T in the slides
                auto deQuantized = quantized * quantizer.at(row, col); // D' in the slides
                dctEncoded.set(row, col, deQuantized);
            }
        }

        // invert the DCT (CT*D*C)
        auto intermediate = matrix::multiply(cMatrixTranspose, dctEncoded);
        auto dctDecoded = matrix::multiply(intermediate, cMatrix);

        // convert back to bytes from real values
        return getRawBlockFromFloat(dctDecoded);
    }

    // Matrices are suggested by JPEG standard
    namespace quantize {
        std::vector<int> luminanceData {
                16, 11, 10, 16, 24, 40, 51, 61,
                12, 12, 14, 19, 26, 58, 60, 55,
                14, 13, 16, 24, 40, 57, 69, 56,
                14, 17, 22, 29, 51, 87, 80, 62,
                18, 22, 37, 56, 68, 109, 103, 77,
                24, 35, 55, 64, 81, 104, 113, 92,
                49, 64, 78, 87, 103, 121, 120, 101,
                72, 92, 95, 98, 112, 100, 103, 99
        };
        quantizer_t luminanceMatrix(BLOCK_DIMENSION, BLOCK_DIMENSION, luminanceData);

        std::vector<int> chromananceData {
            7, 18, 24, 47, 99, 99, 99, 99,
            18, 21, 26, 66, 99, 99, 99, 99,
            24, 26, 56, 99, 99, 99, 99, 99,
            47, 66, 99, 99, 99, 99, 99, 99,
            99, 99, 99, 99, 99, 99, 99, 99,
            99, 99, 99, 99, 99, 99, 99, 99,
            99, 99, 99, 99, 99, 99, 99, 99,
            99, 99, 99, 99, 99, 99, 99, 99
        };
        quantizer_t chromananceMatrix(BLOCK_DIMENSION, BLOCK_DIMENSION, chromananceData);

        quantizer_t luminance() {
            return luminanceMatrix;
        }

        quantizer_t chromanance() {
            return chromananceMatrix;
        }
    }
}
