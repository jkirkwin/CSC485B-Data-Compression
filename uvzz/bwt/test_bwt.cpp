#ifndef CATCH_CONFIG_MAIN
#define CATCH_CONFIG_MAIN
#endif

#include "catch.hpp"
#include "bwt.h"
#include "binary.h"

TEST_CASE("Placeholder", "[bwt]") {
    u8 oracle {2};
    REQUIRE(bwt::bar() == oracle);
}
