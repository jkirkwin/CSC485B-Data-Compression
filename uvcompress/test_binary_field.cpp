#include <cassert>
#include "binary_field.h"
#include <iostream>

void testReverse() {
    // Maximum size field with all bits set
    BinaryField full(MAX_VALUE, MAX_BITS);
    assert(full.getBits() == MAX_BITS);
    assert(full.getData() == MAX_VALUE);
    full.reverse();
    assert(full.getBits() == MAX_BITS);
    assert(full.getData() == MAX_VALUE);
    
    // Empty field
    BinaryField empty(0, 0);
    assert(empty.getBits() == 0);
    assert(empty.getData() == 0);
    empty.reverse();
    assert(empty.getBits() == 0);
    assert(empty.getData() == 0);

    // Three bit field with only one set
    BinaryField oneOfThree(1, 3);
    assert(oneOfThree.getBits() == 3);
    assert(oneOfThree.getData() == 1);
    oneOfThree.reverse();
    assert(oneOfThree.getBits() == 3);
    assert(oneOfThree.getData() == 0b100);
}

void testAppend() {
    BinaryField one (1,1);
    BinaryField zero (0,1);
    BinaryField empty (3,0);
    BinaryField twoZeros (0, 2);
    BinaryField ff (0xff, 8);

    BinaryField oneZero = one.append(zero);
    assert (oneZero.getBits() == one.getBits() + zero.getBits());
    assert (oneZero.getData() == 0b10);

    BinaryField zeroOne = one.prepend(zero);
    assert (zeroOne.getBits() == one.getBits() + zero.getBits());
    assert (zeroOne.getData() == 1);

    BinaryField zeroEmpty = zero.append(empty);
    assert (zeroEmpty.getBits() == zero.getBits());
    assert (zeroEmpty.getData() == zero.getData());

    BinaryField ff0 = ff.append(twoZeros).append(twoZeros);
    assert (ff0.getBits() == ff.getBits() + 2*twoZeros.getBits());
    assert (ff0.getData() == 0xff0);
}

void testEquality() {
    BinaryField a1 (13, 7);
    BinaryField a2 (13, 7);
    BinaryField b (14, 7);
    BinaryField c (13, 6);

    assert (a1 == a2);
    assert ( !(a1 != a2) );

    assert ( !(a1 == b) );
    assert (a1 != b);

    assert ( !(a1 == c) );
    assert (a1 != c);
}

// todo refactor this (and update local env) to use boost test lib
int main() {
    std::cout << "BinaryField -- Running Tests." << std::endl;
    testReverse();
    testAppend();
    testEquality();
    std::cout << "BinaryField -- all tests passed." << std::endl;
}