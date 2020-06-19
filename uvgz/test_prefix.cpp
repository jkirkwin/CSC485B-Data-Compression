#define CATCH_CONFIG_MAIN

#include "catch/catch.hpp"
#include "prefix.h"
#include "shared/binary.h"

TEST_CASE("Canonical code generated successfully", "[prefix]") {
    // canonical code algorithm takes in a set of lengths and outputs a vector of bitsets
    // where the ith bitset is the symbol for the ith element with length input[i]
    using len_vec = std::vector<u32>;
    using result_vec = std::vector<bitset>;

    SECTION("Trivial 2-length input") {
        const len_vec input {1, 1}; // should generate the codewords 0 and 1 (one bit each)
        const result_vec expected {bitset(1, 0), bitset(1, 1)};
        const auto result = constructCanonicalCode(input);
        REQUIRE(expected == result);
    }

    SECTION("6-length input") {
        const len_vec input {2, 4, 2, 2, 4, 3};
        const result_vec expected {
            bitset(2u, 0b00),
            bitset(4u, 0b1110),
            bitset(2u, 0b01),
            bitset(2u, 0b10),
            bitset(4u, 0b1111),
            bitset(3u, 0b110)
        };
        const auto result = constructCanonicalCode(input);
        REQUIRE(expected == result);
    }
}

TEST_CASE("Limited-length huffman generates acceptable code") {
    std::vector<u32> weights = {5, 10, 15, 20, 20, 30}; // adapted from sayood
    std::vector<u32> expectedResult = {3, 3, 3, 3, 2, 2};

    SECTION("Simple example") {
        auto result = package_merge::getCodeLengths(weights, 3);
        REQUIRE(result == expectedResult);
    }

    SECTION("0 weights are ignored") {
        std::vector<u32> weightsWithZeros;
        std::vector<u32> expectedWithZeros;
        for (int i = 0; i < weights.size(); ++i) {
            weightsWithZeros.push_back(0);
            expectedWithZeros.push_back(0);

            weightsWithZeros.push_back(weights.at(i));
            expectedWithZeros.push_back(expectedResult.at(i));

            weightsWithZeros.push_back(0);
            expectedWithZeros.push_back(0);
        }

        auto result = package_merge::getCodeLengths(weights, 3);
        REQUIRE(result == expectedResult);
    }
}