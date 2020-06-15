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

TEST_CASE("Fixed codes lengths are correct", "[prefix]  [type1]") {
    SECTION("LL code lengths are correct") {
        const auto llCodeLengths = getFixedLLCodeLengths();
        const int numSymbols {288};
        REQUIRE (llCodeLengths.size() == numSymbols);

        int i = 0;
        while (i < numSymbols) {
            const auto len = llCodeLengths[i];
            if (i <= 143) {
                REQUIRE(len == 8);
            }
            else if (i <= 255) {
                REQUIRE(len == 9);
            }
            else if (i <= 279) {
                REQUIRE(len == 7);
            }
            else {
                REQUIRE(len == 8);
            }
            ++i;
        }
    }
    SECTION("Distance code lengths are correct") {
        const auto distCodeLengths = getFixedDistanceCodeLengths();
        const auto numSymbols = 32;
        REQUIRE(numSymbols == distCodeLengths.size());

        for(const auto len : distCodeLengths) {
            REQUIRE(len == 5);
        }
    }
}