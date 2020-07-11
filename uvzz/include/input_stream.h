/* input_stream.hpp
   CSC 485B/CSC 578B/SENG 480B - Summer 2020

   Definition of a bitstream class which complies with the bit ordering
   required by the gzip format.

   (The member function definitions are all inline in this header file 
    for convenience, even though the use of such long inlined functions
    might be frowned upon under some style manuals)

   B. Bird - 06/19/2020

   Modified by J. Kirkwin - 07/10/2020
*/ 

#ifndef OUTPUT_STREAM_HPP
#define OUTPUT_STREAM_HPP

#include <iostream>
#include <cstdint>
#include "binary.h"


class InputBitStream{
public:
    /* Constructor */
    explicit InputBitStream( std::istream& input_stream = std::cin ): bitvec{0}, numbits{8}, infile{input_stream}, done{false}, last_real_bit{0} {

    }

    /* Destructor */
    virtual ~InputBitStream()= default;

    /* Read an entire byte from the stream, with the least significant bit read first */
    unsigned char read_byte(){
        return read_bits(8);
    }

    /* Read a 32 bit unsigned integer value (LSB first) */
    u32 read_u32(){
        return read_bits(8) | (read_bits(8)<<8) | (read_bits(8)<<16) | (read_bits(8)<<24);
    }

    /* Read a 16 bit unsigned short value (LSB first) */
    u16 read_u16(){
        return read_bits(8) | (read_bits(8)<<8);
    }

    /* Read the lowest order num_bits bits from the stream into a u32,
       with the least significant bit read first.
    */
    u32 read_bits(int num_bits){
        assert (num_bits >= 0);
        u32 result{};
        for (int i = 0; i < num_bits; i++)
            result |= read_bit()<<i;
        return result;
    }

    /* Read the next num_bits bits and return them as a u32 with the first bit read
     * treated as the most significant one.
     */
    u32 read_bits_msb_first(int num_bits) {
        assert (num_bits >= 0);
        u32 result {};
        for (int i = 0; i < num_bits; ++i) {
            result = (result << 1) | read_bit();
        }
        return result;
    }

    /* Read the lowest order num_bits bits from the stream into a bitset,
       with the least significant bit read first.
    */
    bitset read_bitfield(int num_bits) {
        assert (num_bits >= 0);
        return bitset(num_bits, read_bits(num_bits));
    }

    /* Read a single bit b (stored as the LSB of an unsigned int)
       from the stream */
    unsigned int read_bit(){
        //This has been set up to emit an infinite number of copies
        //of the last bit once EOF is reached (so if the last bit
        //in the file is a 1, any subsequent call to read_bit will
        //return 1).
        if (numbits == 8)
            input_byte();
        if (!done)
            last_real_bit = (bitvec>>(numbits++))&0x1u;
        
        return last_real_bit;
    }

    /* Force the next read to read a byte from the input file */
    void flush_to_byte(){
        numbits = 8;
    }

    /* Answers whether the original input file/stream has been fully
       consumed. This will return false immediately after reading the
       last byte, and will only begin to return true after subsequent
       reads.
     */
    bool input_depleted() const {
        return done;
    }

private:
    void input_byte(){
        char c;
        if (!infile.get(c)){
            done = true;
            numbits = 0;
            return;
        }
        bitvec = (unsigned char)c;
        numbits = 0;
    }
    u32 bitvec;
    u32 numbits;
    std::istream& infile;
    bool done;
    unsigned int last_real_bit;
};


#endif 