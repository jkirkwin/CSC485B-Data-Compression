#ifndef UVGZ_LZSS_H
#define UVGZ_LZSS_H

#include <iostream>
#include "shared/binary.h"


// todo is the total buffer size LOOKAHEAD + HISTORY + 1? Just LOOKAHEAD + HISTORY? Just HISTORY?
#define LOOKAHEAD_SIZE 258
#define HISTORY_SIZE 32768

/*
 * Given an input byte stream, encodes the byte stream as a sequence of
 * literals and (length, distance) pairs called "back-references" according to
 * the LZSS algorithm.
 */
class LzssEncoder {
public:
    // todo this class needs to either be given a callback to be used when the buffer gets full and when flush is called
    /**
     * todo
     */
    LzssEncoder(); // todo

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
};


typedef signed int buffer_index_t;

/**
 * A ring buffer implementation to be used by the LZSS encoder to store history and lookahead data.
 */
// class LzssBuffer {
// public:

//     LzssBuffer();

//     /**
//      *  Get the bytes from start (inclusive) to end (exclusive).
//      *
//      * @return a std::array of bytes.
//      */
//      void get(buffer_index_t start, buffer_index_t end);
// //    std::array<u8> get(int start, int end);


// //    u8 get(int i);

// //    u8 peekNext()

//     // todo might be easiest to wrap domain-specific functions around some standard implementation like
//     //      boost::circular_buffer
// };


#endif // UVGZ_LZSS_H
