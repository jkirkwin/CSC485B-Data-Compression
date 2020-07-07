#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#endif

#include "catch.hpp"
#include "bwt.h"
#include "binary.h"
#include "iostream"

typedef bwt::BwtResult enc_t;
typedef std::vector<u8> dec_t;

std::vector<dec_t> decoded {
    {1},
    {1, 2, 3, 1, 2, 3},
    {0, 0, 0, 0, 0, 0},

    // Courtesy of https://www.dcode.fr/burrows-wheeler-transform
    {9, 8, 7, 4, 6, 4, 8, 2, 3, 4, 5, 6, 9, 7, 200, 4, 5, 3, 5, 6, 3, 2, 1}
};

std::vector<enc_t> encoded {
    enc_t({1}, 0),
    enc_t({3, 3, 1, 1, 2, 2}, 0),
    enc_t({0, 0, 0, 0, 0, 0}, 0),

    // Courtesy of https://www.dcode.fr/burrows-wheeler-transform
    enc_t({2, 3, 8, 6, 2, 5, 200, 3, 7, 6, 4, 3, 4, 5, 4, 5, 8, 9, 4, 9, 6, 1, 7}, 21)
};

TEST_CASE("Encoding gives expected result", "[bwt] [encode]") {
    assert(encoded.size() == decoded.size());
    for (int i = 0; i < encoded.size(); ++i) {
        auto result = bwt::encode(decoded.at(i));
        auto expected = encoded.at(i);
        REQUIRE(result == expected);
    }
}

TEST_CASE("Decoding gives expected result", "[bwt] [decode]") {
    assert(encoded.size() == decoded.size());
    for (int i = 0; i < encoded.size(); ++i) {
        auto result = bwt::decode(encoded.at(i));
        auto expected = decoded.at(i);
        REQUIRE(result == expected);
    }
}

TEST_CASE("Smoke test", "[bwt] [encode] [decode]") {
    std::vector<u8> input;
    const int limit = 255;
    for (int i = 0; i < limit; ++i) {
        input.push_back((u8)i);
        input.push_back((u8)(i+1));
        const auto result = bwt::decode(bwt::encode(input));
        REQUIRE(input == result);
    }
}
