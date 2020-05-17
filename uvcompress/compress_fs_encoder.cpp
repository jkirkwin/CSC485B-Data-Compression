#include "binary_field.h"
#include "compress_fs_encoder.h"
#include <cassert>
#include <iostream>

// TODO Inject an ostream rather than using cin directly
// TODO Add tests

FSEncoder::FSEncoder() {
    // TODO consider making this conditional on receiving some input.
    unsigned char magicNumMSB = (MAGIC_NUMBER >> 8) & 0xFF;
    unsigned char magicNumLSB = MAGIC_NUMBER & 0xFF;
    unsigned char mode = MODE;
    std::cout << magicNumMSB << magicNumLSB << mode;
    *inBuffer = BinaryField(0,0);
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
    while (reversedData.getBits() > 8) {
        // Reverse each group of 8 bits and output it
        BinaryField msb = getMsb(reversedData);
        msb.reverse();
        std::cout << msb.getData();

        int newBitCount = reversedData.getBits() - 8;
        reversedData = BinaryField(reversedData.getData(), newBitCount);
    }

    // Buffer any remaining data
    *inBuffer = reversedData;
}

void FSEncoder::flush() {
    assert (inBuffer->getBits() < 8); // Sanity check

    if (inBuffer->getBits() > 0) {
        // Pad last byte to size
        unsigned char data = inBuffer->getData();
        unsigned char paddedData = data << (8 - inBuffer->getBits());
        std::cout << paddedData;
    }
}