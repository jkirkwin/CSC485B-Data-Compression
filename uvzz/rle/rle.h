#ifndef UVZZ_RLE_H
#define UVZZ_RLE_H

#include <utility>
#include <vector>
#include "binary.h"

/**
 * RLE stands for Run Length Encoding: a simple encoding scheme template by
 * which runs of consecutive symbols are replaced by a (symbol, repetitions)
 * pair.
 *
 * This library provides performs the encoding and decoding operations for a
 * specialized RLE scheme which encodes runs of 0's only:
 * <ul><li>Any non-zero character is encoded as a literal (8-bit) value.</li>
 * <li>For any run of 1 <= k <= n-1 zeros, k literals are encoded.</li>
 * <li>For any run of n <= k zeros, n literals are encoded, followed by a
 * length field representing the value k-n</li>
 * </ul>
 *
 * A length field is of variable length and is encoded as follows:
 * Let len = k-n be the run "continuation", or the number of zeros following the initial run of n zeros
 * Let bin be the binary encoding of len
 * Let count be the number of bits needed to represent bin. For the special case of len = 0, define count = 0.
 * Let u be the unary encoding of count.
 * Then length is encoded as u followed by the least max{count-1, 0} bits of bin
 *
 * Examples:
 * <pre>
 *  len     bin     count       u(count)    encoding    <br>
 *  0       0       0           0           0           <br>
 *  1       1       1           10          10          <br>
 *  2       10      2           110         1100        <br>
 *  3       11      2           110         1101        <br>
 *  4       100     3           1110        111000      <br>
 *  5       101     3           1110        111001      <br>
 *  6       110     3           1110        111010      <br>
 *  10      1010    4           11110       11110010    <br>
 * </pre>
 */
namespace rle {
    const u32 CONTINUATION_THRESHOLD = 4; // todo revisit this and determine the best value to use.

    /**
     * A simple tagged union representing a possible output of the RLE encoding process.
     */
    class Symbol {
    public:
        explicit Symbol(u8 literal): literal(literal), active(&this->literal){

        }

        explicit Symbol(bitset length): length(std::move(length)), active(&this->length) {

        }

        bool isLiteral() const {
            return active == &literal;
        }

        bool isLength() const {
            return !isLiteral();
        }

        u8 getLiteral() const {
            assert(isLiteral());
            return literal;
        }

        bitset getLength() const {
            assert(isLength());
            return length;
        }

    private:
        u8 literal{};
        bitset length;
        void* active;
    };

    inline bool operator==(const Symbol& s1, const Symbol& s2) {
        if (s1.isLiteral() != s2.isLiteral()) {
            return false;
        }
        else if (s1.isLiteral()) {
            return s1.getLiteral() == s2.getLiteral();
        }
        else {
            return s1.getLength() == s2.getLength();
        }
    }

    /**
     * Encode the given byte-oriented data into an encoded symbol sequence.
     * @param rawData The raw data to be encoded.
     * @return The encoded symbol sequence.
     */
    std::vector<Symbol> encode(const std::vector<u8>& rawData);

    /**
     * Convert the given length into an encoded length field.
     * @param length The run length to be encoded.
     * @return The encoded length symbol.
     */
    bitset symbolFromLength(u32 length);

    /**
     * Convert an encoded length field into a numeric value.
     * @param lenSymbol The encoded length field.
     * @return The decoded length value.
     */
    u32 lengthFromSymbol(const bitset& lenSymbol);
}

/**
 * Alternate variable-byte rle encoding functions. These are useful for
 * performing generic RLE where the output should remain byte-oriented.
 *
 * A similar scheme is followed here wherein continuation lengths are
 * encoded only after seeing some number of repetitions of an input byte.
 */
namespace rle::vb {
    const u32 REPEATS_DEFAULT = 3;

    typedef std::vector<u8> byte_vec;

    /**
     * @param input The bytes to perform RLE on.
     * @param repeats The minimum run length after which to encode a length field.
     * @return The encoded result.
     */
    byte_vec encode(const byte_vec& input, u32 repeats=REPEATS_DEFAULT);

    /**
     * @param encoded The encoded RLE result.
     * @param repeats The minimum run length after which to decode a length field.
     * @return The decoded result.
     */
    byte_vec decode(const byte_vec& encoded, u32 repeats=REPEATS_DEFAULT);
}

#endif