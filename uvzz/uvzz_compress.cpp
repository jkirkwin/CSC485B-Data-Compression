#include "rle/rle.h"
#include "bwt/bwt.h"
#include "mtf/mtf.h"
#include "constants.h"
#include <iostream>
#include <output_stream.h>

std::vector<u8> readBlock(u32 size = BLOCK_SIZE, std::istream& inStream = std::cin) {
    std::vector<u8> block;
    block.reserve(size);

    char nextChar {};
    for (int i = 0; i < size; i++) {
        if (inStream.get(nextChar)) {
            block.push_back(nextChar);
        }
        else {
            break;
        }
    }
    return block;
}

void writeVbEncoding(OutputBitStream& out, u32 n) {
    const auto encoded = binary::vb::encode(n);
    assert (! encoded.empty());
    out.push_bytes(encoded);
}

/*
 * Send the variable-byte/delta encoding of the bwt index and block size to the
 * output stream.
 */
void writeBwtOverhead(OutputBitStream& out, const bwt::BwtResult& bwtResult) {
    // Encode and write the bwt index
    writeVbEncoding(out, bwtResult.index);

    // Encode and write the bwt block size (minus the bwt index)
    const auto size = bwtResult.data.size();
    const auto sizeDelta = size - bwtResult.index;
    writeVbEncoding(out, sizeDelta);
}

void pushRle2Result(const std::vector<rle::Symbol>& rle2Result, OutputBitStream& outputBitStream) {
    for (const auto& symbol : rle2Result) {
        if (symbol.isLiteral()) {
            outputBitStream.push_byte(symbol.getLiteral());
        }
        else {
            outputBitStream.push_bits_msb_first(symbol.getLength());
        }
    }
}


void encode(const std::vector<u8>& block, OutputBitStream& outputBitStream) {
    // Run RLE1 and push the size of the resulting block
    const auto vbRleResult = rle::vb::encode(block);

    // Run BWT and push the index and block size to be used
    const auto bwtResult = bwt::encode(vbRleResult);
    writeBwtOverhead(outputBitStream, bwtResult);

    // Run MTF on the BWT block
    const auto mtfResult = mtf::transform(bwtResult.data);

    // Run specialized RLE2
    auto rle2Result = rle::encode(mtfResult);
    pushRle2Result(rle2Result, outputBitStream);

    // todo add the rest of the pipeline here.
}

void sendMagicNumber(OutputBitStream& outputBitStream) {
    outputBitStream.push_u16(FILE_TYPE_MAGIC_NUMBER);
}

int main() {
    OutputBitStream outputBitStream;

    sendMagicNumber(outputBitStream);

    std::vector<u8> block = readBlock();
    while (! block.empty()) {
        encode(block, outputBitStream);
        block = readBlock();
    }

    outputBitStream.flush_to_byte();
    return 0;
}