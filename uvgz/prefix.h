#ifndef UVGZ_PREFIX_H
#define UVGZ_PREFIX_H

#include <vector>
#include "shared/binary.h"
#include "lzss_backref.h"

using bitset_vec = std::vector<bitset>;

namespace package_merge {
    /**
     * Get a set of code lengths for an optimal prefix code for the given
     * weights to the maximum length constraint. Weights of 0 will be treated
     * as if they were infinite and no code word length will be generated for
     * any such entry.
     *
     * @param weights A vector W of weights to use.
     * @param max The maximum allowed codeword length.
     * @return A vector L of lengths such that L[i] is the length of the
     * codeword with weight W[i]. L[i] <= limit for all i.
     */
    std::vector<u32> getCodeLengths(std::vector<u32> weights, u32 limit);
}

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


/**
 * @param lzssSymbols The LZSS symbols to use to create the codes.
 * @return A pair of vectors of code lengths. The first contains lengths for
 * the LL code and the second contains lengths for the distance code.
 */
std::pair<std::vector<u32>, std::vector<u32>> getDynamicCodeLengths(const bitset_vec& lzssSymbols);

typedef std::vector<u32> freq_table_t;
/**
 * This can be used to determine how applicable type 2 might be.
 * @param lzssSymbols The symbols to compute the frequencies over
 * @return A pair of frequency tables for the LL and Distance symbols.
 */
std::pair<freq_table_t, freq_table_t> getLzssSymbolFrequencies(const bitset_vec& lzssSymbols);

/**
 * @param llCodeLengths The lengths of the literals/lengths codewords being used.
 * @param distCodeLengths The lengths of the distance codewords being used.
 * @return A vector of CL symbols encoding the LL and Distance code lengths.
 */
std::vector<bitset> getCLSymbols(const std::vector<u32> &llCodeLengths, const std::vector<u32> &distCodeLengths);


/**
 * Generates an optimal set of lengths for the prefix code to encode the CL symbols.
 * @param clSymbols The CL symbols to be encoded with the prefix code.
 * @return Lengths for a CL (Code-Length) code which will encode the Distance and LL
 * codes' codeword lengths.
 */
std::vector<u32> getCLCodeLengths(const std::vector<bitset> &clSymbols);

#endif //UVGZ_PREFIX_H
