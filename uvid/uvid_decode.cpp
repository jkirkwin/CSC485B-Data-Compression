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

    void setPredictedYBlock(const MacroblockHeader &header, YUVFrame420 &previousFrame, YUVFrame420 &decodedFrame, u32 row, u32 col, decoded_plane_t& yPlane) {
        // To keep these values smaller, they are encoded in the sub-sampled colour planes' coordinates.
        int colOffset = header.motionVectorX * 2;
        int rowOffset = header.motionVectorY * 2;

        u32 maxRow = std::min(yPlane.rows, row + 16);
        u32 maxCol = std::min(yPlane.cols, col + 16);

        for (u32 rowIndex = row; rowIndex < maxRow; ++rowIndex) {
            for (u32 colIndex = col; colIndex < maxCol; ++colIndex) {
                int encoded = yPlane.at(rowIndex, colIndex);
                if (header.predicted) {
                    // Undo the diff that was taken during compression.
                    auto prevRowIndex = rowIndex + rowOffset;
                    auto prevColIndex = colIndex + colOffset;
                    int prev = previousFrame.Y(prevColIndex, prevRowIndex);
                    int actual = encoded + prev;
                    decodedFrame.Y(colIndex, rowIndex) = clampToByte(actual);
                }
                else {
                    // The encoded value is a literal - no diff was taken.
                    decodedFrame.Y(colIndex, rowIndex) = clampToByte(encoded);
                }
            }
        }
    }

    void setPredictedColourBlocks(const MacroblockHeader &header, YUVFrame420 &previousFrame, YUVFrame420 &decodedFrame, u32 row, u32 col, decoded_plane_t& cbPlane, decoded_plane_t& crPlane)  {
        auto scaledRow = row/2;
        auto scaledCol = col/2;

        // To keep these values smaller, they are encoded in the sub-sampled colour planes' coordinates.
        int colOffset = header.motionVectorX;
        int rowOffset = header.motionVectorY;

        u32 maxRow = std::min(cbPlane.rows, scaledRow + 8);
        u32 maxCol = std::min(cbPlane.cols, scaledCol + 8);
        for (u32 rowIndex = scaledRow; rowIndex < maxRow; ++rowIndex) {
            for (u32 colIndex = scaledCol; colIndex < maxCol; ++colIndex) {
                int encodedCb = cbPlane.at(rowIndex, colIndex);
                int encodedCr = crPlane.at(rowIndex, colIndex);

                if (header.predicted) {
                    // Undo the diff that was taken during compression.
                    auto prevRowIndex = rowIndex + rowOffset;
                    auto prevColIndex = colIndex + colOffset;

                    // Undo Cb diff
                    int prevCb = previousFrame.Cb(prevColIndex, prevRowIndex);
                    int actualCb = encodedCb + prevCb;
                    decodedFrame.Cb(colIndex, rowIndex) = clampToByte(actualCb);

                    // Undo Cr diff
                    int prevCr = previousFrame.Cr(prevColIndex, prevRowIndex);
                    int actualCr = encodedCr + prevCr;
                    decodedFrame.Cr(colIndex, rowIndex) = clampToByte(actualCr);
                }
                else {
                    // The encoded value is a literal - no diff was taken.
                    decodedFrame.Cb(colIndex, rowIndex) = clampToByte(encodedCb);
                    decodedFrame.Cr(colIndex, rowIndex) = clampToByte(encodedCr);
                }
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

        // Decode each macroblock
        assert (pFrame.cb.size() == pFrame.macroblockHeaders.size());
        int i = 0;
        for (u32 row = 0; row < height; row += 16) {
            for (u32 col = 0; col < width; col += 16) {
                // There is one macroblock for each 16x16 chunk of the image.
                auto macroBlockHeader = pFrame.macroblockHeaders.at(i++);

                // todo clean this up so we can call a single function 3 times.
                setPredictedYBlock(macroBlockHeader, previousFrame, decodedFrame, row, col, yPlane);
                setPredictedColourBlocks(macroBlockHeader, previousFrame, decodedFrame, row, col, cbPlane, crPlane);
            }
        }
    }

    YUVFrame420 PFrameToYCbCr(const CompressedPFrame& pFrame, YUVFrame420& previousFrame, dct::QualityLevel qualityLevel) {
        YUVFrame420 decodedFrame(pFrame.width, pFrame.height);
        PFrameToYCbCr(pFrame, previousFrame, decodedFrame, qualityLevel);
        return decodedFrame;
    }
}