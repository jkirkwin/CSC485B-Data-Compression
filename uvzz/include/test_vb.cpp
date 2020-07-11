#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "binary.h"

typedef u32 raw_t;
typedef std::vector<u8> enc_t;
std::vector<std::pair<raw_t, enc_t>> pairs {
        {0, {0}},
        {1, {1}},
        {127, {127}},
        {128, {0x81, 0}},
        {129, {0x81, 1}},
        {255, {0x81, 0x7F}},
        {256, {0x82, 0}},
        {800, {0x86, 0x20}},
        {0x7FFF, {0x81, 0xFF, 0x7F}},
};

TEST_CASE("Smoke test encode/decode", "[vb]") {
    for (u32 i = 0; i < 10000; ++i) {
        auto encoded = binary::vb::encode(i);
        auto decoded = binary::vb::decode(encoded);
        REQUIRE(decoded == i);
    }
}

TEST_CASE("Test encode lengths", "[vb]") {
    for (const auto& pair : pairs) {
        REQUIRE(pair.second == binary::vb::encode(pair.first));
    }
}

TEST_CASE("Test decode lengths", "[vb]") {
    for (const auto& pair : pairs) {
        REQUIRE(pair.first == binary::vb::decode(pair.second));
    }
}