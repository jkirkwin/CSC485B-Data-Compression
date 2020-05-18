#include <unordered_map>
#include "lzw_encoder.h"
#include <limits.h>
#include <cassert>
#include <functional>

LzwEncoder::LzwEncoder(consumer_t& acceptBinaryData) {
    this->acceptBinaryData = acceptBinaryData;

    // Insert all single-character values into the symbol table
    int i = 0;
    for(i = 0; i <= UCHAR_MAX; i++) {
        auto s = std::string(1, i);
        std::pair<std::string, int> entry (s, i);
        symbolTable.insert(entry);
    }

    // TODO Verify:
    // I don't think I need to insert an entry for the reset marker
    // since we're not using a contiguous structure like an array.

    // Account for reset marker at index 0x100
    nextIndex = UCHAR_MAX + 2; 
}

void LzwEncoder::acceptChar(char nextChar) {

    int maxIndex = pow(2, MAX_INDEX_BITS); // TODO Move this so it isn't recomputed on each call.
    std::string augmented = workingStr + nextChar;

    if (symbolTable.find(augmented) != symbolTable.end()) {
        workingStr = augmented;
    } 
    else if (nextIndex > maxIndex) {
        // todo duplicated below
        auto index = symbolTable[workingStr];
        workingStr = std::string(1, nextChar);
        acceptBinaryData(BinaryField(index, numBits));
    }
    else {
        std::pair<std::string, int> augmentedEntry (augmented, nextIndex++);
        symbolTable.insert(augmentedEntry);

        // todo duplicated above
        auto index = symbolTable[workingStr];
        workingStr = std::string(1, nextChar);
        acceptBinaryData(BinaryField(index, numBits));

        if (nextIndex > pow(2, numBits)) {
            numBits++;
        }
    }
}

void LzwEncoder::flush() {
    if (!workingStr.empty()) {
        // todo partially triplicated in accept()
        auto index = symbolTable[workingStr];
        acceptBinaryData(BinaryField(index, numBits));
    }
}