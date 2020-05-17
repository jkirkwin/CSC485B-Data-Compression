#ifndef COMPRESS_FS_ENCODER_H
#define COMPRESS_FS_ENCODER_H

#include "binary_field.h"

#define MAGIC_NUMBER 0x1f9d 
#define MAGIC_NUMBER_BITS 16 
#define MODE 0x90
#define MODE_BITS 8

/*
 * Encodes an incoming bitstream into a bytestream which can be saved to a file
 * in the same format used by the UNIX compress tool. Writes output to stdout.
 */
class FSEncoder {
    public:
        FSEncoder();

        void acceptData(unsigned short data, int numBits) {
            BinaryField binaryFieldData(data, numBits);
            acceptData(binaryFieldData);
        }

        /*
         * Takes the given data as input. This does not guarantee that anything
         * is immediately output.
         */ 
        void acceptData(BinaryField data);

        /**
         * Flush the remaining buffered input to stdout. This should only be 
         * called when the entire input stream has been passed to this object
         * via acceptData(). 
         */ 
        void flush();

    private:
        BinaryField* inBuffer;
};

#endif