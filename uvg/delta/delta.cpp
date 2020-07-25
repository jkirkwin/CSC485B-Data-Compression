#include "delta.h"

namespace delta {

    const bitset zeroEncoding(1, 0u);

    bitset encode(long diff) {
        if (diff == 0) {
            return zeroEncoding;
        }
        else {
            unsigned int abs = std::abs(diff);
            unsigned int sign = abs == diff ? 0 : 1;

            unsigned int bits {0};
            while ( (abs >> bits) != 0 ) {
                ++bits;
            }

            // Create the result and add the binary encoding and sign bit
            auto numBits = bits > 0 ? bits - 1 : 0;
            unsigned int data = (abs << 1u) | sign;
            bitset result(numBits + 1, data);

            // Add the unary encoding
            result.push_back(false);
            for (int i = 0; i < bits; ++i) {
                result.push_back(true);
            }

            return result;
        }
    }

    long decode(const bitset& encoded) {
        // Special case: length of 0
        if (encoded.size() == 1) {
            assert(!encoded.test(0));
            return 0;
        }
        else {
            // if there are b bits required for the binary encoding, the length
            // field will have b + 1 bits for the unary representation and b-1
            // for the binary representation for a total of 2b + 1 including the sign bit.
            // Using this lets us skip reading through the unary portion of the length field.
            assert(encoded.size() % 2 == 1);
            const u32 bits = (encoded.size() - 1) / 2;

            // The binary encoding and sign bit will live in the lower b bits.
            auto binMask = (1u << bits) - 1;
            auto lowHalf = encoded.to_ulong() & binMask;
            auto encodedBinaryValue = lowHalf >> 1u;
            auto sign = lowHalf & 1u;

            // Determine if sign is negative or positive
            auto multiplier = sign == 0 ? 1 : -1;

            // Account for the missing/implied MSBit of the binary encoding.
            auto msbit = 1u << (bits - 1);
            long realBinaryValue = (long) encodedBinaryValue | msbit;

            return realBinaryValue * multiplier;
        }
    }

    long decodeFromBitStream(InputBitStream& inputBitStream) {
        unsigned int unaryCount {0};
        while (inputBitStream.read_bit() == 1) {
            ++unaryCount;
        }
        if (unaryCount == 0) {
            return 0;
        }
        else {
            // Find the explicit binary field
            auto explicitBitCount = unaryCount - 1;
            auto explicitBits = inputBitStream.read_bits_msb_first(explicitBitCount);

            // get the sign bit
            auto sign = inputBitStream.read_bit();
            auto multiplier = sign == 0 ? 1 : -1;

            // add the implied MS bit
            auto msbit = 1u << explicitBitCount;
            long realBinaryValue = (long) explicitBits | msbit;

            return realBinaryValue * multiplier;
        }
    }
}
