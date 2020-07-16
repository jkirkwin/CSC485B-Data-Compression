#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "rle.h"
#include <fstream>
#include "constants.h"
#include "output_stream.h"
#include "input_stream.h"

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

TEST_CASE("Smoke test variable byte RLE", "[vb] [rle] [decode] [encode]") {
    std::vector<u8> input {};
    int repeatThreshold = 4;
    // Minimal run to require length field
    for (int i = 0; i < repeatThreshold; ++i) {
        input.push_back(1);
    }
    // String of non-repeating characters
    for (int i = 0; i < 100; ++i) {
        input.push_back(i);
    }
    // Repeated chars without length field
    for (int i = 0; i < repeatThreshold-1; ++i) {
        input.push_back(2);
    }
    // Long sequence of repetitions
    for (int i = 0; i < 10; ++i) {
        input.push_back(0xFF);
    }
    // Add one more for fun
    input.push_back(8);

    auto encoded = rle::vb::encode(input, repeatThreshold);
    assert (encoded.size() < input.size());
    auto result = rle::vb::decode(encoded, repeatThreshold);
    REQUIRE(input == result);
}

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
        input =  {0, 1, 0, 3, 2};
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

TEST_CASE("Decode length from bitstream", "[decode]") {
    // Create a dummy file to write to
    const auto filepath = "/tmp/uvzz_test_rle.bin";

    for (u32 length = 0; length < BLOCK_SIZE * 2; ++length) {
        std::ofstream fOut(filepath);
        auto lengthField = rle::symbolFromLength(length);
        OutputBitStream outStream(fOut);
        outStream.push_bits_msb_first(lengthField);
        outStream.flush_to_byte();
        fOut.close();

        std::ifstream fIn(filepath);
        InputBitStream inStream(fIn);
        auto resultLength = rle::readLengthFromBitstream(inStream);
        fIn.close();

        REQUIRE (length == resultLength);
    }
}
