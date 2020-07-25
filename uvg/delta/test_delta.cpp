#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "delta.h"
#include "input_stream.hpp"
#include "output_stream.hpp"

using namespace delta;

typedef std::pair<long, bitset> dec_enc_pair;

std::vector<dec_enc_pair> pairs {
        { 0, bitset(1, 0b0u) },
        { -1, bitset(3, 0b101u) },
        { 2, bitset(5, 0b11000u) },
        { -3, bitset(5, 0b11011u) },
        { 4, bitset(7, 0b1110000u) },
        { -5, bitset(7, 0b1110011u) },
        { 6, bitset(7, 0b1110100u) },
        { -10, bitset(9, 0b111100101u) },
        { 1000, bitset(21, 0b111111111101111010000u) } // 1000 = 0b1111101000
};

TEST_CASE("Test encode", "") {
    for (const auto& pair : pairs) {
        REQUIRE(encode(pair.first) == pair.second);
    }
}

TEST_CASE("Test decode", "") {
    for (const auto& pair : pairs) {
        REQUIRE(decode(pair.second) == pair.first);
    }
}

TEST_CASE("Smoke test conversion", "") {
    for (const auto& pair : pairs) {
        REQUIRE(decode(encode(pair.first)) == pair.first);
        REQUIRE(encode(decode(pair.second)) == pair.second);
    }
}

TEST_CASE("Decode from bitstream", "") {
    // Create a dummy file to write to
    const auto filepath = "/tmp/uvg_test_delta.bin";

    for (int size = -100; size < 100; ++size) {
        std::ofstream fOut(filepath);
        auto encoded = encode(size);
        OutputBitStream outStream(fOut);
        outStream.push_bits_msb_first(encoded);
        outStream.flush_to_byte();
        fOut.close();

        std::ifstream fIn(filepath);
        InputBitStream inStream(fIn);
        auto result = decodeFromBitStream(inStream);
        fIn.close();

        REQUIRE(size == result);
    }
}

