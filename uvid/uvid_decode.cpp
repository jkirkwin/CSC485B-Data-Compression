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

    unsigned char clampToByte(int i) {
        if (i < 0) {
            return 0;
        }
        else if (i > 255) {
            return 255;
        }
        else {
            return (unsigned char) i;
        }
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
                decodedFrame.Y(x, y) = clampToByte(yPlane.at(y, x));
            }
        }
        for (u32 y = 0; y < scaledHeight; y++) {
            for (u32 x = 0; x < scaledWidth; x++) {
                decodedFrame.Cb(x, y) = clampToByte(cbPlane.at(y, x));
                decodedFrame.Cr(x, y) = clampToByte(crPlane.at(y, x));
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

            // todo decode general macroblocks
            assert (header.predicted);
            assert (header.motionVectorX == 0);
            assert (header.motionVectorY == 0);
        }

        // Undo the global diff
        for (u32 y = 0; y < height; y++) {
            for (u32 x = 0; x < width; x++) {
                int actual = yPlane.at(y, x) + (int)previousFrame.Y(x, y);
                decodedFrame.Y(x, y) = clampToByte(actual);
            }
        }
        for (u32 y = 0; y < height/2; y++) {
            for (u32 x = 0; x < width/2; x++) {
                int cbActual = cbPlane.at(y, x) + (int)previousFrame.Cb(x, y);
                int crActual = crPlane.at(y, x) + (int)previousFrame.Cr(x, y);
                decodedFrame.Cb(x, y) = clampToByte(cbActual);
                decodedFrame.Cr(x, y) = clampToByte(crActual);
            }
        }
    }

    YUVFrame420 PFrameToYCbCr(const CompressedPFrame& pFrame, YUVFrame420& previousFrame, dct::QualityLevel qualityLevel) {
        YUVFrame420 decodedFrame(pFrame.width, pFrame.height);
        PFrameToYCbCr(pFrame, previousFrame, decodedFrame, qualityLevel);
        return decodedFrame;
    }
}