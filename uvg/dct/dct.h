#ifndef UVG_DCT_H
#define UVG_DCT_H

#include "Eigen/Dense"
// todo add quality param
/**
 * Contains functions for performing and inverting a Discrete Cosine Transform.
 *
 * The algorithm:
 * - The input matrix is partitioned into rectangular blocks.
 * - Each block is run through DCT to produce a coefficient matrix
 * - Each block of coefficients is quantized to reduce size.
 * - Each block is then linearized into a sequence of quantized coefficients in
 *   an easily compressible zig-zag pattern.
 *
 * Note that this is a lossy technique, and the result of calling transform()
 * followed by invert() is unlikely to yield the exact input data.
 */
namespace dct {
    const auto BLOCK_DIMENSION = 8;

    typedef Eigen::Matrix<unsigned char, BLOCK_DIMENSION, BLOCK_DIMENSION> raw_block_t;

    // todo make this an array of size 64
    // todo does this need to be an int? inspect the values of the matrix when running on test data
    typedef std::vector<int> encoded_block_t;

    namespace quantize {
        typedef Eigen::Matrix<int, BLOCK_DIMENSION, BLOCK_DIMENSION> quantizer_t;
        quantizer_t luminance();
        quantizer_t chromanance();
    }

    /**
     * Perform the DCT transform on the given matrix.
     *
     * @return An encoded sequence of DCT blocks. Each block is a transformed,
     * quantized, and linearized encoding of a region of the input matrix.
     * The first element of each block is the DC coefficient, which is followed
     * by the AC coefficients in order of proximity to the DC coefficient.
     */
    std::vector<encoded_block_t> transform(const Eigen::MatrixX<unsigned char>&, const quantize::quantizer_t&);

    struct context {
        unsigned int height, width;
        quantize::quantizer_t quantizer;
    };

    /**
     * Inverts the DCT transform given the set of encoded blocks.
     *
     * @return The decompressed matrix. Note: This is unlikely to be identical
     * to the original input. This DCT is a lossy process.
     */
    Eigen::MatrixX<unsigned char> invert(const std::vector<std::vector<int>>& blocks, const context&);

    /**
     * Decodes the given block into a matrix representation.
     */
    raw_block_t decodeBlock(const std::vector<int>& block, const quantize::quantizer_t&);
}

#endif
