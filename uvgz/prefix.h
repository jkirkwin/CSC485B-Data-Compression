#ifndef UVGZ_PREFIX_H
#define UVGZ_PREFIX_H

#include <vector>
#include "shared/binary.h"

using bitset_vec = std::vector<bitset>;

/**
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
std::vector<bitset> getFixedLLCode();

/**
 * @return A vector of code words for the fixed type 1 distance code in the
 * format specified by constructCanonicalCode().
 */
std::vector<bitset> getFixedDistanceCode();


std::pair<bitset_vec, bitset_vec> getDynamicCodes(const bitset_vec& lzssSymbols);

// todo - type 2 - generate dynamic codes
// todo - type 2 - CL code specifics
// todo - optimization - package-merge implementation

#endif //UVGZ_PREFIX_H
