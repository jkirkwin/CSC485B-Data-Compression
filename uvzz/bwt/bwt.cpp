#include "bwt.h"

namespace bwt {

    BwtResult encode(const std::vector<u8>& input) {
        return {input, 0}; // todo stub
    }

    std::vector<u8> decode(const BwtResult& bwtResult) {
        return bwtResult.data; // todo stub
    }
}

