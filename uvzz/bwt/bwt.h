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
     * @tparam V The type of data that was encoded.
     */
    template <class V>
    struct BwtResult {
        const std::vector<V>& data;
        const u32 index;
    };

    /**
     * Performs the Burrows-Wheeler Transform on the given input data.
     * @tparam V The type of input data
     * @param input A vector of input data.
     * @return A BwtResult.
     */
    template <class V>
    BwtResult<V> encode(const std::vector<V>& input) {
        // todo stub
        return {input, 0};
    }

     /**
      * Reverses the Burrows-Wheeler transform to produce the original data.
      * @tparam V The type of data that was transformed
      * @param bwtResult The output of a Burrows-Wheeler transform.
      * @return The original input data.
      */
    template <class V>
    std::vector<V> decode(const BwtResult<V>& bwtResult) {
        // todo stub
        return bwtResult.data;
    }
}

#endif //UVZZ_BWT_H
