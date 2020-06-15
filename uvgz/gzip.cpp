#include "gzip.h"
#include "prefix.h"
#include "lzss.h"
#include "lzss_backref.h"

void GzipEncoder::encode(std::istream& inStream) {
    pushHeader();

    // read chunks of data until input is empty.
    const auto chunkSize = kb; // todo reading a kilobyte at a time.
    auto currentBlock = readChunk(inStream, chunkSize);
    auto nextBlock = readChunk(inStream, chunkSize);
    while(! currentBlock.empty()) {
        bool last = nextBlock.empty();
        updateFooterValues(currentBlock);
        sendBlock(0, currentBlock, last);

        currentBlock = nextBlock; // todo use reference types here to speed things up?
        nextBlock = readChunk(inStream, chunkSize);
    }

    pushFooter();
}

GzipEncoder::chunk_t GzipEncoder::readChunk(std::istream& inStream, u32 chunkSize) {
    char nextChar {};
    std::vector<u8> chunk {};

    for (int i = 0; i < chunkSize; i++) {
        if (inStream.get(nextChar)) {
            chunk.push_back(nextChar);
        }
        else {
            break;
        }
    }

    return chunk;
}

/*
 * Update the CRC and ISIZE values based to include the given data.
 */
void GzipEncoder::updateFooterValues(chunk_t &chunk) {
    // CRC requires a void* parameter. Get the pointer to the first cell of the
    // chunk's internal array.
    // See: https://stackoverflow.com/questions/4289612/getting-array-from-stdvector
    u8* chunkArray = &chunk[0];

    if (this->inputSize == 0) {
        // Store initial CRC
        this->crc = CRC::Calculate(chunkArray, chunk.size(), this->crcTable);
    }
    else {
        // Update running CRC
        this->crc = CRC::Calculate(chunkArray, chunk.size(), this->crcTable, this->crc);
    }

    this->inputSize += chunk.size();
}

/**
 * Creats and LzssEncoder that writes output to the given buffer.
 * @param outBuffer The buffer to be written to. It is assumed that this buffer
 * is large enough to accommodate the entire output of the encoder.
 */
LzssEncoder getLzssEncoder(std::vector<bitset>& outBuffer) {
    LzssEncoder::symbol_consumer_t forward = [&outBuffer](const bitset& b) {
        outBuffer.push_back(b);
    };
    return LzssEncoder(forward);
}

/**
 * Creates an LzssEncoder and runs it on the given input data.
 * @return The resulting sequence of LZSS symbols.
 */
std::vector<bitset> runLzss(const std::vector<u8>& input) {
    std::vector<bitset> lzssOutputBuffer {};
    lzssOutputBuffer.reserve(input.size()); // Hopefully the input won't expand. todo consider scaling this up
    auto lzssEncoder = getLzssEncoder(lzssOutputBuffer);

    for(const auto byte : input) {
        lzssEncoder.acceptByte(byte);
    }
    return lzssOutputBuffer;
}

/**
 * Does not output the EOB symbol.
 * @param symbols The LZSS Symbols to be encoded and output
 * @param llCode The length-literal prefix code backreferences
 * @param distanceCode The distance prefix code for backreferences
 */
void GzipEncoder::outputLzssSymbols(const std::vector<bitset>& symbols,
                                    const std::vector<bitset>& llCode,
                                    const std::vector<bitset>& distanceCode) {
    // todo this assumes block type 1

    for(int i = 0; i < symbols.size(); ++i) {
        const auto symbol = symbols[i];
        if (symbol.size() != LITERAL_BITS) {
            // Beginning of a backreference
            assert(false); // we shouldn't be getting here yet.
            // todo this LITERAL_BITS comparison is problematic because 256 is actually considered a literal and would
            //      require 256 bits. If we assume that 256 is not in the input, then it could manually be sent later?
        }
        else {
            // Literal
            auto literalValue = symbol.to_ulong();
            auto encoded = llCode[literalValue];
            pushMsbFirst(encoded);
        }
    }
}

void GzipEncoder::sendBlock(int type, chunk_t &data, bool last) {
    // todo we're creating a new LzssEncoder for each chunk. Should just use one. Make it a member.

    // Block type 1 header fields
    const auto isLast = last ? 1 : 0;
    this->outBitStream.push_bit(isLast);
    this->outBitStream.push_bits(type, 2);

    if (type == 0) {
        // pad to byte boundary
        this->outBitStream.flush_to_byte();

        // len and ~len
        auto len = data.size();
        assert (len <= (1u << 16u)); // todo this is failing.
        this->outBitStream.push_u16(len);
        this->outBitStream.push_u16(~len);

        // content
        for (const auto byte : data) {
            this->outBitStream.push_byte(byte);
        }
    }
    else if(type == 1) {    // LZSS
        const auto llCode = getFixedLLCode();
        const auto distanceCode = getFixedDistanceCode();
        const auto lzssOutput = runLzss(data);
        outputLzssSymbols(lzssOutput, llCode, distanceCode);

        // Insert the EOB marker. This is not sent through the LZSS scheme but must be encoded as if it were.
        const auto eob = 256u;
        const auto encodedEob = llCode.at(eob);
        pushMsbFirst(encodedEob);
        // todo confirm that LZSS should not account for the EOB in its backreference calculations.
        //      Otherwise we will need to refactor.
    }
    else {
        assert(false);
        // todo block type 2 unimplemented
    }
}

/*
 * Write a generic GZip header.
 */
void GzipEncoder::pushHeader() {
    this->outBitStream.push_bytes(
            0x1f, 0x8b, //Magic Number
            0x08, //Compression (0x08 = DEFLATE)
            0x00, //Flags
            0x00, 0x00, 0x00, 0x00, //MTIME (little endian)
            0x00, //Extra flags
            0x03 //OS (Linux)
    );
}

/*
 * Output the running CRC and ISIZE values and clear them.
 */
void GzipEncoder::pushFooter() {
    this->outBitStream.flush_to_byte();
    this->outBitStream.push_u32(this->crc);
    this->outBitStream.push_u32(this->inputSize);
    this->inputSize = 0;
}

void GzipEncoder::pushMsbFirst(const bitset &bits) {
    for(unsigned long i = bits.size() - 1; i > 0; --i) {
        this->outBitStream.push_bit(bits[i]);
    }
}

