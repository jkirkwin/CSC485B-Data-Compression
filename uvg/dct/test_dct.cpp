#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "dct.h"
#include "matrix.h"

using namespace dct;

//template<typename T>
//std::string matrixToString(Eigen::MatrixX<T> m) {
//    std::stringstream ss;
//    ss << m;
//    return ss.str();
//}

//TEST_CASE("Check DC coefficient", "") {
//    quantize::quantizer_t quantizer;
//    quantizer.setConstant(100000);
//    quantizer(0, 0) = 1; // Keep only the DC coefficient
//
//    auto inputMatrix = raw_block_t::Ones();
//    auto encoded = transform(inputMatrix, quantizer);
//
//    REQUIRE(encoded.size() == 1);
//    auto block = encoded.at(0);
//    REQUIRE(block.size() == BLOCK_CAPACITY);
//
//    // todo compute what the DC coefficient should be
////    REQUIRE(block.at(0) == 1);
//    for (int i = 1; i < BLOCK_CAPACITY; ++i) {
//        REQUIRE(block.at(i) == 0);
//    }
//}

//TEST_CASE("No quantization yields rounded values", "") {
//    raw_block_t input;
//    input.setConstant(100);
//
//    auto dctResult = transform(input, quantize::none());
//    inversionContext ctx = {BLOCK_DIMENSION, BLOCK_DIMENSION, quantize::none()};
//    auto decoded = invert(dctResult, ctx);
//
//    // todo check that decoded is at least close to the input
//    for (int i = 0; i < BLOCK_DIMENSION; ++i) {
//        for (int j = 0; j < BLOCK_DIMENSION; ++j) {
//            auto original = input(i, j);
//            auto result = decoded(i, j);
//            auto difference = std::abs(original - result); // todo they aren't as close as I was expecting...
//        }
//    }
//}

//TEST_CASE("Odd-sized inputs", "") {
//
//}

// Examples from https://matrix.reshish.com/
TEST_CASE("Matrix Transpose", "") {
    using namespace matrix;

    std::vector<int> inputData {
            1, 2, 3,
            4, 5, 6,
            7, 8, 9,
            10, 11, 12
    };

    std::vector<int> transposeData {
            1, 4, 7, 10,
            2, 5, 8, 11,
            3, 6, 9, 12
    };

    Matrix<int> input(4, 3, inputData);

    Matrix<int> result = transpose(input);
    REQUIRE(result.rows == input.cols);
    REQUIRE(result.cols == input.rows);
    REQUIRE(result.data == transposeData);
}

TEST_CASE("Matrix multiply", "") {
    std::vector<int> lhsData {
        5, 1,
        3, 2,
        9, 0,
        4, 7
    };
    std::vector<int> rhsData {
        3, 2, 10,
        8, -2, -10
    };
    std::vector<int> expectedData {
        23, 8, 40,
        25, 2, 10,
        27, 18, 90,
        68, -6, -30
    };

    matrix::Matrix<int> lhs(4, 2, lhsData);
    matrix::Matrix<int> rhs(2, 3, rhsData);
    auto result = matrix::multiply(lhs, rhs);

    REQUIRE(result.cols == 3);
    REQUIRE(result.rows == 4);
    REQUIRE(result.data == expectedData);
}

TEST_CASE("Matrix block operations", "") {

    std::vector<int> data(100);
    for (int i = 0; i < 100; ++i) {
        data.at(i) = i;
    }

    matrix::Matrix<int> m(10, 10, data);

    auto topLeft = m.getBlock(0, 0, 1, 1);
    REQUIRE(topLeft.rows == 1);
    REQUIRE(topLeft.cols == 1);
    REQUIRE(topLeft.at(0, 0) == 0);

    m.insertBlock(0, 0, matrix::Matrix<int>(1, 1, 101));
    auto newTopLeft = m.getBlock(0, 0, 1, 1);
    REQUIRE(newTopLeft.rows == 1);
    REQUIRE(newTopLeft.cols == 1);
    REQUIRE(newTopLeft.at(0, 0) == 101);

    auto bigBlock = m.getBlock(1, 1, 6, 4);
    REQUIRE(bigBlock.rows == 6);
    REQUIRE(bigBlock.cols == 4);
    for (int i = 0; i < bigBlock.rows; ++i) {
        for (int j = 0; j < bigBlock.cols; ++j) {
            auto expectedValue = (i+1)*10 + j + 1;
            REQUIRE(bigBlock.at(i, j) == expectedValue);
        }
    }

    m.insertBlock(0, 0, bigBlock);
    for (int i = 0; i < bigBlock.rows; ++i) {
        for (int j = 0; j < bigBlock.cols; ++j) {
            REQUIRE(bigBlock.at(i, j) == m.at(i, j));
        }
    }
}
