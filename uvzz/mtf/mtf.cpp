#include "mtf.h"
#include <vector>
#include <algorithm>
#include <list>
#include <cassert>
#include <boost/circular_buffer.hpp>

namespace mtf {

    // An array-based structure is used here in the hopes that the table will
    // be more likely to fit in cache than if a list was used.
    typedef boost::circular_buffer<u8> index_table_t;

    index_table_t getInitialIndexTable() {
        index_table_t table(256);
        for(int i = 0; i < 256; ++i) {
            table.push_back((u8)i);
        }
        return table;
    }

    std::vector<u8> transform(const std::vector<u8>& input) {
        std::vector<u8> result;
        result.reserve(input.size());

        auto table = getInitialIndexTable();
        assert (table.size() == 256);

        for (u8 inByte : input) {
            // Find the index in the table of the next symbol
            // Inspired by https://stackoverflow.com/questions/3909784/how-do-i-find-a-particular-value-in-an-array-and-return-its-index
            auto item = std::find(table.begin(), table.end(), inByte);
            auto index = std::distance(table.begin(), item);

            // Move the index to the front of the table.
            table.erase(item);
            table.push_front(inByte);

            // Add the index to the result.
            result.push_back(index);
        }
        return result;
    }

    std::vector<u8> invert(const std::vector<u8>& transformedData) {
        std::vector<u8> result;
        result.reserve(transformedData.size());

        auto table = getInitialIndexTable();
        assert (table.size() == 256);

        for (u8 index : transformedData) {
            // Get the symbol at index and add it to the result
            auto symbol = table.at(index);
            result.push_back(symbol);

            // Move to front
            table.erase(table.begin() + index);
            table.push_front(symbol);
        }
        return result;
    }
}