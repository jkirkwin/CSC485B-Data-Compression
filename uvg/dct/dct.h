#ifndef UVG_DCT_H
#define UVG_DCT_H

#include <utility>
#include <vector>
#include "matrix.h"

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
    const auto BLOCK_CAPACITY = 64;

    typedef std::array<unsigned char, BLOCK_CAPACITY> raw_block_t;
    typedef std::array<int, BLOCK_CAPACITY> encoded_block_t;

    /**
     * Holds accessors for various useful quantization matrixes.
     */
    namespace quantize {
        typedef matrix::Matrix<int> quantizer_t;

        /**
         * @return The quantizer used by JPEG for the luminance plane in a
         * YCbCr image.
         */
        quantizer_t luminance();

        /**
         * @return The quantizer used by JPEG for the chromanance planes in a
         * YCbCr image.
         */
        quantizer_t chromanance();

        /**
         * @return A quantizer matrix of all 1's. If this is used, the result
         * of the DCT process will be the rounded coefficients of the DCT
         * result i.e. no real quantization).
         */
        inline quantizer_t none() {
            return quantizer_t (BLOCK_DIMENSION, BLOCK_DIMENSION, 1);
        }
    }

    /**
     * Perform a block-based DCT transform on the given matrix.
     *
     * @return An encoded sequence of DCT blocks. Each block is a transformed,
     * quantized, and linearized encoding of a region of the input matrix.
     * The first element of each block is the DC coefficient, which is followed
     * by the AC coefficients in order of proximity to the DC coefficient.
     */
    std::vector<encoded_block_t> transform(const matrix::Matrix<unsigned char>&, const quantize::quantizer_t&);

    /**
     * Encode a single block.
     *
     * The input matrix is transformed via DCT, quantized, and linearized.
     */
    encoded_block_t encodeBlock(const raw_block_t& rawBlock, const quantize::quantizer_t&);

    struct inversionContext {
        unsigned int height, width;
        quantize::quantizer_t quantizer;

        inversionContext(unsigned int h, unsigned int w, quantize::quantizer_t  q):
            height(h), width(w), quantizer(std::move(q)) {
        }
    };

    inline inversionContext luminanceContext(unsigned int h, unsigned int w) {
        return inversionContext(h, w, quantize::luminance());
    }

    inline inversionContext chromananceContext(unsigned int h, unsigned int w) {
        return inversionContext(h, w, quantize::chromanance());
    }

    /**
     * Inverts the DCT transform given the set of encoded blocks.
     *
     * @return The decompressed matrix. Note: This is unlikely to be identical
     * to the original input. This DCT is a lossy process.
     */
    matrix::Matrix<unsigned char> invert(const std::vector<encoded_block_t>& blocks, const inversionContext&);

    /**
     * Decodes the given block into a matrix representation.
     */
    raw_block_t decodeBlock(const encoded_block_t& block, const quantize::quantizer_t&);
}

#endif
