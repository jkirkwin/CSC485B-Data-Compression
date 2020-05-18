/* CSC 485B
 * Jamie Kirkwin
 * Assignment 1: uvcompress
 */

#include <iostream>
#include "compress_fs_encoder.h"
#include "lzw_encoder.h"

/**
 * Uses the LZW algorithm as implemented in the UNIX compress utility to 
 * compress the input received via stdin and sends the result to stdout.
 * 
 * Reset markers are not used due to historical bugs in the compress utility's
 * implementation.  
 */ 
// TODO Test running the executable with no input (should produce no input) 
int main() {
    FSEncoder *fSEncoder = new FSEncoder();
    LzwEncoder lzwEncoder(fSEncoder);

    char c;
    while(std::cin.get(c)) {
        lzwEncoder.acceptChar(c);
    }
    lzwEncoder.flush();

    delete fSEncoder;
    return 0;
}