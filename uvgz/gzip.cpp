#include "gzip.h"
#include "prefix.h"
#include "lzss.h"
#include "lzss_backref.h"

void GzipEncoder::encode(std::istream& inStream) {
    pushHeader();
    processInput(inStream);
    pushFooter();
}

void GzipEncoder::processInput(std::istream& inStream) {
    // Repeatedly read an arbitrarily sized chunk of input data and run it
    // through LZSS. Determine the block type to use based on the result.

    const auto inputChunkSize = kb; // todo only reading a kilobyte at a time for now

    // Create Lzss output buffer and encoder
    std::vector<bitset> lzssOutputBuffer;
    const auto buffSize = inputChunkSize * 2; // Ensure we have enough room in case of expansion.
    lzssOutputBuffer.reserve(buffSize);
    LzssEncoder::symbol_consumer_t symbolConsumer = [&](const bitset& symbol) {
        lzssOutputBuffer.push_back(symbol);
    };
    LzssEncoder lzssEncoder(symbolConsumer);

    // Start reading chunks
    auto currentChunk = readChunk(inStream, inputChunkSize);
    auto nextChunk = readChunk(inStream, inputChunkSize);
    while(! currentChunk.empty()) {
        // Run the chunk through LZSS
        lzssOutputBuffer.clear();
        lzssEncoder.acceptData(currentChunk);

        // Send the data
        bool last = nextChunk.empty();
        updateFooterValues(currentChunk);
        if (last) {
            lzssEncoder.flush();
        }
        sendBlocks(currentChunk, lzssOutputBuffer, last);

        // Update markers
        currentChunk = nextChunk; // todo use reference types here to speed things up?
        nextChunk = readChunk(inStream, inputChunkSize);
    }
}

void GzipEncoder::sendBlocks(std::vector<u8> &rawData, std::vector<bitset> &lzssData, bool endOfData) {
    // todo choose block types based on lzss results
    //    sendBlockType0(rawData, endOfData);
    sendBlockType1(lzssData, endOfData);
}

void GzipEncoder::sendBlockType0(std::vector<u8> &data, bool last) {
    assert(data.size() < MAX_TYPE_0_SIZE);

    // header fields
    const auto isLast = last ? 1 : 0;
    this->outBitStream.push_bit(isLast);
    const auto type = 0;
    this->outBitStream.push_bits(type, 2);

    // pad to byte boundary
    this->outBitStream.flush_to_byte();

    // len and ~len
    auto len = data.size();
    assert (len < MAX_TYPE_0_SIZE);
    this->outBitStream.push_u16(len);
    this->outBitStream.push_u16(~len);

    // content
    for (const auto byte : data) {
        this->outBitStream.push_byte(byte);
    }
}

void GzipEncoder::sendBlockType1(std::vector<bitset> &lzssData, bool last) {
    // header fields
    const auto isLast = last ? 1 : 0;
    this->outBitStream.push_bit(isLast);
    const auto type = 1;
    this->outBitStream.push_bits(type, 2);

    // content
    const auto llCode = getFixedLLCode();
    const auto distCode = getFixedDistanceCode();
    for (int i = 0; i < lzssData.size(); ++i) {
        const auto symbol = lzssData.at(i);
        if (symbol.size() == LITERAL_BITS) {  // Literal
            const auto codeWord = llCode.at(symbol.to_ulong());
            pushMsbFirst(codeWord);
        }
        else {  // Backreference
            const auto &lenBaseSymbol = symbol; // Reference to avoid copy
            const auto lenOffsetSymbol = lzssData.at(++i);
            const auto distBaseSymbol = lzssData.at(++i);
            const auto distOffsetSymbol = lzssData.at(++i);

            const auto lenCodeWord = llCode.at(lenBaseSymbol.to_ulong());
            const auto distCodeWord = distCode.at(distBaseSymbol.to_ulong());

            // Note that rule #1 applies to the offsets but not the prefix code words.
            pushMsbFirst(lenCodeWord);
            this->outBitStream.push_bits(lenOffsetSymbol);
            pushMsbFirst(distCodeWord);
            this->outBitStream.push_bits(distOffsetSymbol);
        }
    }

    // Insert the EOB marker. This is not sent through the LZSS scheme but must
    // be encoded as if it were.
    const auto eob = 256u;
    const auto encodedEob = llCode.at(eob);
    pushMsbFirst(encodedEob);
}

/*
 * Reads from the given stream up to the given amount of data.
 */
GzipEncoder::chunk_t GzipEncoder::readChunk(std::istream& inStream, u32 chunkSize) {
    char nextChar {};
    std::vector<u8> chunk;
    chunk.reserve(chunkSize);

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
    u8* chunkArray = &chunk.at(0);

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
    const auto numBits = bits.size();
    for(unsigned long i = 0; i < numBits; ++i) {
        this->outBitStream.push_bit(bits[numBits - i - 1]);
    }
}
