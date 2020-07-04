#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "rle.h"

typedef std::pair<u32, bitset> dec_enc_pair;

std::vector<dec_enc_pair> pairs {
        { 0, bitset(1, 0b0u) },
        { 1, bitset(2, 0b10u) },
        { 2, bitset(4, 0b1100u) },
        { 3, bitset(4, 0b1101u) },
        { 4, bitset(6, 0b111000u) },
        { 5, bitset(6, 0b111001u) },
        { 6, bitset(6, 0b111010u) },
        { 10, bitset(8, 0b11110010u) },
        { 1000, bitset(20, 0b11111111110111101000u) } // 1000 = 0b1111101000
};

TEST_CASE("Test encode length", "[rle] [encode]") {
    for (const auto& pair : pairs) {
        REQUIRE(rle::symbolFromLength(pair.first) == pair.second);
    }
}

TEST_CASE("Test decode length", "[rle] [decode]") {
    for (const auto& pair : pairs) {
        REQUIRE(rle::lengthFromSymbol(pair.second) == pair.first);
    }
}

TEST_CASE("Smoke test length conversions", "[rle] [encode] [decode]") {
    for (const auto& pair : pairs) {
        REQUIRE(rle::lengthFromSymbol(rle::symbolFromLength(pair.first)) == pair.first);
        REQUIRE(rle::symbolFromLength(rle::lengthFromSymbol(pair.second)) == pair.second);
    }
}

TEST_CASE("Test encode sequence", "[rle] [encode]") {
    std::vector<u8> input {};
    std::vector<rle::Symbol> expected {};
    SECTION("No runs") {
        input =  {0, 1, 0, 0, 2};
        for (u8 byte : input) {
            expected.push_back(rle::Symbol(byte));
        }
    }
    SECTION("Minimal run") {
        for (int i = 0; i < rle::CONTINUATION_THRESHOLD; ++i) {
            input.push_back(0);
            expected.push_back(rle::Symbol(0));
        }
        expected.push_back(rle::Symbol(rle::symbolFromLength(0)));

        REQUIRE(expected == rle::encode(input));
    }
    SECTION("Long run") {
        auto continuationLen = 100;
        for (int i = 0; i < rle::CONTINUATION_THRESHOLD + continuationLen; ++i) {
            input.push_back(0);
        }
        for (int i = 0; i < rle::CONTINUATION_THRESHOLD; ++i) {
            expected.push_back(rle::Symbol(0));
        }
        expected.push_back(rle::Symbol(rle::symbolFromLength(continuationLen)));
    }
    const auto result = rle::encode(input);
    REQUIRE(result == expected);
}