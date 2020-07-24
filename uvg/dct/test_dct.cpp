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

TEST_CASE("Odd-sized inputs", "") {

}

TEST_CASE("Matrix Transpose") {
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

    Matrix<int> result = transpose<int>(input);
    REQUIRE(result.rows == input.cols);
    REQUIRE(result.cols == input.rows);
    REQUIRE(result.data == transposeData);
}

TEST_CASE("Matrix multiply", "") {
    REQUIRE(false); // todo
}

TEST_CASE("Matrix insert block", "") {
    REQUIRE(false); // todo
}

