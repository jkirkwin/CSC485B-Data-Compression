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

TEST_CASE("Dynamic codes lengths successfully generated", "[prefix] [type2]") {
    REQUIRE(false); // todo
}

TEST_CASE("Huffman generates correct code lengths") {
    SECTION("Trivial case") {
        std::vector<u32> weights {1, 2};
        std::vector<u32> expected {1, 1};
        REQUIRE (huffman::getCodeLengths(weights) == expected);
    }
    SECTION("Input contains weights of size 0") {
        std::vector<u32> weights {1, 0, 2};
        std::vector<u32> expected {1, 0, 1};
        REQUIRE (huffman::getCodeLengths(weights) == expected);
    }

    SECTION("Large input") {
        // Example adapted from https://en.wikipedia.org/wiki/Huffman_coding#Compression
        std::vector<u32> weights {1, 0, 1, 4, 0, 1, 1, 1, 0,
                                  2, 2, 2, 2, 0, 2, 0, 0, 2,
                                  3, 4, 0, 1, 7, 0, 0, 0, 0};
        std::vector<u32> expected {5, 0, 5, 3, 0, 5, 5, 5, 0,
                                   4, 4, 4, 4, 0, 4, 0, 0, 4,
                                   4, 3, 0, 5, 3, 0 ,0 ,0 ,0};
        REQUIRE (huffman::getCodeLengths(weights) == expected);
    }
}