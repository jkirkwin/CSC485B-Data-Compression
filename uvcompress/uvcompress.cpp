/* CSC 485B
 * Jamie Kirkwin
 * Assignment 1: uvcompress
 */

#include <iostream>
#include "compress_fs_encoder.h"
#include "lzw_encoder.h"
#include <functional>

LzwEncoder getLzwEncoder(FSEncoder& downstreamEncoder) {
    using namespace std::placeholders;
    consumer_t forwarder = std::bind(&FSEncoder::acceptData, &downstreamEncoder, _1);
    LzwEncoder lzwEncoder(forwarder);
    return lzwEncoder;
}

void sendStdinToEncoder(LzwEncoder encoder) {
    char c;
    while(std::cin.get(c)) {
        encoder.acceptChar(c);
    }
}

/**
 * Uses the LZW algorithm as implemented in the UNIX compress utility to 
 * compress the input received via stdin and sends the result to stdout.
 * 
 * Reset markers are not used due to historical bugs in the compress utility's
 * implementation.  
 */ 
// TODO Test running the executable with no input (should produce no output) 
int main() {
    FSEncoder fSEncoder; 
    auto lzwEncoder = getLzwEncoder(fSEncoder);    

    sendStdinToEncoder(lzwEncoder);

    lzwEncoder.flush();
    fSEncoder.flush();

    return 0;
}