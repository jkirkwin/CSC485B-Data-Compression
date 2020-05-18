#include "binary_field.h"
#include "compress_fs_encoder.h"
#include <cassert>
#include <iostream>

// TODO Add tests

FSEncoder::FSEncoder(std::ostream* outStream) {
    // TODO consider making this conditional on receiving some input.
    inBuffer = new BinaryField(0,0);
    this->outStream = outStream;

    unsigned char magicNumMSB = (MAGIC_NUMBER >> 8) & 0xFF;
    unsigned char magicNumLSB = MAGIC_NUMBER & 0xFF;
    unsigned char mode = MODE;
    *outStream << magicNumMSB << magicNumLSB << mode;
}

FSEncoder::~FSEncoder() {
    delete inBuffer;
}

BinaryField getMsb(BinaryField field) {
    int shift = std::max(0, field.getBits() - 8);
    unsigned char msb = field.getData() >> shift;
    return BinaryField(msb, 8);
}

void FSEncoder::acceptData(BinaryField symbol) {
    symbol.reverse();
    
    // Group into bytes
    BinaryField reversedData = inBuffer->append(symbol); 
    while (reversedData.getBits() >= 8) {
        // Reverse each group of 8 bits and output it
        BinaryField msb = getMsb(reversedData);
        msb.reverse();
        *outStream << msb.getData();

        int newBitCount = reversedData.getBits() - 8;
        reversedData = BinaryField(reversedData.getData(), newBitCount);
    }

    // Buffer any remaining data
    *inBuffer = reversedData;
}

void FSEncoder::flush() {
    assert (inBuffer->getBits() < 8); // Sanity check

    if (inBuffer->getBits() > 0) {
        // Pad last byte to size and output
        unsigned char data = inBuffer->getData();
        unsigned char paddedData = data << (8 - inBuffer->getBits());
        *outStream << paddedData;
    }
}