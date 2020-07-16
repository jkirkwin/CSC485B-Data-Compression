#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "binary.h"
#include "mtf.h"

typedef std::pair<std::vector<u8>, std::vector<u8>> input_oracle_pair_t;

input_oracle_pair_t getLongInputResultPair() {
    std::vector<u8> input {};
    std::vector<u8> expected {};
    for (int i = 0; i < 256; ++i) {
        input.push_back(i);
        expected.push_back(i);

        input.push_back(i);
        expected.push_back(0);
    }
    return {input, expected};
}

std::vector<input_oracle_pair_t> pairs {
        {
                {1, 2, 2, 3, 2},
                {1, 2, 0, 3, 1}
        },
        {
                {0, 0, 0, 2, 2, 2, 3, 0, 0, 0},
                {0, 0, 0, 2, 0, 0, 3, 2, 0, 0}
        },
        {
                getLongInputResultPair()
        },
        {
                {255},
                {255}
        },
};

TEST_CASE("Smoke test", "[mtf] [encode] [decode]") {
    std::vector<u8> input {1,2,4,5,2,0,255};
    REQUIRE(input == mtf::invert(mtf::transform(input)));
}

TEST_CASE("Transform", "[mtf] [encode]") {
    for (const auto& pair : pairs) {
        REQUIRE(pair.second == mtf::transform(pair.first));
    }
}

TEST_CASE("Invert", "[mtf] [decode]") {
    for (const auto& pair : pairs) {
        REQUIRE(pair.first == mtf::invert(pair.second));
    }
}
