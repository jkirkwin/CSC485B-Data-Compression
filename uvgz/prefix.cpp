#include "prefix.h"
#include "lzss_backref.h"

// LL codeword lengths for type 1:
// 0-143 -> 8 bits
// 144-255 -> 9 bits
// 256-279 -> 7 bits
// 280-287 -> 8 bits
constexpr std::array<std::array<u32, 3>, 4> fixedLLCodeLengths {{
    {{0, 143, 8}},
    {{144, 255, 9}},
    {{256, 279, 7}},
    {{280, 287, 8}}
}};

/**
 * To be used with constructCanonicalCode()
 * @return The lengths used for the fixed block type 1 LL code.
 */
std::vector<u32> getFixedLLCodeLengths() {
    std::vector<u32> lengths {};

    const auto entryCount = fixedLLCodeLengths.size();
    const auto lastEntry = fixedLLCodeLengths.at(entryCount - 1);
    const auto totalLengths = lastEntry.at(1) + 1;
    lengths.reserve(totalLengths);

    for (auto range : fixedLLCodeLengths) {
        const auto rangeStart {range.at(0)};
        const auto rangeEnd {range.at(1)};
        const auto bits {range.at(2)};
        for(int i = rangeStart; i <= rangeEnd; i++) {
            lengths.push_back(bits);
        }
    }
    return lengths;
}

std::vector<bitset> getFixedLLCode() {
    return constructCanonicalCode(getFixedLLCodeLengths());
}

/**
 * To be used with constructCanonicalCode()
 * @return The lengths used for the fixed block type 1 Distance code.
 */
std::vector<u32> getFixedDistanceCodeLengths() {
    // 32 lengths at 5 bits each
    std::vector<u32> lengths {};
    lengths.reserve(32);
    const u32 bits = 5;

    for(int i = 0; i < 32; i++) {
        lengths.push_back(bits);
    }
    return lengths;
}

std::vector<bitset> getFixedDistanceCode() {
    return constructCanonicalCode(getFixedDistanceCodeLengths());
}

// Minimally adapted from the block2 starter code provided by Bill.
std::vector<bitset> constructCanonicalCode(std::vector<u32> const & lengths){

    unsigned int size = lengths.size();
    std::vector< unsigned int > length_counts(16,0); //Lengths must be less than 16 for DEFLATE
    u32 max_length = 0;
    for(auto i: lengths){
        assert(i <= 15);
        length_counts.at(i)++;
        max_length = std::max(i, max_length);
    }
    length_counts.at(0) = 0; //Disregard any codes with alleged zero length

    std::vector< bitset > result_codes(size);

    //The algorithm below follows the pseudocode in RFC 1951
    std::vector< unsigned int > next_code(size,0);
    {
        //Step 1: Determine the first code for each length
        unsigned int code = 0;
        for(unsigned int i = 1; i <= max_length; i++){
            code = (code+length_counts.at(i-1))<<1;
            next_code.at(i) = code;
        }
    }
    {
        //Step 2: Assign the code for each symbol, with codes of the same length being
        //        consecutive and ordered lexicographically by the symbol to which they are assigned.
        for(unsigned int symbol = 0; symbol < size; symbol++){
            unsigned int length = lengths.at(symbol);
            if (length > 0) {
                result_codes.at(symbol) = bitset(length, next_code.at(length)++);
            }
        }
    }
    return result_codes;
}

std::pair<freq_table_t, freq_table_t> getLzssSymbolFrequencies(const bitset_vec& lzssSymbols) {
    // todo consider using arrays instead

    // 285 entries for LL code
    freq_table_t llFreqs(285);

    // 29 entries for Dist code
    freq_table_t distFreqs(29);

    for(int i = 0; i < lzssSymbols.size(); ++i) {
        const auto symbol = lzssSymbols.at(i);
        if (symbol.size() != LITERAL_BITS) { // backref
            llFreqs.at(symbol.to_ulong())++;

            const auto distSymbol = lzssSymbols.at(i+2);
            distFreqs.at(distSymbol.to_ulong())++;

            assert (lzssSymbols.size() > i+3); // Sanity check
            i+=3; // Skip past the rest of the backreference.
        }
        else { // literal
            llFreqs.at(symbol.to_ulong())++;
        }
    }

    // Since the EOB marker is not in the LZSS input in my implementation, we must manually add it here.
    assert(llFreqs.at(256) == 0);
    llFreqs.at(256)++;

    return {llFreqs, distFreqs};
}

std::vector<u32> frequenciesToLengths(const std::vector<u32>& frequencies) {
    // todo this is where we need to run huffman.
    // we will need to run huffman again later for CL code, so it must be generic.
}

std::pair<std::vector<u32>, std::vector<u32>> getDynamicCodeLengths(const bitset_vec& lzssSymbols) {
//    const auto freqs = getLzssSymbolFrequencies(lzssSymbols);
//    const auto llFreqs = freqs.first;
//    const auto distFreqs = freqs.second;
//
//    const auto llCodeLengths = frequenciesToLengths(llFreqs);
//    const auto distCodeLengths = frequenciesToLengths(distFreqs);

    // todo actually generate dynamic codes



    // Using dummy lengths for now
    std::vector<u32> llCodeLengths;
    for(unsigned int i = 0; i <= 225; i++) {
        llCodeLengths.push_back(8);
    }
    for(unsigned int i = 226; i <= 285; i++) {
        llCodeLengths.push_back(9);
    }

    // Using dummy lengths for now
    std::vector<u32> distCodeLengths;
    for(unsigned int i = 0; i <= 1; i++) {
        distCodeLengths.push_back(4);
    }
    for(unsigned int i = 2; i <= 29; i++) {
        distCodeLengths.push_back(5);
    }

    return {llCodeLengths, distCodeLengths};
}

std::vector<u32> getCLCodeLengths(const std::vector<u32> &llCodeLengths, const std::vector<u32> &distCodeLengths) {
    // todo implement this properly
    return {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5};
}