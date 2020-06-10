#ifndef UVGZ_LZSS_H
#define UVGZ_LZSS_H

#include <iostream>
#include "shared/binary.h"
#include <boost/circular_buffer.hpp>
#include "lzss_backref.h"


/**
 * A ring buffer implementation to be used by the LZSS encoder to store history
 * and lookahead data.
 */
class LzssBuffer {
public:
    typedef signed int buffer_index_t;

    const int LOOKAHEAD_SIZE = 258;
    const int HISTORY_SIZE = 32768;
    const int TOTAL_BUFF_SIZE = LOOKAHEAD_SIZE + HISTORY_SIZE; // todo is this right?

    LzssBuffer() {
        this->ringBuff = buff_t (TOTAL_BUFF_SIZE);
    }

//     /**
//      *  Get the bytes from start (inclusive) to end (exclusive).
//      *
//      * @return a std::array of bytes.
//      */
//      void get(buffer_index_t start, buffer_index_t end);
// //    std::array<u8> get(int start, int end);


// //    u8 get(int i);

// //    u8 peekNext()

private:
    typedef boost::circular_buffer<bitset> buff_t;
    buff_t ringBuff;

    //     // todo might be easiest to wrap domain-specific functions around some standard implementation like
    //     //      boost::circular_buffer
};


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
    typedef std::function< void(bitset) > bitset_consumer_t;

    /**
     * Constructs a new encoder.
     *
     * @param acceptSymbol The function called when output symbols must be written downstream. This happens when the
     * buffer is full and input is received, or when flush() is called.
     */
    explicit LzssEncoder(bitset_consumer_t &acceptBitset) : acceptBitset(acceptBitset) {
    }

    /**
     * Accepts a single byte in. This may or may not result in a symbol or
     * symbols being output.
     */
    void acceptByte(u8 byte);

    /**
     * Any unprocessed input sitting in the lookahead buffer is processed and
     * all encoded data is guaranteed to be output after this call.
     */
    void flush();

    /**
     * todo
     */
    // ~LzssEncoder(); // todo needed?
private:
    // todo add buffer - need to be able to shift it over as we go
    //      Ring buffer?
    LzssBuffer buffer;
    bitset_consumer_t& acceptBitset;
};

#endif // UVGZ_LZSS_H
