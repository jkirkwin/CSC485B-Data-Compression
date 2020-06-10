/**
 * Handy binary definitions and utilities.
 */

#ifndef UVGZ_BINARY_H
#define UVGZ_BINARY_H

#include <cstdint>
#include <boost/dynamic_bitset.hpp>
#include <cassert>

// Aliases for readability
using bitset = boost::dynamic_bitset<>;
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;


/**
 * Computes the number of bits required to store the given value. The returned
 * value may not be the minimum possible number, depending on the parameters.
 * @param value The value to store
 * @param minBits May be specified to save time in favour of potential memory
 * overhead. Must be strictly positive.
 */
inline unsigned int bitsRequired(unsigned int value, unsigned int minBits = 1) {
    assert (minBits > 0);
    unsigned int bitsNeeded = minBits;
    while( (value >> bitsNeeded) > 0) {
        ++bitsNeeded;
    }
    return bitsNeeded;
}


/**
 * Returns a bitset containing the given value using <= minBits bits.
 */
inline bitset getMinBitset(unsigned int value, unsigned int minBits = 1) {
    const auto bits = bitsRequired(value, minBits);
    return bitset(bits, value);
}

#endif //UVGZ_BINARY_H
