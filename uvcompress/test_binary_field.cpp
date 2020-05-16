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

// todo refactor this (and update local env) to use boost test lib
int main() {
    testReverse();
    std::cout << "all tests passed." << std::endl;
}