#include "bwt.h"
#include "binary.h"
#include <functional>

namespace bwt {

    // Unique sentinel value required to build the suffix tree.
    const std::int16_t TERMINATOR = -1;

    /*
     * Transform the given byte sequence into a sequence of variants terminated
     * with a non-u8 variant instance. This allows us to add in a
     */
    std::vector<std::int16_t> getTerminatedData(const std::vector<u8>& rawData) {
        std::vector<int16_t> result;
        result.reserve(rawData.size() + 1);
        for (u8 datum : rawData) {
            result.push_back(datum);
        }
        result.push_back(TERMINATOR);
        return result;
    }

    /*
     * Effectively as <= operation for the byte strings pointed ot by the given
     * indexes.
     *
     * Return true if the byte sequence pointed to by first is
     * lexicographically less than the one pointed to by second.
     *
     * If the strings are identical, then returns true iff first < second to
     * preserve order.
     */
    bool compareIndexes(const std::vector<u8>& data, u32 first, u32 second) {
        for (int i = 0; i < data.size(); ++i) {
            auto firstIndex = (first + i) % data.size();
            auto secondIndex = (second + i) % data.size();
            auto firstValue = data.at(firstIndex);
            auto secondValue = data.at(secondIndex);
            if (firstValue != secondValue) {
                return firstValue < secondValue;
            }
        }
        // The suffixes are strictly equal but to preserve order we
        // compare the indexes instead of using std::stable_sort
        // because it is of a higher complexity class than
        // std::sort.
        assert (first != second);
        return first < second;
    }

    /*
     * Sort the index vector in place based on the lexicographic ordering of
     * the strings (from data) represented by the indexes.
     */
    void sortIndexes(const std::vector<u8>& data, std::vector<u32>& indexes) {
        using namespace std::placeholders;
        auto compare = std::bind(compareIndexes, data, _1, _2);
        std::sort(indexes.begin(), indexes.end(), compare);
    }

    /*
     * Generate a vector {0, 1, ... , n-1, n}
     */
    std::vector<u32> getIndexes(u32 n) {
        std::vector<u32> indexes(n);
        for (int i = 0; i < n; ++i) {
            indexes.at(i) = i;
        }
        return indexes;
    }

    /*
     * Returns a vector of indexes into the input sequence, ordered
     * lexicographically using the string pointed to by each one.
     */
    std::vector<u32> getSortedIndexes(const std::vector<u8>& input) {
        auto indexes = getIndexes(input.size());
        sortIndexes(input, indexes);
        return indexes;
    }

    /*
     * Produce a BwtResult from the sorted indexes.
     */
    BwtResult getResultFromIndexes(const std::vector<u32>& indexes, const std::vector<u8>& data) {
        assert (data.size() == indexes.size());

        std::vector<u8> resultVector {};
        resultVector.reserve(data.size());
        u32 originalRow {0};

        const auto inputSymbols = indexes.size();
        for (int i = 0; i < inputSymbols; ++i) {
            // Index is the beginning of the suffix.
            auto index = indexes.at(i);
            if (index == 0) {
              originalRow = i; // Record which row contains the input sequence.
            }

            // Push the last character of each suffix in sorted order.
            auto lastIndex = (index + inputSymbols - 1) % indexes.size();
            auto last = data.at(lastIndex);
            resultVector.push_back(last);
        }

        return {resultVector, originalRow};
    }

    BwtResult encode(const std::vector<u8>& input) {
        // todo use Ukkonen or McCright here instead.

        // Naive impl for now
        const auto sortedIndexes = getSortedIndexes(input);
        return getResultFromIndexes(sortedIndexes, input);
    }

    std::vector<u8> decode(const BwtResult& bwtResult) {
        return {}; // todo stub
    }
}

