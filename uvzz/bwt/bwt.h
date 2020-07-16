#ifndef UVZZ_BWT_H
#define UVZZ_BWT_H

#include <vector>
#include "binary.h"

/**
 * Contains encoding and decoding functions for the Burrows Wheeler Transform.
 * This transform does not do any compression on its own, but rearranges data
 * so that other techniques (often Run-Length Encoding [RLE]) may be more
 * effective.
 *
 * The forward algorithm is as follows:
 * <ul>
 * <li>Accept input a_1, a_2, ... , a_n</li>
 * <li>Create an n-by-n matrix of all possible shifts of a_1, ... , a_n</li>
 * <li>Sort the rows of the matrix in lexicographic order</li>
 * <li>Output the nth column and the index of the row containing the original
 * input string</li>
 * </ul>
 *
 * In practice we are unlikely to actually create an n-by-n matrix for the
 * input data and rely on more clever data structures.
 */
namespace bwt {

    /**
     * The last column of the sorted BWT matrix and the index of the entry in
     * that column that comes from the un-shifted input data string.
     */
    struct BwtResult {
        BwtResult(const std::vector<u8>& data, u32 index): data(data), index(index) {
        }
        const std::vector<u8> data;
        const u32 index;
    };

    inline bool operator==(const BwtResult& first, const BwtResult& second) {
        return first.index == second.index && first.data == second.data;
    }

    /**
     * Performs the Burrows-Wheeler Transform on the given input data.
     * @param input A vector of input data.
     * @return A BwtResult.
     */
    BwtResult encode(const std::vector<u8>& input);

     /**
      * Reverses the Burrows-Wheeler transform to produce the original data.
      * @param bwtResult The output of a Burrows-Wheeler transform.
      * @return The original input data.
      */
    std::vector<u8> decode(const BwtResult& bwtResult);

    inline std:: vector<u8> decode(const std::vector<u8>& data, u32 index) {
        return decode({data, index});
    }
}

#endif //UVZZ_BWT_H
