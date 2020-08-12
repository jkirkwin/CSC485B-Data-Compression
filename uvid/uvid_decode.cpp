#include "uvid_decode.h"

namespace decode {

    std::array<decoded_plane_t, 3> invertDct(const std::array<encoded_plane_t, 3>& encodedPlanes,
                                             dct::QualityLevel qualityLevel,
                                             u32 height,
                                             u32 width) {
        auto yContext = dct::luminanceContext(height, width);
        auto yPlane = dct::invert(encodedPlanes.at(0), yContext, qualityLevel);

        auto colourContext = dct::chromananceContext(height/2, width/2);
        auto cbPlane = dct::invert(encodedPlanes.at(1), colourContext, qualityLevel);
        auto crPlane = dct::invert(encodedPlanes.at(2), colourContext, qualityLevel);

        return {yPlane, cbPlane, crPlane};
    }

    YUVFrame420 IFrameToYCbCr(const CompressedIFrame &iFrame, dct::QualityLevel qualityLevel) {
        YUVFrame420 decodedFrame(iFrame.width, iFrame.height);
        IFrameToYCbCr(iFrame, decodedFrame, qualityLevel);
        return decodedFrame;
    }

    void IFrameToYCbCr(const CompressedIFrame &iFrame, YUVFrame420 &decodedFrame, dct::QualityLevel qualityLevel) {
        u32 height = iFrame.height, width = iFrame.width;
        u32 scaledHeight = height / 2, scaledWidth = width / 2;

        std::array<encoded_plane_t, 3> planes {iFrame.y, iFrame.cb, iFrame.cr};
        auto invertedPlanes = invertDct(planes, qualityLevel, height, width);
        auto yPlane = invertedPlanes.at(0);
        auto cbPlane = invertedPlanes.at(1);
        auto crPlane = invertedPlanes.at(2);

        for (u32 y = 0; y < height; y++) {
            for (u32 x = 0; x < width; x++) {
                decodedFrame.Y(x, y) = yPlane.at(y, x);
            }
        }
        for (u32 y = 0; y < scaledHeight; y++) {
            for (u32 x = 0; x < scaledWidth; x++) {
                decodedFrame.Cb(x, y) = cbPlane.at(y, x);
                decodedFrame.Cr(x, y) = crPlane.at(y, x);
            }
        }
    }

    void PFrameToYCbCr(const CompressedPFrame &pFrame, YUVFrame420 &previousFrame, YUVFrame420 &decodedFrame,
                       dct::QualityLevel qualityLevel) {
        auto height = pFrame.height, width = pFrame.width;

        std::array<encoded_plane_t, 3> planes {pFrame.y, pFrame.cb, pFrame.cr};
        auto invertedPlanes = invertDct(planes, qualityLevel, height, width);
        auto yPlane = invertedPlanes.at(0);
        auto cbPlane = invertedPlanes.at(1);
        auto crPlane = invertedPlanes.at(2);

        assert (pFrame.cb.size() == pFrame.macroblockHeaders.size());
        for (u32 i = 0; i < pFrame.macroblockHeaders.size(); ++i) {
            auto header = pFrame.macroblockHeaders.at(i);
            assert (!header.predicted); // todo implement decoding of predicted macroblocks.
        }

        for (u32 y = 0; y < height; y++) {
            for (u32 x = 0; x < width; x++) {
//                decodedFrame.Y(x, y) = yPlane.at(y, x);
                // todo testing issue by encoding only y blocks with diffs.
                decodedFrame.Y(x, y) = yPlane.at(y, x) + previousFrame.Y(x, y);
            }
        }
        for (u32 y = 0; y < height/2; y++) {
            for (u32 x = 0; x < width/2; x++) {
                decodedFrame.Cb(x, y) = cbPlane.at(y, x);
                decodedFrame.Cr(x, y) = crPlane.at(y, x);
            }
        }
    }

    YUVFrame420 PFrameToYCbCr(const CompressedPFrame& pFrame, YUVFrame420& previousFrame, dct::QualityLevel qualityLevel) {
        YUVFrame420 decodedFrame(pFrame.width, pFrame.height);
        PFrameToYCbCr(pFrame, previousFrame, decodedFrame, qualityLevel);
        return decodedFrame;
    }
}