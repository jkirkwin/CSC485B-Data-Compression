#ifndef UVGZ_GZIP_H
#define UVGZ_GZIP_H

#include "shared/binary.h"
#include "shared/output_stream.hpp"
#include "CRC.h"
#include "lzss.h"

class GzipEncoder {
public:

    const u32 MAX_TYPE_0_SIZE = (1u << 16u) - 1;

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
    void encode(std::istream& inStream=std::cin);

private:
    typedef CRC::Table<crcpp_uint32, 32> crc_table_t;
    const crc_table_t crcTable = crc_table_t(CRC::CRC_32()); // table used to increase speed
    u32 crc = 0; // Running CRC32 value
    u32 inputSize = 0; // Running counter
    OutputBitStream outBitStream;

    typedef std::vector<u8> chunk_t;
    static chunk_t readChunk(std::istream& inStream, u32 chunkSize=mb);

    typedef std::vector<bitset> bitset_vec_t;

    void processInput(std::istream& inStream);
    void sendBlocks(std::vector<u8> & rawData, bitset_vec_t & lzssData, bool endOfData);
    void sendBlockType0(std::vector<u8> &data, bool last);
    void sendBlockType1(std::vector<bitset> &lzssData, bool last);
    void sendBlockType2(std::vector<bitset> &lzssData, bool last);
    void sendLzssBlockHeaderFields(u8 type, bool last);
    void sendLzssOutput(const bitset_vec_t &lzssData, const bitset_vec_t &llCode, const bitset_vec_t &distCode);
    void sendEOB(const bitset_vec_t &llCode);

    void updateFooterValues(chunk_t& chunk);

    void pushHeader();
    void pushFooter();
    void pushMsbFirst(const bitset& bits);
};

#endif //UVGZ_GZIP_H
