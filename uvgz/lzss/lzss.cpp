#include "lzss.h"

LzssEncoder::LzssEncoder() {
    // todo
}

void LzssEncoder::flush() {
    // todo
}

void LzssEncoder::acceptByte(u8 byte) {
    // todo
    // fill the buffer until the lookahead is full.
    // if the buffer is full when this is called, try to find a backreference for the next data and output it.
    std::cout << byte;
}

// LzssBuffer::LzssBuffer() {

// }