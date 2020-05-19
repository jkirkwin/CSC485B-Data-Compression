/* CSC 485B
 * Jamie Kirkwin
 * Assignment 1: uvcompress
 * V00875987
 */

#ifndef BIN_FIELD_H
#define BIN_FIELD_H

#include <cassert>
#include <cmath>

#define MAX_BITS 24 
#define MAX_VALUE pow(2, MAX_BITS) - 1

/*
 * Represents a sequence of between 0 and 24 bits.
 */ 
class BinaryField {
    public:
        BinaryField(unsigned int data, int bits) {
            assert (validBitCount(bits));
            this->bits = bits;
            this->data = data & getMask(bits);
        }
  
        int getBits() const {
            return bits;
        }
  
        int getData() {
            return data;
        }

        void reverse();

        bool operator== (const BinaryField &other) const {
            return this->bits == other.bits && 
                   this->data == other.data;
        }
        
        bool operator!= (const BinaryField &other) const {
            return ! (*this == other);
        }

        /*
         * Creates a new BinaryField which uses this as the least significant
         * bit sequence prepended by the given BinaryField.
         */ 
        BinaryField prepend(BinaryField);

        /*
         * Creates a new BinaryField which uses this as the most significant 
         * bit sequence followed by the given BinaryField. 
         */ 
        BinaryField append(BinaryField);

        /*
         * Answers whether the ith bit is set (where the lsbit is indexed 0).
         */ 
        bool isBitSet(int i);

        /*
         * Sets the ith bit (where the lsbit is indexed 0).
         */ 
        void setBit(int i); 
        
        /*
         * Clears the ith bit (where the lsbit is indexed 0).
         */ 
        void unsetBit(int i);

        /*
         * Clears the ith bit if it is set. Otherwise sets the ith bit (where 
         * the lsbit is indexed 0).
         */ 
        void flipBit(int i) {
            if (isBitSet(i)) {
                unsetBit(i);
            } else {
                setBit(i);
            }
        }

        /*
         * Sets or clears the ith bit (where the lsbit is indexed 0).
         */ 
        void setBitTo(int i, bool val) {
            if (val) {
                setBit(i);
            } else {
                unsetBit(i);
            }
        }
  
    private:
        unsigned int data;
        int bits;

        bool validBitCount(int i) {
            return 0 <= i && i <= MAX_BITS;
        }

        /*
         * Returns a mask of n bits padded (on the left) with zeros.
         * 
         * From https://stackoverflow.com/questions/1392059/algorithm-to-generate-bit-mask
         */
        int getMask(int n) {
            return (1 << n) - 1;
        }

        void swapBits(int i, int j); 
};

#endif