#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "binary.h"

TEST_CASE("Smoke test encode/decode", "vb") {
    for (u32 i = 0; i < 10000; ++i) {
        auto encoded = binary::vb::encode(i);
        auto decoded = binary::vb::decode(encoded);
        REQUIRE(decoded == i);
    }
}