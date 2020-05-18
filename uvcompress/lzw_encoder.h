#ifndef LZW_H
#define LZW_H
#include <unordered_map>
#include <functional>
#include "binary_field.h"

#define STARTING_INDEX_BITS 9
#define MAX_INDEX_BITS 16

// For convenience. 
typedef std::unordered_map<std::string, int> table_t; 
typedef std::function< void(BinaryField) > consumer_t;

/*
 * Takes in characters and performs the LZW algorithm to encode them as 
 * BinaryField's which are then written downstream.
 */ 
class LzwEncoder {
    public:
        /*
         * Creates a new LzwEncoder which sends its output to the given 
         * function.
         */
        LzwEncoder(consumer_t& acceptBinaryData);

        /*
         * Processes the next input character. This may or may not result in a
         * write to the downstream entity.
         */ 
        void acceptChar(char);

        /*
         * Force any buffered data to be written downstream. This should only
         * be called after all input has been piped into the encoder.
         */ 
        void flush();

    private:
        table_t symbolTable;
        int nextIndex = 0;
        int numBits = STARTING_INDEX_BITS;
        std::string workingStr = "";
        consumer_t acceptBinaryData;
};

#endif