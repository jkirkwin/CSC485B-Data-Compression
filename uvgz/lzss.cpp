#include "lzss_backref.h"
#include "lzss.h"

void LzssEncoder::acceptByte(u8 item) {
    if (lookahead.full()) {
        encodeFromLookahead();
    }
    lookahead.push_back(item);
}

void LzssEncoder::flush() {
    while(!lookahead.empty()) {
        encodeFromLookahead();
    }
}

void LzssEncoder::encodeFromLookahead() {
    assert (!lookahead.empty());
    // todo do naive linear search here (and add a todo for a hash-based impl)
    const auto next = popLookahead();
    outputLiteral(next);
}

void LzssEncoder::outputLiteral(u8 literalValue) {
    const bitset literalEncoding(LITERAL_BITS, literalValue);
    writeSymbol(literalEncoding);
}

u8 LzssEncoder::popLookahead() {
        const auto front = lookahead.front();
        lookahead.pop_front();
        return front;
}