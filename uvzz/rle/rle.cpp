#include "rle.h"
#include <cassert>

namespace rle {

    void pushLiteral(u8 literal, std::vector<Symbol>& result) {
        result.push_back(Symbol(literal));
    }

    void pushLength(u32 length, std::vector<Symbol>& result) {
        auto symbol = Symbol(symbolFromLength(length));
        result.push_back(symbol);
    }

    void flushRun(u32 runLength, std::vector<Symbol>& result) {
        if (runLength >= CONTINUATION_THRESHOLD) {
            auto continuationLength = runLength - CONTINUATION_THRESHOLD;
            pushLength(continuationLength, result);
        }
    }

    std::vector<Symbol> encode(const std::vector<u8>& rawData) {
        std::vector<Symbol> result {};
        result.reserve(rawData.size());

        u32 currentRun = 0;
        for (const auto& datum : rawData) {
            if (datum == 0) {
                if (currentRun < CONTINUATION_THRESHOLD) {
                    pushLiteral(datum, result);
                }
                ++currentRun;
            }
            else {
                flushRun(currentRun, result);
                currentRun = 0;
                pushLiteral(datum, result);
            }
        }
        flushRun(currentRun, result);

        return result;
    }

    u32 lengthFromSymbol(const bitset& lenSymbol) {
        // Special case: length of 0
        if (lenSymbol.size() == 1) {
            assert(!lenSymbol.test(0));
            return 0;
        }
        else {
            // if there are b bits required for the binary encoding, the length
            // field will have b + 1 bits for the unary representation and b-1
            // for the binary representation for a total of 2b. Using this lets
            // us skip reading through the unary portion of the length field.
            assert(lenSymbol.size() % 2 == 0);
            const u32 bits = lenSymbol.size() / 2;

            // The binary encoding will live in the lower b-1 bits.
            auto binMask = (1u << (bits-1u)) - 1;
            auto encodedBinaryValue = lenSymbol.to_ulong() & binMask;
            auto impliedDelta = binMask + 1; // Account for the missing (implied) MSBit
            return encodedBinaryValue + impliedDelta;
        }
    }

    bitset symbolFromLength(u32 length) {
        // Compute bits needed for binary representation
        u32 bits {0};
        while ( (length >> bits) != 0 ) {
            ++bits;
        }

        // Create the result and add the binary encoding
        auto numBits = bits > 0 ? bits - 1 : 0;
        bitset result(numBits, length);

        // Add the unary encoding
        result.push_back(false);
        for (int i = 0; i < bits; ++i) {
            result.push_back(true);
        }

        return result;
    }
}