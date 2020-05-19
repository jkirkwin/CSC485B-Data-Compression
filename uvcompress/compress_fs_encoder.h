#ifndef COMPRESS_FS_ENCODER_H
#define COMPRESS_FS_ENCODER_H

#include "binary_field.h"
#include <iostream>

#define MAGIC_NUMBER_MSB 0x1fU
#define MAGIC_NUMBER_LSB 0x9dU
#define MODE 0x90U
#define MODE_BITS 8

/*
 * Encodes an incoming bitstream into a bytestream which can be saved to a file
 * in the same format used by the UNIX compress tool. Writes output to stdout.
 */
class FSEncoder {
    public:

        /*
         * Creates an FSEncoder which sends output to the given stream, or to 
         * std::cout if no stream is specified.
         */ 
        FSEncoder(std::ostream* outStream = &std::cout);

        /*
         * Takes the given data as input. This does not guarantee that anything
         * is immediately output.
         */ 
        void acceptData(BinaryField data);

        /**
         * Flush the remaining buffered input to the output stream. This 
         * should only be called after the entire input stream has been passed 
         * to this object via acceptData(). 
         */ 
        void flush();

        ~FSEncoder();
    private:
        std::ostream* outStream;
        BinaryField* inBuffer;
};

#endif