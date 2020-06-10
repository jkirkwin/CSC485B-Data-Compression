#include "lzss_backref.h"
#include "lzss.h"

void LzssEncoder::flush() {
    // todo
}

void LzssEncoder::acceptByte(u8 byte) {
    // todo
    // fill the buffer until the lookahead is full.
    // if the buffer is full when this is called, try to find a backreference for the next data and output it.

    acceptBitset(bitset(LITERAL_BITS, byte)); // No backreferences or buffering done.
}