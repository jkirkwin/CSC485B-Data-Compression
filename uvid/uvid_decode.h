#ifndef UVID_UVID_DECODE_H
#define UVID_UVID_DECODE_H

#include <tuple>
#include "yuv_stream.hpp"
#include "input_stream.hpp"
#include "dct/dct.h"

/**
 * Common decoding functionality needed by both compressor and decompressor.
 */
namespace decode {

    typedef std::vector<dct::encoded_block_t> encoded_plane_t;
    typedef matrix::Matrix<int> decoded_plane_t;

    struct CompressedIFrame {
        u32 height, width;
        encoded_plane_t y, cb, cr;

        CompressedIFrame(u32 h, u32 w, encoded_plane_t& y, encoded_plane_t& cb, encoded_plane_t& cr) :
            height(h), width(w), y(y), cb(cb), cr(cr) {
        }
    };

    /**
     * Decodes the given I-Frame and stores the result in a YCbCr Frame.
     */
    YUVFrame420 IFrameToYCbCr(const CompressedIFrame& iFrame, dct::QualityLevel qualityLevel);

    /**
     * Decodes the given I-Frame and stores the result in the provided YCbCr Frame.
     */
    void IFrameToYCbCr(const CompressedIFrame& iFrame, YUVFrame420& decodedFrame, dct::QualityLevel qualityLevel);


    /**
     * P-Frames are broken into macro-blocks. Each macro-block consists of an
     * 8x8 block of the subsampled Cb and Cr planes, and the 4 8x8 blocks of
     * the Y plane that share the same coordinates as the (unsampled) Cb and Cr
     * components.
     *
     * Macro-blocks may or may not be predicted. Those which are predicted
     * include a two-dimensional motion vector which points to the location in
     * the previous frame which was used to predict the the macro-block.
     *
     * A motion vector is stored as the difference between the macro-block's
     * top left corner and the top left corner of the predictive block in the
     * previous frame.
     */
    struct MacroblockHeader {
        bool predicted;
        int motionVectorX, motionVectorY;
    };

    struct CompressedPFrame {
        u32 height, width;
        encoded_plane_t y, cb, cr;
        std::vector<MacroblockHeader> macroblockHeaders;

        CompressedPFrame(u32 h, u32 w) : height(h), width(w) {
        }

        CompressedPFrame(u32 h, u32 w, encoded_plane_t& y, encoded_plane_t& cb, encoded_plane_t& cr, std::vector<MacroblockHeader>& headers) :
                height(h), width(w), y(y), cb(cb), cr(cr), macroblockHeaders(headers) {
        }
    };

    YUVFrame420 PFrameToYCbCr(const CompressedPFrame& pFrame, YUVFrame420& previousFrame, dct::QualityLevel qualityLevel);

    /**
     * @param pFrame The received predicted (P-) frame
     * @param previousFrame The previously *decoded* frame
     * @param resultFrame The frame in which to store the decoded result
     */
    void PFrameToYCbCr(const CompressedPFrame &pFrame, YUVFrame420 &previousFrame, YUVFrame420 &resultFrame,
                       dct::QualityLevel qualityLevel);

}

#endif //UVID_UVID_DECODE_H
