#include "uvid_decode.h"

namespace decode {

    YUVFrame420 IFrameToYCbCr(const CompressedIFrame &iFrame, dct::QualityLevel qualityLevel) {
        YUVFrame420 decodedFrame(iFrame.width, iFrame.height);
        IFrameToYCbCr(iFrame, decodedFrame, qualityLevel);
        return decodedFrame;
    }

    void IFrameToYCbCr(const CompressedIFrame &iFrame, YUVFrame420& decodedFrame, dct::QualityLevel qualityLevel) {
        int height = iFrame.height, width = iFrame.width;
        int scaledHeight = height / 2, scaledWidth = width / 2;

        auto yContext = dct::luminanceContext(height, width);
        auto yPlane = dct::invert(*iFrame.y, yContext, qualityLevel);

        auto colourContext = dct::chromananceContext(scaledHeight, scaledWidth);
        auto cbPlane = dct::invert(*iFrame.cb, colourContext, qualityLevel);
        auto crPlane = dct::invert(*iFrame.cr, colourContext, qualityLevel);

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
}