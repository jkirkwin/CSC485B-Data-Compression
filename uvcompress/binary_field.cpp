#include "binary_field.h"
#include <cassert>

void BinaryField::reverse() {
    int mid = bits/2;
    int i;
    for (i = 0; i < mid; i++) {
        swapBits(i, bits - i - 1); 
    }
}

bool BinaryField::isBitSet(int i) {
    assert (0 <= i && i <= bits);
    return (data >> i) & (1);
}

void BinaryField::setBit(int i) {
    assert (0 <= i && i <= bits);
    
    // Use of 1U influenced by http://graphics.stanford.edu/~seander/bithacks.html#SwappingBitsXOR
    data = data | (1U << i);
}

void BinaryField::unsetBit(int i) {
    assert (0 <= i && i <= bits);
    
    // Use of 1U influenced by http://graphics.stanford.edu/~seander/bithacks.html#SwappingBitsXOR
    unsigned short mask = ~(1U << i);
    data = data & mask;
}

void BinaryField::swapBits(int i, int j) {
    assert (validBitCount(i) && validBitCount(j)); 
    if (isBitSet(i) == isBitSet(j)) {
        return;
    } else {
        flipBit(i);
        flipBit(j);
    }
}