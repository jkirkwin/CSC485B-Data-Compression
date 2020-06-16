#ifndef UVGZ_LZSS_H
#define UVGZ_LZSS_H

#include <iostream>
#include <functional>
#include "shared/binary.h"
#include <boost/circular_buffer.hpp>
#include "lzss_backref.h"

/**
 * Encodes a byte stream as a sequence of literals and (length, distance) pairs
 * called "back-references" according to the LZSS algorithm.
 *
 * Back references have 2 components, which each have two sub-components.
 * 1. Length
 *      i. A symbol indicating the base length of the reference (9 bits)
 *     ii. An offset indicating an additional delta on top of the base length
 *         (0-5 bits)
 * 2. Distance
 *      i. A symbol indicating the base distance to travel to the beginning of
 *         the back-reference. (5 bits)
 *     ii. An offset to be added to the base distance. (0-13 bits)
 *
 * Output takes the form of a sequence of bitsets of varying lengths,
 * representing various instances of the 5 possible symbol types (literal,
 * length base, length offset, distance base, distance offset).
 *
 * A downstream symbol recipient can decode the symbol stream using the
 * following rules:
 *  1. If the next symbol uses 8 bits, treat it as a literal and move to the
 *     next one.
 *  2. If the next symbol uses 9 bits, it is the first component of a back-
 *     reference. Process it and the next 3 symbols as described above.
 */
class LzssEncoder {
public:

    // Type of the downstream entity required. Likely a callback to the object
    // giving input data.
    typedef std::function< void(bitset) > symbol_consumer_t;

    // todo consider increasing these
    static const int LOOKAHEAD_CAPACITY = 100u; // maximum allowed value of 258
    static const int HISTORY_CAPACITY = 1000u; // maximum allowed value of 32768

    /**
     * Constructs a new encoder.
     *
     * @param writeSymbol The function called when output symbols must be
     * written downstream. This happens when the buffer is full and input is
     * received, or when flush() is called.
     */
    explicit LzssEncoder(symbol_consumer_t& writeSymbol): writeSymbol(writeSymbol) {
        this->lookahead = buff_t (LOOKAHEAD_CAPACITY);
        this->history = buff_t (HISTORY_CAPACITY);
    }

    ~LzssEncoder() {
        flush();
    }

    /**
     * Accepts a single byte in. This may or may not result in a symbol or
     * symbols being output.
     */
    void acceptByte(u8 item);

    /**
     * Convenience member function to pass in multiple bytes at once.
     */
    void acceptData(const std::vector<u8> & bytes) {
        for(const auto byte : bytes) {
            acceptByte(byte);
        }
    }

    /**
     * Processes any content in the lookahead buffer. The lookahead will be
     * empty after this call.
     */
    void flush();

    /**
     * Encodes at least the first character in the lookahead buffer. More
     * than one character from the lookahead may be encoded by a single call.
     */
    void encodeFromLookahead();

    /**
     * Discard any existing history and pending input.
     */
    void reset();

private:
    typedef boost::circular_buffer<u8> buff_t;
    buff_t history;
    buff_t lookahead;

    symbol_consumer_t& writeSymbol;

    u8 popLookahead();

    void outputLiteral(u8 literalValue);

};
#endif // UVGZ_LZSS_H
