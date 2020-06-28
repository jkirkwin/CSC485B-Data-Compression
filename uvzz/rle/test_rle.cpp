#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "rle.h"

TEST_CASE("Placeholder", "[rle]") {
    u8 oracle {1};
    REQUIRE(rle::foo() == oracle);
}
