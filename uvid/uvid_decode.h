#ifndef UVID_UVID_DECODE_H
#define UVID_UVID_DECODE_H

#include "yuv_stream.hpp"
#include "input_stream.hpp"
#include "dct/dct.h"

namespace decode {

    struct CompressedIFrame {
        typedef std::vector<dct::encoded_block_t> plane_t;
        int height, width;
        plane_t *y, *cb, *cr;

        CompressedIFrame(u32 h, u32 w, plane_t *y, plane_t *cb, plane_t *cr) :
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

}

#endif //UVID_UVID_DECODE_H
