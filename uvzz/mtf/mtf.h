#ifndef UVZZ_MTF_H
#define UVZZ_MTF_H

#include <vector>
#include "binary.h"

/**
 * The Move To Front (mtf) transform is a simple transform that is particularly
 * useful in conjunction with delta, variable-length,  and run-length-encoding
 * compression techniques.
 *
 * https://en.wikipedia.org/wiki/Move-to-front_transform
 */
namespace mtf {

    /**
     * Perform the Move to Front transform on the given data.
     * @param input The data to tranform.
     * @return The transformed result.
     */
    std::vector<u8> transform(const std::vector<u8>& input);

    /**
     * Invert the Move to Front transform to produce the original data.
     * @param transformedData The output of a MTF transform.
     * @return The original data as it was before running transform().
     */
    std::vector<u8> invert(const std::vector<u8>& transformedData);
}

#endif //UVZZ_MTF_H
