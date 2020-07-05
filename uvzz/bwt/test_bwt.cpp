#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#endif

#include "catch.hpp"
#include "bwt.h"
#include "binary.h"

TEST_CASE("Encoding gives expected result", "[bwt] [encode]") {
    // TODO implement
    REQUIRE(false);
}

TEST_CASE("Decoding gives expected result", "[bwt] [decode]") {
    // TODO implement
    REQUIRE(false);
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
