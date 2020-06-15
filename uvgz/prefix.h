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
std::vector<u32> getFixedLLCodeLengths() {
    std::vector<u32> lengths {};

    const auto entryCount = fixedLLCodeLengths.size();
    const auto lastEntry = fixedLLCodeLengths[entryCount - 1];
    const auto totalLengths = lastEntry[1] + 1;
    lengths.reserve(totalLengths);

    for (auto range : fixedLLCodeLengths) {
        const auto rangeStart {range[0]};
        const auto rangeEnd {range[1]};
        const auto bits {range[2]};
        for(int i = rangeStart; i <= rangeEnd; i++) {
            lengths.push_back(bits);
        }
    }
    return lengths;
}

/**
 * To be used with constructCanonicalCode()
 * @return The lengths used for the fixed block type 1 Distance code.
 */
std::vector<u32> getFixedDistanceCodeLengths() {
    // 32 lengths at 5 bits each
    std::vector<u32> lengths {};
    lengths.reserve(32);
    const u32 bits = 5;

    for(int i = 0; i < 32; i++) {
        lengths.push_back(bits);
    }
    return lengths;
}


/** Adapted from Bill's provided code.
 *
 * Given a vector of lengths where lengths.at(i) is the code length for symbol
 * i, returns a vector of codewords with the given lengths.
 *
 * The codes for symbols with length zero are undefined.
 */
std::vector< bitset > constructCanonicalCode( std::vector<u32> const & lengths ){

    unsigned int size = lengths.size();
    std::vector< unsigned int > length_counts(16,0); //Lengths must be less than 16 for DEFLATE
    u32 max_length = 0;
    for(auto i: lengths){
        assert(i <= 15);
        length_counts.at(i)++;
        max_length = std::max(i, max_length);
    }
    length_counts[0] = 0; //Disregard any codes with alleged zero length

    std::vector< bitset > result_codes(size);

    //The algorithm below follows the pseudocode in RFC 1951
    std::vector< unsigned int > next_code(size,0);
    {
        //Step 1: Determine the first code for each length
        unsigned int code = 0;
        for(unsigned int i = 1; i <= max_length; i++){
            code = (code+length_counts.at(i-1))<<1;
            next_code.at(i) = code;
        }
    }
    {
        //Step 2: Assign the code for each symbol, with codes of the same length being
        //        consecutive and ordered lexicographically by the symbol to which they are assigned.
        for(unsigned int symbol = 0; symbol < size; symbol++){
            unsigned int length = lengths.at(symbol);
            if (length > 0) {
                result_codes.at(symbol) = bitset(length, next_code.at(length)++);
            }
        }
    }
    return result_codes;
}

/**
 * @return A vector of code words for the fixed type 1 LL code in the format
 * specified by constructCanonicalCode().
 */
std::vector<bitset> getFixedLLCode() {
    return constructCanonicalCode(getFixedLLCodeLengths());
}

/**
 * @return A vector of code words for the fixed type 1 distance code in the
 * format specified by constructCanonicalCode().
 */
std::vector<bitset> getFixedDistanceCode() {
    return constructCanonicalCode(getFixedDistanceCodeLengths());
}

// todo - type 2 - CL code specifics
// todo - type 2 - generate dynamic codes
// todo - optimization - package-merge implementation

#endif //UVGZ_PREFIX_H
