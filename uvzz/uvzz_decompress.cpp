#include <input_stream.h>
#include <fstream>
#include "bwt/bwt.h"
#include "mtf/mtf.h"
#include "rle/rle.h"
#include "constants.h"

struct BadFileTypeException : std::exception {
};

struct UnexpectedEndOfInput : std::exception {
};

std::vector<u8> decodeRleBlock(InputBitStream& inStream) {
    std::vector<u8> block{};
    block.reserve(BLOCK_SIZE);

    u32 currentRun{0};
    while (block.size() < BLOCK_SIZE) {
        if (currentRun == rle::CONTINUATION_THRESHOLD) {
            // A length field comes next
            auto length = rle::readLengthFromBitstream(inStream);
            assert (! inStream.input_depleted()); // Runs should never cross block boundaries
            for (int i = 0; i < length; ++i) {
                block.push_back(0);
            }
            currentRun = 0;
        }
        else {
            u8 literal = inStream.read_byte();
            if (inStream.input_depleted()) {
                // We've run over the end of the input.
                break;
            }
            else if (literal == 0) {
                ++currentRun;
            }
            else {
                currentRun = 0;
            }
            block.push_back(literal);
        }
    }
    return block;
}

u8 getByteAndThrowIfDepleted(InputBitStream& inputBitStream) {
    auto b = inputBitStream.read_byte();
    if (inputBitStream.input_depleted()) {
        throw UnexpectedEndOfInput {};
    }
    return b;
}

u32 decodeVbFromInStream(InputBitStream& inputBitStream) {
    using binary::vb::FLAG;
    std::vector<u8> encoding {};
    auto nextByte = getByteAndThrowIfDepleted(inputBitStream);
    while ( (nextByte & FLAG) != 0) {
        encoding.push_back(nextByte);
        nextByte = getByteAndThrowIfDepleted(inputBitStream);
    }
    encoding.push_back(nextByte);
    return binary::vb::decode(encoding);
}

/*
 * Read the variable byte encoding from the input stream and decode it.
 */
u32 getBwtIndex(InputBitStream& inputBitStream) {
    return decodeVbFromInStream(inputBitStream);
}

/*
 * Read and decode the variable byte encoding of the size, add it to the index.
 */
u32 getBwtSize(InputBitStream& inputBitStream, u32 bwtIndex) {
    auto encodedSize = decodeVbFromInStream(inputBitStream);
    return encodedSize + bwtIndex;
}

std::vector<u8> getRawDataBlock(InputBitStream& inputBitStream) {
    if (inputBitStream.input_depleted()) {
        return {};
    }

    // Get the bwt metadata
    auto bwtIndex = getBwtIndex(inputBitStream);
    auto blockSize = getBwtSize(inputBitStream, bwtIndex);

    // Pull out the encoded block
    std::vector<u8> block {};
    block.reserve(blockSize);
    while (block.size() < blockSize) {
        auto nextByte = inputBitStream.read_byte();
        block.push_back(nextByte);

        assert (! inputBitStream.input_depleted());
    }

    // todo add in the rest of the pipeline here.

    // Invert the MTF transform
    const auto mtfDecoded = mtf::invert(block);

    // Invert the BWT
    const auto bwtDecoded = bwt::decode(mtfDecoded, bwtIndex);

    // Invert RLE1 and return the decoded block
    auto rleDecoded = rle::vb::decode(bwtDecoded);
    return rleDecoded;
}

void printRawData(const std::vector<u8>& rawData) {
    for (const auto c : rawData) {
        std::cout << c;
    }
}

bool verifyFileType(InputBitStream& inputBitStream) {
    return inputBitStream.read_u16() == FILE_TYPE_MAGIC_NUMBER;
}

int main() {
    std::cerr << "Decoding (RLE1 & BWT only)" << std::endl;

    // todo remove this once debugging is over
//    const auto filename = "/home/jamie/csc485/CSC485B-Data-Compression/uvzz/harbour.zz";
//    std::fstream f(filename);
//    InputBitStream inputBitStream(f);
    InputBitStream inputBitStream;

    if (!verifyFileType(inputBitStream)) {
        throw BadFileTypeException();
    }

    while (!inputBitStream.input_depleted()) {
        try {
            const auto rawData = getRawDataBlock(inputBitStream);
            printRawData(rawData);
        }
        catch (UnexpectedEndOfInput& e) {
            break;
        }
    }
}