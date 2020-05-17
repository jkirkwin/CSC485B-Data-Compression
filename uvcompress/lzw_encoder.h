#ifndef LZW_H
#define LZW_H
#include <unordered_map>
#include "compress_fs_encoder.h" // todo decouple these

#define STARTING_INDEX_BITS 9
#define MAX_INDEX_BITS 16

// For convenience. 
/*
 * TODO after fully implementing the scheme this way, try using map and 
 * benchmark to compare the 
 */
typedef std::unordered_map<std::string, int> table_t; 

class LzwEncoder {
    public:
        LzwEncoder(FSEncoder*);
        void acceptChar(char);
        void flush();
    private:
        FSEncoder *fsEncoder; 
        table_t symbolTable;
        int nextIndex = 0;
        int numBits = STARTING_INDEX_BITS;
        std::string workingStr = "";
};

#endif