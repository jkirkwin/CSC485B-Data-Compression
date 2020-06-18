#ifndef UVGZ_PREFIX_H
#define UVGZ_PREFIX_H

#include <vector>
#include "shared/binary.h"
#include "lzss_backref.h"

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
 * @return Lengths for a CL (Code-Length) code which will encode the Distance and LL
 * codes' codeword lengths.
 */
std::vector<u32> getCLCodeLengths(const std::vector<u32> &llCodeLengths, const std::vector<u32> &distCodeLengths);

// todo - type 2 - CL code specifics - first make sure the first huffman application is working first - use a dummy CL code as done in block2 starter code

// todo - optimization - package-merge implementation

#endif //UVGZ_PREFIX_H
