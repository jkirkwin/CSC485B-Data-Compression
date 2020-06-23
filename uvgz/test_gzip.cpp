#define CATCH_CONFIG_MAIN

#include "catch/catch.hpp"
#include "gzip.h"

void verifyGzipHeader(std::istream& output) {
    const int headerBytes = 10;
    char headerBuf[headerBytes];
    output.read(headerBuf, headerBytes);

    REQUIRE( (u8)headerBuf[0] == 0x1f); // Magic Number
    REQUIRE( (u8)headerBuf[1] == 0x8b); // Magic Number
    REQUIRE( (u8)headerBuf[2] == 0x08); // CM
    REQUIRE( (u8)headerBuf[3] == 0x00); // Flags
    REQUIRE( (u8)headerBuf[8] == 0x00); // XFL
    REQUIRE( (u8)headerBuf[9] == 0x03); //OS
}

// todo
TEST_CASE("Smoke test", "[gzip]") {
    std::istringstream gzipInput("Hello");
    std::stringstream gzipOutput;

    GzipEncoder gzipEncoder(gzipOutput);
    gzipEncoder.encode(gzipInput);

    // GZIP Header
    verifyGzipHeader(gzipOutput);

    // Block Header
    u8 firstByte = gzipOutput.get();

    std::cout << std::hex << firstByte;

    const auto isLast = firstByte & 0b10000000;
    REQUIRE(isLast);
    const auto blockType = firstByte & 0x60; // X__X XXXX
    REQUIRE(blockType == 0x40); // X10X XXXX

    // Block contents

    // Expect EOB marker

    // Read Footer


    REQUIRE(false); // todo
}