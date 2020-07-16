#ifndef UVZZ_BINARY_H
#define UVZZ_BINARY_H

#include <cstdint>
#include <boost/dynamic_bitset.hpp>

using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using bitset = boost::dynamic_bitset<>;

namespace binary::vb {
    const u32 MASK = 0x7F; // 0111 1111
    const u32 FLAG = 0x80; // 1000 0000


    /**
     * Encode a number in a variable number of bytes.
     * The result will have bytes ordered from most to least significant.
     * @param n The number to encode.
     * @return A sequence of bytes which encode n.
     */
    inline std::vector<u8> encode(u32 n) {
        bitset b(32, n);

        std::vector<u8> result {};
        result.push_back(n & MASK);

        // get each set of 7 bits from n until we have read the most significant 1 bit.
        while (n>>7u > 0) {
            n = n >> 7u;
            u8 next = (n & MASK) | FLAG;
            result.push_back(next);
        }

        std::reverse(result.begin(), result.end());
        return result;
    }

    /**
     * Decode a variable byte encoding.
     * @param encoded The encoded byte sequence.
     * @return The original number.
     */
    inline u32 decode(const std::vector<u8>& encoded) {
        u32 result {0};
        for (int i = 0; i < encoded.size(); ++i) {
            auto byte = encoded.at(i);

            assert ( (i == encoded.size() - 1) == ((byte & FLAG) == 0) );

            auto next7 = byte & MASK;
            result = (result << 7u) | next7;
        }
       return result;
    }
}


#endif