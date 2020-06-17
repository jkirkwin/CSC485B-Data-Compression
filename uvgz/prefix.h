#ifndef UVGZ_PREFIX_H
#define UVGZ_PREFIX_H

#include <vector>
#include "shared/binary.h"

// length code type 1:
// 0-143 -> 8 bits
// 144-255 -> 9 bits
// 256-279 -> 7 bits
// 280-287 -> 8 bits
constexpr std::array<std::array<u32, 3>, 4> fixedLLCodeLengths {{
    {{0, 143, 8}},
    {{144, 255, 9}},
    {{256, 279, 7}},
    {{280, 287, 8}}
}};


/**
 * To be used with constructCanonicalCode()
 * @return The lengths used for the fixed block type 1 LL code.
 */
std::vector<u32> getFixedLLCodeLengths();

/**
 * To be used with constructCanonicalCode()
 * @return The lengths used for the fixed block type 1 Distance code.
 */
std::vector<u32> getFixedDistanceCodeLengths();


/** Adapted from Bill's provided code.
 *
 * Given a vector of lengths where lengths.at(i) is the code length for symbol
 * i, returns a vector of codewords with the given lengths.
 *
 * The codes for symbols with length zero are undefined.
 */
std::vector<bitset> constructCanonicalCode(std::vector<u32> const & lengths);

/**
 * @return A vector of code words for the fixed type 1 LL code in the format
 * specified by constructCanonicalCode().
 */
inline std::vector<bitset> getFixedLLCode() {
    return constructCanonicalCode(getFixedLLCodeLengths());
}

/**
 * @return A vector of code words for the fixed type 1 distance code in the
 * format specified by constructCanonicalCode().
 */
inline std::vector<bitset> getFixedDistanceCode() {
    return constructCanonicalCode(getFixedDistanceCodeLengths());
}

// todo - type 2 - generate dynamic codes
// todo - type 2 - CL code specifics
// todo - optimization - package-merge implementation

#endif //UVGZ_PREFIX_H
