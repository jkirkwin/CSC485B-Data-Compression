#ifndef UVG_DCT_H
#define UVG_DCT_H

#include <utility>

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
    const auto BLOCK_CAPACITY = 64;

    typedef Eigen::Matrix<unsigned char, BLOCK_DIMENSION, BLOCK_DIMENSION> raw_block_t;

    // todo make this an array of size 64
    typedef std::vector<int> encoded_block_t;

    /**
     * Holds accessors for various useful quantization matrixes.
     */
    namespace quantize {
        typedef Eigen::Matrix<int, BLOCK_DIMENSION, BLOCK_DIMENSION> quantizer_t;

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
            return quantizer_t::Ones();
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
    std::vector<encoded_block_t> transform(const Eigen::MatrixX<unsigned char>&, const quantize::quantizer_t&);

    /**
     * Encode a single block.
     *
     * The input matrix is transformed via DCT, quantized, and linearized.
     */
    encoded_block_t encodeBlock(const raw_block_t& rawBlock, const quantize::quantizer_t&);


    // todo rename to inversionContext and add docstring
    struct context {
        unsigned int height, width;
        quantize::quantizer_t quantizer;

        context(unsigned int h, unsigned int w, quantize::quantizer_t  q):
            height(h), width(w), quantizer(std::move(q)) {
        }
    };

    inline context luminanceContext(unsigned int h, unsigned int w) {
        return context(h, w, quantize::luminance());
    }

    inline context chromananceContext(unsigned int h, unsigned int w) {
        return context(h, w, quantize::chromanance());
    }

    /**
     * Inverts the DCT transform given the set of encoded blocks.
     *
     * @return The decompressed matrix. Note: This is unlikely to be identical
     * to the original input. This DCT is a lossy process.
     */
    Eigen::MatrixX<unsigned char> invert(const std::vector<encoded_block_t>& blocks, const context&);

    /**
     * Decodes the given block into a matrix representation.
     */
    raw_block_t decodeBlock(const encoded_block_t& block, const quantize::quantizer_t&);
}

#endif
