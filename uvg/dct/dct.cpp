#include <array>
#include <cassert>
#include <cmath>
#include <iostream>
#include "dct.h"
#include <algorithm>

namespace dct {
    typedef Eigen::Matrix<float, BLOCK_DIMENSION, BLOCK_DIMENSION> matrix_f_t;

    // Should only be accessed via getCMatrix()
    // todo this lazy eval is pretty ugly. Consider hiding it.
    matrix_f_t _cMatrix;
    bool _cMatrixInit = false;

    /**
     * The 'C' matrix is the one given in the slides which can be used to
     * construct and invert the DCT.
     */
    matrix_f_t getCMatrix() {
        if (!_cMatrixInit) {
            const double n = BLOCK_DIMENSION;
            const auto rootOneOverN = 1.0 / sqrt(n);
            const auto rootTwoOverN = sqrt(2.0 / n);

            for (int i = 0; i < BLOCK_DIMENSION; ++i) {
                for (int j = 0; j < BLOCK_DIMENSION; ++j) {
                    if (i == 0) {
                        _cMatrix(i, j) = rootOneOverN;
                    }
                    else {
                        const auto multiplier = (2*j + 1)*i / (2*n);
                        _cMatrix(i, j) = rootTwoOverN * cos(multiplier * M_PI);
                    }
                }
            }
            _cMatrixInit = true;
        }

        return _cMatrix;
    }

    encoded_block_t encodeBlock(
            const raw_block_t& rawBlock,
            const matrix_f_t& cMatrix,
            const quantize::quantizer_t& quantizer) {

        // convert the block to hold floats so that we can use Eigen's
        // multiplication operation to compute the DCT
        const auto fBlock = rawBlock.cast<float>();
        const auto dctResult = cMatrix * fBlock * cMatrix.transpose();

        // quantize the block
        raw_block_t quantized;
        for (int row = 0; row < BLOCK_DIMENSION; ++row) {
            for (int col = 0; col < BLOCK_DIMENSION; ++col) {
                auto dctElem = dctResult(row, col);
                auto quantElem = (float) quantizer(row, col);
                quantized(row, col) = std::round(dctElem / quantElem);
            }
        }

        // todo linearize the block
        // for now (to test) just emit the blocks in row-major order.
        encoded_block_t result;
        result.reserve(quantized.size());
        for (int i = 0; i < quantized.rows(); ++i) {
            for (int j = 0; j < quantized.cols(); ++j) {
                result.push_back(quantized(i, j));
            }
        }
        return result;
    }

    std::vector<encoded_block_t> transform(const Eigen::MatrixX<unsigned char>& inputMatrix, const quantize::quantizer_t& quantizer) {
        assert (inputMatrix.rows() > 0 && inputMatrix.cols() > 0);

        const auto cMatrix = getCMatrix();

        std::vector<encoded_block_t> result;
        result.reserve(inputMatrix.size() / (BLOCK_DIMENSION*BLOCK_DIMENSION));

        for (int row = 0; row < inputMatrix.rows(); row += BLOCK_DIMENSION) {
            for (int col = 0; col < inputMatrix.cols(); col += BLOCK_DIMENSION) {
                const auto numRows = std::min(inputMatrix.rows() - row, (long) BLOCK_DIMENSION);
                const auto numCols = std::min(inputMatrix.cols() - col, (long) BLOCK_DIMENSION);

                // ensure the block is big enough. If we have a decreased number of rows or columns
                // then the missing entries are filled with zeros.
                Eigen::Matrix<unsigned char, BLOCK_DIMENSION, BLOCK_DIMENSION> rawBlock;
                rawBlock.setZero();

                // This copies into only the numRows*numCols section of the raw block. The rest remains 0.
                rawBlock.topLeftCorner(numRows, numCols) = inputMatrix.block(row, col, numRows, numCols);

                auto encodedBlock = encodeBlock(rawBlock, cMatrix, quantizer);
                result.push_back(encodedBlock);
            }
        }
        return result;
    }

    // Scale up the given dimension to be a multiple of the block size.
    unsigned int getExpandedDimension(int dim) {
        auto difference = dim % BLOCK_DIMENSION;
        return dim + difference;
    }

    Eigen::MatrixX<unsigned char> invert(const std::vector<std::vector<int>>& blocks, const context& ctx) {
        // Create a matrix that is bigger than or the same size as the original input to easily insert blocks.
        auto expandedHeight = getExpandedDimension(ctx.height);
        auto expandedWidth = getExpandedDimension(ctx.width);
        Eigen::MatrixX<unsigned char> expandedMatrix(expandedHeight, expandedWidth);

        // Populate the expanded matrix
        for (int row = 0; row < expandedMatrix.rows(); row += BLOCK_DIMENSION) {
            for (int col = 0; col < expandedMatrix.cols(); col += BLOCK_DIMENSION) {
                auto flattenedIndex = row * BLOCK_DIMENSION + col;
                auto block = blocks.at(flattenedIndex);
                auto decodedBlock = decodeBlock(block, ctx.quantizer);

                // insert the block into the matrix
                expandedMatrix.block(row, col, BLOCK_DIMENSION, BLOCK_DIMENSION) = decodedBlock;
            }
        }

        // Take a slice of the generated image the size of the original
        return expandedMatrix.topLeftCorner(ctx.height, ctx.width);
    }

    raw_block_t decodeBlock(const std::vector<int>& block, const quantize::quantizer_t& quantizer) {
        assert (block.size() == BLOCK_DIMENSION*BLOCK_DIMENSION); // todo should really just use std::array<64>

        // construct the de-quantized matrix
        Eigen::Matrix<float, BLOCK_DIMENSION, BLOCK_DIMENSION> dctResult;
        for (int row = 0; row < BLOCK_DIMENSION; ++row) {
            for (int col = 0; col < BLOCK_DIMENSION; ++col) {
                auto flattenedIndex = row * BLOCK_DIMENSION + col;
                auto quantized = block.at(flattenedIndex);
                auto deQuantized = quantized * quantizer(row, col);
                dctResult(row, col) = deQuantized;
            }
        }

        // invert the DCT (CT*D*C)
        auto cMatrix = getCMatrix();
        Eigen::Matrix<float, BLOCK_DIMENSION, BLOCK_DIMENSION> dctDecoded = cMatrix.transpose() * dctResult * cMatrix;

        // convert the decoded value back to bytes. some data may be lost since
        // we're truncating rather than rounding here.
        return dctDecoded.cast<unsigned char>();
    }

    // Matrices are suggested by JPEG standard
    namespace quantize {

        bool _luminanceSet = false;
        bool _chromananceSet = false;
        quantizer_t _luminance;
        quantizer_t _chromanance;

        quantizer_t luminance() {
            if (!_luminanceSet) {
                _luminance <<
                        16, 11, 10, 16, 24, 40, 51, 61,
                        12, 12, 14, 19, 26, 58, 60, 55,
                        14, 13, 16, 24, 40, 57, 69, 56,
                        14, 17, 22, 29, 51, 87, 80, 62,
                        18, 22, 37, 56, 68, 109, 103, 77,
                        24, 35, 55, 64, 81, 104, 113, 92,
                        49, 64, 78, 87, 103, 121, 120, 101,
                        72, 92, 95, 98, 112, 100, 103, 99;
                _luminanceSet = true;
            }
            return _luminance;
        }

        quantizer_t getChromananceQuanizer() {
            if (!_chromananceSet) {
                _chromanance <<
                        7, 18, 24, 47, 99, 99, 99, 99,
                        18, 21, 26, 66, 99, 99, 99, 99,
                        24, 26, 56, 99, 99, 99, 99, 99,
                        47, 66, 99, 99, 99, 99, 99, 99,
                        99, 99, 99, 99, 99, 99, 99, 99,
                        99, 99, 99, 99, 99, 99, 99, 99,
                        99, 99, 99, 99, 99, 99, 99, 99,
                        99, 99, 99, 99, 99, 99, 99, 99;
                _chromananceSet = true;
            }
            return _chromanance;
        }
    }
}
