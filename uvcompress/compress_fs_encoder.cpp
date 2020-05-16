#include "binary_field.h"
#include "compress_fs_encoder.h"
#include <cassert>

void FSEncoder::acceptData(BinaryField data) {
    data.reverse();
    assert(false);
    // todo implement
}

void FSEncoder::flush() {
    assert (false); // todo implement
}