/**
 * Structures and utilities used to create and interpret LZSS back-references.
 */

#ifndef UVGZ_LZSS_BACKREF_H
#define UVGZ_LZSS_BACKREF_H

#include <cassert>
#include "shared/binary.h"

const unsigned int LITERAL_BITS = 8;
const unsigned int LENGTH_BASE_BITS = 9;
const unsigned int DISTANCE_BASE_BITS = 5;

/**
 * Elements are of the form {base length code, offset bits, range start, range end}
 */
const std::array<std::array<unsigned int, 4>, 29> lengthCodeTable {{
      {{257, 0, 3, 3}},
      {{258, 0, 4, 4}},
      {{259, 0, 5, 5}},
      {{260, 0, 6, 6}},
      {{261, 0, 7, 7}},
      {{262, 0, 8, 8}},
      {{263, 0, 9, 9}},
      {{264, 0, 10, 10}},
      {{265, 1, 11, 12}},
      {{266, 1, 13, 14}},
      {{267, 1, 15, 16}},
      {{268, 1, 17, 18}},
      {{269, 2, 19, 22}},
      {{270, 2, 23, 26}},
      {{271, 2, 27, 30}},
      {{272, 2, 31, 34}},
      {{273, 3, 35, 42}},
      {{274, 3, 43, 50}},
      {{275, 3, 51, 58}},
      {{276, 3, 59, 66}},
      {{277, 4, 67, 82}},
      {{278, 4, 83, 98}},
      {{279, 4, 99, 114}},
      {{280, 4, 115, 130}},
      {{281, 5, 131, 162}},
      {{282, 5, 163, 194}},
      {{283, 5, 195, 226}},
      {{284, 5, 227, 257}},
      {{285, 0, 258, 258}}
}};

/**
 * Elements are of the form {base distance code, offset bits, range start, range end}
 */
const std::array<std::array<unsigned int, 4>, 30> distanceCodeTable {{
    {{ 0, 0, 1, 1 }},
    {{ 1, 0, 2, 2 }},
    {{ 2, 0, 3, 3 }},
    {{ 3, 0, 4, 4 }},
    {{ 4, 1, 5, 6 }},
    {{ 5, 1, 7, 8 }},
    {{ 6, 2, 9, 12 }},
    {{ 7, 2, 13, 16 }},
    {{ 8, 3, 17, 24 }},
    {{ 9, 3, 25, 32 }},
    {{ 10, 4, 33, 48 }},
    {{ 11, 4, 49, 64 }},
    {{ 12, 5, 65, 96 }},
    {{ 13, 5, 97, 128 }},
    {{ 14, 6, 129, 192 }},
    {{ 15, 6, 193, 256 }},
    {{ 16, 7, 257, 384 }},
    {{ 17, 7, 385, 512 }},
    {{ 18, 8, 513, 768 }},
    {{ 19, 8, 769, 1024 }},
    {{ 20, 9, 1025, 1536 }},
    {{ 21, 9, 1537, 2048 }},
    {{ 22, 10, 2049, 3072 }},
    {{ 23, 10, 3073, 4096 }},
    {{ 24, 11, 4097, 6144 }},
    {{ 25, 11, 6145, 8192 }},
    {{ 26, 12, 8193, 12288 }},
    {{ 27, 12, 12289, 16384 }},
    {{ 28, 13, 16385, 24576 }},
    {{ 29, 13, 24577, 32768 }}
}};

/**
 * Get the length of a backreference using the given base length symbol and
 * offset.
 * @param baseSymbol A bitset containing the symbol for the base length.
 * @param offsetSymbol A bitset containing the offset.
 * @return The length of the reference.
 */
inline unsigned int getBackrefLength(const bitset& baseSymbol, const bitset& offsetSymbol) {
    const auto minSymbol = lengthCodeTable[0][0];
    assert (baseSymbol.to_ulong() >= minSymbol);

    const auto index = baseSymbol.to_ulong() - minSymbol;
    const auto tableEntry = lengthCodeTable[index];
    const auto baseValue = tableEntry[2];

    const auto offsetValue = offsetSymbol.to_ulong();
    const auto maxPossibleOffset = (1u << tableEntry[1]) - 1;
    assert (offsetValue <= maxPossibleOffset);

    return baseValue + offsetValue;
}

/**
 * Return a pair of bitsets representing the length given.
 */
inline std::pair<bitset, bitset> getLengthBackref(unsigned int length) {
    assert (length >= 0);
    auto const maxLen = lengthCodeTable[lengthCodeTable.size() - 1][3];
    assert (length <= maxLen);

    // todo this could be sped up
    // todo factor this out so its easier to optimize later
    // linear search for the length

    int i;
    for (i = 0; i < lengthCodeTable.size(); ++i) {
        const auto cell = lengthCodeTable[i];
        if (cell[3] >= length) {
            break;
        }
    }

    const auto cell = lengthCodeTable[i];
    const bitset base(LENGTH_BASE_BITS, cell[0]);
    const auto offsetMagnitude = length - cell[2];
    const int offsetBits = cell[1];
    const bitset offset(offsetBits, offsetMagnitude);
    return std::make_pair(base, offset);
}

/**
 * Get the distance of a back-reference given its two components.
 * @param baseSymbol Then encoding of the base distance.
 * @param offsetSymbol The offset from the base distance.
 * @return The total distance encoded in the two bitsets.
 */
inline unsigned int getBackrefDistance(const bitset& baseSymbol, const bitset& offsetSymbol) {
    const auto minSymbol = distanceCodeTable[0][0];
    assert (baseSymbol.to_ulong() >= minSymbol);

    const auto index = baseSymbol.to_ulong() - minSymbol;
    const auto tableEntry = distanceCodeTable[index];
    const auto baseValue = tableEntry[2];

    const auto offsetValue = offsetSymbol.to_ulong();
    const auto maxPossibleOffset = (1u << tableEntry[1]) - 1;
    assert (offsetValue <= maxPossibleOffset);

    return baseValue + offsetValue;
}

inline std::pair<bitset, bitset> getDistanceBackref(unsigned int distance) {
    assert (distance >= 1);
    auto const maxDistance = distanceCodeTable[distanceCodeTable.size() - 1][3];
    assert (distance <= maxDistance);

    // todo this could be sped up
    // todo factor this out so its easier to optimize later
    // linear search for the length

    int i;
    for (i = 0; i < distanceCodeTable.size(); ++i) {
        const auto cell = distanceCodeTable[i];
        if (cell[3] >= distance) {
            break;
        }
    }

    const auto cell = distanceCodeTable[i];

    const auto baseSymbol = cell[0];
    const bitset base(DISTANCE_BASE_BITS, baseSymbol);

    const auto offsetMagnitude = distance - cell[2];
    const int offsetBits = cell[1];
    const bitset offset(offsetBits, offsetMagnitude);

    return std::make_pair(base, offset);
}

#endif