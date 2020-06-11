#ifndef UVGZ_GZIP_H
#define UVGZ_GZIP_H

#include "shared/binary.h"
#include "shared/output_stream.hpp"
#include "CRC.h"

class GzipEncoder {
public:

    /**
     * Creates a new GZip encoder that will write to the given ostream when
     * run() is called.
     */
    explicit GzipEncoder(std::ostream& outStream=std::cout): outBitStream(OutputBitStream(outStream)){
        // intentionally empty
    }

    /**
     * Encode the contents of the given istream.
     */
    void encode(std::istream& inStream);

private:
    typedef CRC::Table<crcpp_uint32, 32> crc_table_t;
    const crc_table_t crcTable = crc_table_t(CRC::CRC_32()); // table used to increase speed
    crcpp_uint32 crc = 0; // The running CRC32 value
    u32 inputSize = 0; // Running counter
    OutputBitStream outBitStream;

    typedef std::vector<u8> chunk_t;
    static chunk_t readChunk(std::istream& inStream, u32 chunkSize=mb);
    void processChunk(chunk_t& data);
    void updateFooterValues(chunk_t& chunk);

    void pushHeader();
    void pushFooter();
};

#endif //UVGZ_GZIP_H
