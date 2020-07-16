#include "rle.h"
#include <cassert>
#include "binary.h"

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

    std::vector<u8> decode(const std::vector<Symbol>& encodedData) {
        std::vector<u8> result {};
        for (const auto& symbol : encodedData) {
            if (symbol.isLiteral()) {
                result.push_back(symbol.getLiteral());
            }
            else {
                assert (symbol.isLength());
                auto len = lengthFromSymbol(symbol);
                for (int i = 0; i < len; ++i) {
                    result.push_back(0);
                }
            }
        }
        return result;
    }

    u32 lengthFromSymbol(const Symbol& lenSymbol) {
        assert (lenSymbol.isLength());
        return lengthFromSymbol(lenSymbol.getLength());
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

    u32 readLengthFromBitstream(InputBitStream& inStream) {
        auto unaryCount {0};
        while (inStream.read_bit() == 1) {
            ++unaryCount;
        }
        if (unaryCount == 0) {
            return 0;
        }
        else {
            // Find and push the explicit binary field
            auto explicitBitCount = unaryCount - 1;
            auto explicitBits = inStream.read_bits_msb_first(explicitBitCount);
            bitset lengthField(explicitBitCount, explicitBits);

            // add the unary encoding
            lengthField.push_back(false);
            for (int i = 0; i < unaryCount; ++i) {
                lengthField.push_back(true);
            }

            return lengthFromSymbol(lengthField);
        }
    }
}

namespace rle::vb {

    void pushLength(u32 length, byte_vec& resultBuffer) {
        auto encodedLen = binary::vb::encode(length);
        for (auto b : encodedLen) {
            resultBuffer.push_back(b);
        }
    }

    byte_vec getLengthBytes(const byte_vec& buffer, int startIndex) {
        byte_vec result{};
        auto flag = binary::vb::FLAG;
        for (int i = startIndex; i < buffer.size(); ++i) {
            auto nextByte = buffer.at(i);
            result.push_back(nextByte);
            if ( (nextByte & flag) == 0) {
                break; // Last byte int the length field was found.
            }
        }
        return result;
    }

    byte_vec encode(const byte_vec& input, u32 repeats) {
        auto currentRun = 0;
        u16 prevByte = -1; // begin with a value that cannot be present in the input
        byte_vec result {};
        for (const auto byte : input) {
            if (byte == prevByte) {
                if (currentRun < repeats) {
                    result.push_back(byte);
                }
                ++currentRun;
            }
            else {
                if (currentRun >= repeats) {
                    auto len = currentRun - repeats;
                    vb::pushLength(len, result);
                }
                result.push_back(byte);
                prevByte = byte;
                currentRun = 1;
            }
        }
        if (currentRun >= repeats) {
            vb::pushLength(currentRun - repeats, result);
        }
        return result;
    }

    byte_vec decode(const byte_vec& encoded, u32 repeats) {
        assert (repeats > 1);

        u32 currentRun {0};
        u16 prev = -1; // Use a value than cannot occur in the input vector
        byte_vec result {};

        for (u32 i = 0; i < encoded.size(); ++i) {
            if (currentRun == repeats) {
                // Expect a length encoding here
                auto lengthField = getLengthBytes(encoded, i);
                assert (! lengthField.empty());
                i += lengthField.size() - 1; // -1 because I will be incremented in loop test above

                u32 continuation = binary::vb::decode(lengthField);
                for (int j = 0; j < continuation; ++j) {
                    result.push_back(prev);
                }
                currentRun = 0;
            }
            else {
                auto nextByte = encoded.at(i);
                if (nextByte == prev) {
                    ++currentRun;
                }
                else {
                    prev = nextByte;
                    currentRun = 1;
                }
                result.push_back(nextByte);
            }
        }
        return result;
    }
}