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

    const auto inputChunkSize = 20*kb;

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
        lzssEncoder.acceptData(currentChunk);

        // Send the data
        bool last = nextChunk.empty();
        updateFooterValues(currentChunk);
        lzssEncoder.flush();
        sendBlocks(currentChunk, lzssOutputBuffer, last);
        lzssOutputBuffer.clear();

        // Update markers
        currentChunk = nextChunk; // todo use references here to decrease memory
        nextChunk = readChunk(inStream, inputChunkSize);
    }
}

void GzipEncoder::sendBlocks(std::vector<u8> &rawData, std::vector<bitset> &lzssData, bool endOfData) {
    const int minThreshold {50};
    const int type2Threshold {500};
    const double maxToleratedExpansion {1.2}; // Determined by benchmarking.
    if (rawData.size() <= minThreshold) {
        // Type one has the least overhead so we use it for the shortest blocks.
        sendBlockType1(lzssData, endOfData);
    }
    else if (lzssData.size() >= rawData.size() * maxToleratedExpansion) {
        // Type 0 is used if we don't have many back references
        sendBlockType0(rawData, endOfData);
    }
    else if (rawData.size() <= type2Threshold) {
        // Type 2 will usually be best for longer inputs.
        sendBlockType1(lzssData, endOfData);
    }
    else {
        sendBlockType2(lzssData, endOfData);
    }
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
    sendLzssBlockHeaderFields(1, last);

    // content
    const auto llCode = getFixedLLCode();
    const auto distCode = getFixedDistanceCode();
    sendLzssOutput(lzssData, llCode, distCode);

    sendEOB(llCode);
}

void removeTrailingZeros(std::vector<u32>& vec) {
    while (!vec.empty() && vec.back() == 0) {
        vec.pop_back();
    }
}

u32 countTrailingZeros(const std::vector<u32>& vec, std::vector<u32> permutation) {
    assert (vec.size() == permutation.size());
    const auto size = vec.size();
    for (int i = 0; i < size; ++i) {
        auto index = permutation.at(size - i - 1);
        auto item = vec.at(index);
        if (item != 0) {
            return i;
        }
    }
    return vec.size();
}

// Adapted from Bill's provided code for block type 2.
void GzipEncoder::sendBlockType2(std::vector<bitset> &lzssData, bool last) {
    // Send common header fields
    sendLzssBlockHeaderFields(2, last);

    // Get the dynamic codes to use
    const auto lengths = getDynamicCodeLengths(lzssData);
    auto llCodeLengths = lengths.first;
    auto distCodeLengths = lengths.second;
    const auto llCode = constructCanonicalCode(llCodeLengths);
    const auto distCode = constructCanonicalCode(distCodeLengths);

    // Compute the CL code and metadata
//    auto llCodeLengthCount = llCodeLengths.size() - countTrailingZeros(llCodeLengths);
//    auto distCodeLengthCount = distCodeLengths.size() - countTrailingZeros(distCodeLengths);
    removeTrailingZeros(llCodeLengths);
    removeTrailingZeros(distCodeLengths);
    auto clSymbols = getCLSymbols(llCodeLengths, distCodeLengths);
    auto clCodeLengths = getCLCodeLengths(clSymbols);
    auto clCode = constructCanonicalCode(clCodeLengths);

    // There needs to be at least one use of symbol 256 (EOB), so there must be at least 257 elements
    assert(llCodeLengths.size() >= 257);
    u32 HLIT = llCodeLengths.size() - 257;
    u32 HDIST = distCodeLengths.empty() ? 0 : distCodeLengths.size() - 1;

    // The lengths are written in a strange order, dictated by RFC 1951
    const std::vector<u32> clPermutation {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
    const u32 trailingCLZeros = countTrailingZeros(clCodeLengths, clPermutation);
    const u32 clCodesUsed = std::max( (u32)clCodeLengths.size() - trailingCLZeros, 4u);
    u32 HCLEN = clCodesUsed - 4;

    // These are all numbers so Rule #1 applies
    this->outBitStream.push_bits(HLIT, 5);
    this->outBitStream.push_bits(HDIST,5);
    this->outBitStream.push_bits(HCLEN,4);

    // Push each CL code length in 3 bits (Rule #1 applies).
    for (unsigned int i = 0; i < clCodesUsed; ++i) {
        auto len = clCodeLengths.at(clPermutation.at(i));
        this->outBitStream.push_bits(len,3);
    }

    // Send the LL and Distance code lengths with the CL code
    for (const auto& item : clSymbols) {
        const auto bits = item.size();
        bool isCLSymbol = bits == 4 || bits == 5; // Otherwise it must be an offset portion of an RLE sequence

        if (isCLSymbol) {
            const auto codeWord = clCode.at(item.to_ulong());
            pushMsbFirst(codeWord);
        }
        else {
            // RLE Offset value
            this->outBitStream.push_bits(item);
        }
    }
    if (distCodeLengths.empty()) {
        // Push a single length for the distance code
        // how many bits do I use for this?
        this->outBitStream.push_bits(0,1);
    }

    // send the actual block content
    sendLzssOutput(lzssData, llCode, distCode);

    sendEOB(llCode);
}

void GzipEncoder::sendLzssBlockHeaderFields(u8 type, bool last) {
    const auto isLast = last ? 1 : 0;
    this->outBitStream.push_bit(isLast);
    this->outBitStream.push_bits(type, 2);
}

void GzipEncoder::sendLzssOutput(
        const std::vector<bitset> &lzssData,
        const std::vector<bitset> &llCode,
        const std::vector<bitset> &distCode) {

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
}

void GzipEncoder::sendEOB(const std::vector<bitset>& llCode) {
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
