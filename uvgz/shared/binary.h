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

constexpr u16 kb = (1u << 10u);
constexpr u32 mb = (1u << 20u);

#endif //UVGZ_BINARY_H
