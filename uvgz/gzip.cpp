#include "gzip.h"

void GzipEncoder::encode(std::istream& inStream) {
    pushHeader();

    // read chunks of data until input is empty
    auto chunk = readChunk(inStream);
    while(!chunk.empty()) {
        updateFooterValues(chunk);
        processChunk(chunk);
        chunk = readChunk(inStream);
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

void GzipEncoder::processChunk(chunk_t &data) {
    assert(false);
    // todo
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
    this->outBitStream.push_u32(this->crc);
    this->outBitStream.push_u32(this->inputSize);
    this->inputSize = 0;
}

