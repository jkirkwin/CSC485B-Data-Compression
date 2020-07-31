#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "matrix.h"

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

TEST_CASE("getting and setting cell value") {
    std::vector<int> data(4);
    for (int i = 0; i < 4; ++i) {
        data.at(i) = i;
    }

    matrix::Matrix<int> m(2, 2, data);

    REQUIRE(m.at(0,0) == 0);
    m.set(0,0) = -1;
    REQUIRE(m.at(0,0) == -1);
}
