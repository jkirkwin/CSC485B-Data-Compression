#ifndef UVG_DELTA_H
#define UVG_DELTA_H

#include <vector>
#include <boost/dynamic_bitset.hpp>
#include "input_stream.hpp"

/**
 * A simple delta/differential encoding library.
 *
 * Encodings use a signed, variable bit representation and are encoded as follows:
 * Let len = |k-n| be the magnitude of the difference to be encoded.
 * Let bin be the binary encoding of len
 * Let count be the number of bits needed to represent bin. For the special case of len = 0, define count = 0.
 * Let u be the unary encoding of count.
 * Then length is encoded as:
 * * u, followed by
 * * the least max{count-1, 0} bits of bin, followed by
 * * a sign bit to indicate the signedness of the initial length input,
 * unless the length is zero, in which case this is omitted. 0 indicates
 * positive and 1 indicates negative.
 *
 * Examples:
 * <pre>
 *  len     bin     count       u(count)     Sign     encoding    <br>
 *   0         0     0               0       N/A      0           <br>
 *   1         1     1              10       1        100          <br>
 *  -2        10     2             110       0        11001        <br>
 *  -3        11     2             110       0        11011        <br>
 *   4       100     3            1110       1        1110000      <br>
 *   5       101     3            1110       1        1110010      <br>
 *  -6       110     3            1110       0        1110101      <br>
 *  10      1010     4           11110       1        111100100    <br>
 * </pre>
 *
 * This method is used because it allows the size of the encoding to grow
 * proportionally to the size of the difference, and allows a very low cost
 * encoding for 0's.
 */
namespace delta {
    using bitset = boost::dynamic_bitset<>;

    /**
     * Encode The given size into a bit set.
     */
    bitset encode(long diff);

    inline bitset encode(int magnitude, int previous) {
        return encode(magnitude - previous);
    }

    /**
     * Decode the given encoding into the original size.
     */
    long decode(const bitset& encoded);

    /**
     * Read the first encoded size from the bit stream and return it.
     */
    long decodeFromBitStream(InputBitStream& inputBitStream);

}

#endif //UVG_DELTA_H
