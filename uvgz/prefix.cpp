#include "prefix.h"
#include "lzss_backref.h"
#include <queue>
#include <utility>
#include <set>

namespace package_merge {

    /**
     * An entry in a package has a weight and a set of keys which allow us to
     * map the computed code lengths back to the original input items.
     */
    struct Entry {
        Entry(u32 key, u32 weight): keys({key}), weight(weight) {
        }

        Entry(std::vector<u32>  keys, u32 weight): keys(std::move(keys)), weight(weight) {
        }

        Entry(Entry& a, Entry& b) {
            keys.insert(keys.begin(), a.keys.begin(), a.keys.end());
            keys.insert(keys.begin(), b.keys.begin(), b.keys.end());
            weight = a.weight + b.weight;
        }

        std::vector<u32> keys;
        u32 weight;
    };

    /**
     * Used to order entries within a package.
     */
    struct EntryComparator {
        bool operator()(const Entry& a, const Entry& b) const {
            if (a.weight == b.weight) {
                // This is needed to allow distinct entries with the same
                // weight to be inserted into a set.
                return a.keys < b.keys;
            }
            else {
                return a.weight < b.weight;
            }
        }
    };

    typedef std::set<Entry, EntryComparator> pkg;

    /*
     * Get a package containing pairs of consecutive elements in the given
     * package. If there are an odd number of elements in the original package
     * then the largest element will not belong to any of the pairs returned.
     */
    pkg getAdjacentPairs(const pkg& package) {
        pkg result;
        auto it = package.begin();
        while (it != package.end()) {
            auto first = *it;
            ++it;
            if (it != package.end()) { // There is a pair remaining
                auto second = *it;
                ++it;

                Entry pair(first, second);
                result.insert(pair);
            }
        }
        return result;
    }

    /**
     * @return True iff the given initial package can be encoded subject to the given limit.
     */
    bool isPossibleLimit(const pkg& p, u32 limit) {
        // The most inputs we can encode with the limit L is 2^Limit
        return 1u << limit;
    }

    std::vector<u32> getCodeLengths(std::vector<u32> weights, u32 limit) {
        // First, eliminate all weights of 0 to create the initial package.
        pkg initial;
        for (int i = 0; i < weights.size(); ++i) {
            if (weights.at(i) > 0) {
                initial.insert(Entry(i, weights.at(i)));
            }
        }
        assert (isPossibleLimit(initial, limit));

        std::vector<u32> codeLengths(weights.size(), 0);
        if (initial.empty()) {
            // Quit early. No non-zero weights are given, so there is no
            // information content and there is no non-zero length.
            return codeLengths;
        }
        else if(initial.size() == 1) {
            // Quit early. There is only a single non-zero weight.
            auto entry = *(initial.begin());
            auto keys = entry.keys;
            assert (keys.size() == 1);
            auto key = keys.front();
            codeLengths[key]++;
            return codeLengths;
        }
        else {
            // Perform the algorithm as described in the Sayood book, under the guarantee
            // that we have a non-trivial input.

            // For L-1 iterations, compute the pairs from the current package and
            // merge them with the initial set of nodes.
            pkg package = initial;
            for (int i = 0; i < limit-1; ++i) {
                package = getAdjacentPairs(package);
                pkg initialCopy = initial; // Without this, the merge operation destroys our initial package.
                package.merge(initialCopy);
            }

            // The first 2m-2 items in the final package are used to compute the lengths
            auto entriesUsed = 2 * initial.size() - 2;
            assert (entriesUsed <= package.size());

            // Compute code lengths from the final package. The keys stored in each entry
            // give us the position of the entries in the input (and output) array(s).
            auto it = package.begin();
            for (int i = 0; i < entriesUsed; ++i) {
                for (auto key : it->keys) {
                    codeLengths.at(key)++;
                }
                ++it;
            }
        }

        // todo check that the KM inequality is met
        return codeLengths;
    }
}

// todo remove this and just use a loop with fixed numbers
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

std::pair<std::vector<u32>, std::vector<u32>> getDynamicCodeLengths(const bitset_vec& lzssSymbols) {
    // Get the frequencies of LZSS symbols
    const auto freqs = getLzssSymbolFrequencies(lzssSymbols);
    const auto llFreqs = freqs.first;
    const auto distFreqs = freqs.second;

    // Run package merge algorithm to get optimal code length tables.
    const auto llCodeLengths = package_merge::getCodeLengths(llFreqs, 15);
    const auto distCodeLengths = package_merge::getCodeLengths(distFreqs, 15);

    return {llCodeLengths, distCodeLengths};
}

/*
 * Append CL symbols for the given run of the same length.
 */
void appendCLRun(u32 len, u32 times, std::vector<bitset>& result) {
    const u32 symbolBits {4};
    const u32 rleBits {5};
    const bitset literal(symbolBits, len);

    if (times < 4) {
        // We must use explicit literals
        for (int i = 0; i < times; ++i) {
            result.push_back(literal);
        }
    }
    else if (len != 0) {  // Use symbol 16 for runs
        bitset rleSymbol(rleBits, 16);
        u32 baseCount = 3;
        u32 offsetBits = 2;
        u32 offsetMax = 3;

        // Push one literal first
        result.push_back(literal);
        times--;

        while (times >= baseCount) {
            // Push the RLE symbol
            result.push_back(rleSymbol);
            times -= baseCount;

            // Push the offset field
            u8 delta = std::min(times, offsetMax);
            bitset offset(offsetBits, delta);
            result.push_back(offset);
            times -= delta;
        }

        // Push any remaining literals
        while (times > 0) {
            result.push_back(literal);
            --times;
        }
    }
    else { // Encode a run of zeros
        // We choose between symbols 18 and 17 for RLE depending on run length.
        while (times > 0) {
            if (times >= 11) {
                bitset rleSymbol(rleBits, 18);
                u32 baseCount = 11;
                u32 offsetBits = 7;
                u32 offsetMax = 127;

                result.push_back(rleSymbol);
                times -= baseCount;

                u8 delta = std::min(times, offsetMax);
                bitset offset(offsetBits, delta);
                result.push_back(offset);
                times -= delta;
            }
            else if (times >= 3) {
                bitset rleSymbol(rleBits, 17);
                u32 baseCount = 3;
                u32 offsetBits = 3;
                u32 offsetMax = 7;

                result.push_back(rleSymbol);
                times -= baseCount;

                u8 delta = std::min(times, offsetMax);
                bitset offset(offsetBits, delta);
                result.push_back(offset);
                times -= delta;
            }
            else {
                result.push_back(literal);
            }
        }
    }
}

/*
 * Appends the CL symbols for the given lengths to the result vector.
 */
void appendCLSymbols(const std::vector<u32> &lengths, std::vector<bitset>& result) {
    // For each length, determine the appropriate symbol and check if we can use RLE.
    u32 prev = lengths.at(0); // the last length found
    u32 count = 1; // the number of times we've seen prev
    for (int i = 1; i < lengths.size(); ++i) {
        auto len = lengths.at(i);
        if (len == prev) { // Run continues
            ++count;
        }
        else { // End of run
            // Output count copies of prev and update locals
            appendCLRun(prev, count, result);
            prev = len;
            count = 1;
        }
    }
    appendCLRun(prev, count, result);
}

std::vector<bitset> getCLSymbols(const std::vector<u32> &llCodeLengths, const std::vector<u32> &distCodeLengths) {
    // Runs overlapping the two length tables will not be encoded for the sake of simplicity
    std::vector<bitset> result;
    if (!llCodeLengths.empty()) {
        appendCLSymbols(llCodeLengths, result);
    }
    if (!distCodeLengths.empty()) {
        appendCLSymbols(distCodeLengths, result);
    }
    return result;
}

std::vector<u32> getCLFrequencies(const std::vector<bitset> &clSymbols) {
    std::vector<u32> freqs(19, 0);
    for (const auto& symbol : clSymbols) {
        auto symbolBits = symbol.size();

        // A field of any other width must be one of the offset fields following an RLE symbol.
        bool isLiteralOrRle = symbolBits == 4 || symbolBits == 5;
        if (isLiteralOrRle) {
            freqs.at(symbol.to_ulong())++;
        }
    }
    return freqs;
}

std::vector<u32> getCLCodeLengths(const std::vector<bitset> &clSymbols) {
    const int maxBits = 7;
    const auto freqs = getCLFrequencies(clSymbols);
    return package_merge::getCodeLengths(freqs, maxBits);
}
