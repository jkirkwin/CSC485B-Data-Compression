#include <iostream>
#include "output_stream.hpp"
#include <string>
#include "CRC.h"

const std::string V_NUMBER_STRING = "V00875987";
const auto V_NUMBER_VALUE = 875987u;

void sendBlock(OutputBitStream& outBitStream) {
    // header fields
    const auto isLast = 1;
    outBitStream.push_bit(isLast);
    const auto type = 0;
    outBitStream.push_bits(type, 2);

    // pad to byte boundary
    outBitStream.flush_to_byte();

    // len and ~len
    auto len = V_NUMBER_STRING.length();
    outBitStream.push_u16(len);
    outBitStream.push_u16(~len);

    // content
    for (const u8 byte: V_NUMBER_STRING) {
        outBitStream.push_byte(byte);
    }
}

/*
 * Write a generic GZip header.
 */
void pushHeader(OutputBitStream& outBitStream) {
    outBitStream.push_bytes(
            0x1f, 0x8b, //Magic Number
            0x08, //Compression (0x08 = DEFLATE)
            0x10, //Flags - add comment field
            0x00, 0x00, 0x00, 0x00, //MTIME (little endian)
            0x00, //Extra flags
            0x03 //OS (Linux)
    );

    // push a comment field to increase size to required amount 
    const auto limit = V_NUMBER_VALUE - V_NUMBER_STRING.length() - 24; // Output must be V_NUMBER_VALUE bytes in total
    for (int i = 0; i < limit; ++i) {
        outBitStream.push_byte(0xFF);
    }
    outBitStream.push_byte(0);
}

void pushFooter(OutputBitStream& outBitStream) {
    auto inputSize = V_NUMBER_STRING.length();
    auto crc = CRC::Calculate(V_NUMBER_STRING.c_str(), inputSize, CRC::CRC_32());

    outBitStream.flush_to_byte();
    outBitStream.push_u32(crc);
    outBitStream.push_u32(inputSize);
}

int main() {
    OutputBitStream outBitStream;
    pushHeader(outBitStream);
    sendBlock(outBitStream);
    pushFooter(outBitStream);
}