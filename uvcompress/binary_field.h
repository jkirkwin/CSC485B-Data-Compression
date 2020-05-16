#ifndef BIN_FIELD_H
#define BIN_FIELD_H

#include <cassert>
#include <cmath>

#define MAX_BITS 16
#define MAX_VALUE pow(2, MAX_BITS) - 1

/*
 * Represents a sequence of between 0 and 17 bits.
 */ 
class BinaryField {
    public:
        BinaryField(unsigned short data, int bits) {
            assert (validBitCount(bits));
            this->bits = bits;
            this->data = data;
        }
  
        int getBits() const {
            return bits;
        }
  
        int getData() {
            return data;
        }

        void reverse();

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
        unsigned short data;
        int bits;

        bool validBitCount(int i) {
            return 0 <= i && i <= MAX_BITS;
        }

        void swapBits(int i, int j); 
};

#endif