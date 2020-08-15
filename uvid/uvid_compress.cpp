/* uvid_compress.cpp
   CSC 485B/578B/SENG 480B - Data Compression - Summer 2020

   todo update this docstring
   Starter code for Assignment 5

   Reads video data from stdin in uncompresed YCbCr (YUV) format 
   (With 4:2:0 subsampling). To produce this format from 
   arbitrary video data in a popular format, use the ffmpeg
   tool and a command like 

     ffmpeg -i videofile.mp4 -f rawvideo -pixel_format yuv420p - 2>/dev/null | ./this_program <width> height>

   Note that since the width/height of each frame is not encoded into the raw
   video stream, those values must be provided to the program as arguments.

   B. Bird - 07/15/2020
   J. Kirkwin - 08/07/2020
*/

#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <cassert>
#include <cstdint>
#include <tuple>
#include "output_stream.hpp"
#include "yuv_stream.hpp"
#include "dct/dct.h"
#include "delta/delta.h"
#include <stdexcept>
#include "uvid_decode.h"
#include <optional>

dct::QualityLevel getQualityLevel(const std::string& qualityString) {
    if (qualityString == "low") {
        return dct::low;
    }
    else if (qualityString == "medium") {
        return dct::med;
    }
    else if (qualityString == "high") {
        return dct::high;
    }
    else {
        std::cerr << "Bad quality level provided: '" << qualityString << "'. Acceptable values are 'low', 'medium', and 'high'" << std::endl;
        throw std::invalid_argument("Bad quality level");
    }
}

void sendQualityLevel(dct::QualityLevel qualityLevel, OutputBitStream& outputBitStream) {
    auto numBits = 2;
    if (qualityLevel == dct::low) {
        outputBitStream.push_bits(0, numBits);
    }
    else if(qualityLevel == dct::med) {
        outputBitStream.push_bits(1, numBits);
    }
    else {
        assert (qualityLevel == dct::high);
        outputBitStream.push_bits(2, numBits);
    }
}

void writeBlocks(const std::vector<dct::encoded_block_t>& blocks, OutputBitStream& outputBitStream) {
    for (const auto& block : blocks) {
        assert (block.size() == dct::BLOCK_CAPACITY);

        // Push the DC coefficient and the first AC coefficient as literals.
        auto dc = block.front();
        outputBitStream.push_u32(dc);
        auto ac0 = block.at(1);
        outputBitStream.push_u32(ac0);

        // Push the remaining AC coefficients as deltas
        auto prev = ac0;
        for (int i = 2; i < block.size(); ++i) {
            int cur = block.at(i);
            int diff = cur - prev;
            auto encodedDiff = delta::encode(diff);
            outputBitStream.push_bits_msb_first(encodedDiff);

            prev = cur;
        }
    }
}

decode::CompressedIFrame getIFrame(YUVFrame420& inputFrame, dct::QualityLevel qualityLevel) {
    auto yPlane = inputFrame.getIntPlaneY();
    auto cbPlane = inputFrame.getIntPlaneCb();
    auto crPlane = inputFrame.getIntPlaneCr();

    auto encodedYPlane = dct::transform(yPlane, dct::quantize::luminance(), qualityLevel);
    auto encodedCbPlane = dct::transform(cbPlane, dct::quantize::chromanance(), qualityLevel);
    auto encodedCrPlane = dct::transform(crPlane, dct::quantize::chromanance(), qualityLevel);

    auto height = yPlane.rows;
    auto width = yPlane.cols;
    return decode::CompressedIFrame(height, width, encodedYPlane, encodedCbPlane, encodedCrPlane);
}

void pushIFrame(OutputBitStream& outputBitStream, decode::CompressedIFrame& iFrame) {
    writeBlocks(iFrame.y, outputBitStream);
    writeBlocks(iFrame.cb, outputBitStream);
    writeBlocks(iFrame.cr, outputBitStream);
}

struct Macroblock {
    u32 topLeftX, topLeftY;
    typedef matrix::Matrix<int> block_t;
    block_t yBlock, cbBlock, crBlock;

    Macroblock(block_t& y, block_t& cb, block_t& cr, u32 xCoord, u32 yCoord) :
        yBlock(y), cbBlock(cb), crBlock(cr), topLeftX(xCoord), topLeftY(yCoord) {

    }
};

Macroblock getMacroblockFromYUVFrame(YUVFrame420& previous, u32 row, u32 col, u32 height, u32 width) {
    // Row and col give the top-left coordinates of the chunk of the previous block to be used.

    // Copy the colour blocks.
    std::vector<int> cbData, crData;
    for (u32 x = col; x < col + width; ++x) {
        for (u32 y = row; y < row + height; ++y) {
            cbData.push_back(previous.Cb(x, y));
            crData.push_back(previous.Cr(x, y));
        }
    }
    // Copy the y blocks
    std::vector<int> yData;
    for (u32 x = col*2; x < (col + width)*2; ++x) {
        for (u32 y = row*2; y < (row + height)*2; ++y) {
            yData.push_back(previous.Y(x, y));
        }
    }

    // Create the matrices and return the Macroblock struct
    Macroblock::block_t yBlock(height*2, width*2, yData);
    Macroblock::block_t cbBlock(height, width, cbData);
    Macroblock::block_t crBlock(height, width, crData);
    return Macroblock(yBlock, cbBlock, crBlock, col, row);
}

std::vector<std::pair<u32, u32>> getLocalSearchCoordinates(u32 row, u32 col, u32 height, u32 width) {
    // Return each point above/below/diagonal by any of these amounts to the point given that lie inside the image.
    std::vector<int> spreadFactors {1, 10, 20}; // todo find better factors by benchmarking

    std::vector<std::pair<u32, u32>> results;
    results.push_back({col, row}); // Always check the same position in the previous frame.
    for (auto spread : spreadFactors) {
        for (int i = -1; i < 2; ++i) {
            for (int j = -1; j < 2; ++j) {
                if (i != 0 || j != 0) {
                    int x = (int)col + spread*i;
                    int y = (int)row + spread*j;
                    auto validX = 0 <= x && x + 8 < width;
                    auto validY = 0 <= y && y + 8 < height;
                    if (validX && validY) {
                        results.push_back({x,y});
                    }
                }
            }
        }
    }
    return results;
}

// Subtracts second from first
matrix::Matrix<int> diff(matrix::Matrix<int>& first, matrix::Matrix<int>& second) {
    assert (first.cols == second.cols && first.rows == second.rows);
    std::vector<int> diffs(first.capacity());
    for (int i = 0; i < first.capacity(); ++i) {
        diffs.at(i) = first.data.at(i) - second.data.at(i);
    }
    return matrix::Matrix<int>(first.rows, first.cols, diffs);
}

u32 sumAbsValues(const std::vector<int>& v) {
    u32 sum = 0;
    for (auto i : v) {
        sum += std::abs(i);
    }
    return sum;
}

decode::CompressedPFrame getPFrame(YUVFrame420& inputFrame, YUVFrame420& previousFrame, dct::QualityLevel qualityLevel) {
    auto height = inputFrame.getIntPlaneY().rows;
    auto width = inputFrame.getIntPlaneY().cols;
    auto scaledHeight = height/2;
    auto scaledWidth = width/2;

    // Get the data from each plane in the new frame.
    auto yPlane = inputFrame.getIntPlaneY();
    auto cbPlane = inputFrame.getIntPlaneCb();
    auto crPlane = inputFrame.getIntPlaneCr();

    // Get the data from the previous frame.
    auto prevYPlane = previousFrame.getIntPlaneY();
    auto prevCbPlane = previousFrame.getIntPlaneCb();
    auto prevCrPlane = previousFrame.getIntPlaneCr();

    // todo
    //      - Consider randomly choosing which subsets of the blocks to compare to save time.
    //              - If we compare a small subset of cb, cr, and y chunks and get a good result
    //                that should give us high confidence.
    //  - search elsewhere
    //      - Only implement this if you have time
    //      - Only resort to this if we can't find a "good" match nearby.
    //      - Consider a random fade-away


    // Divide the image into macroblocks and record the headers.
    std::vector<decode::MacroblockHeader> macroblockHeaders;
    for (u32 row = 0; row < scaledHeight; row += 8) {
        for (u32 col = 0; col < scaledWidth; col += 8) {
            auto numRows = std::min(8u, scaledHeight - row);
            auto numCols = std::min(8u, scaledWidth - col);
            auto cbBlock = cbPlane.getBlock(row, col, numRows, numCols);
            auto crBlock = crPlane.getBlock(row, col, numRows, numCols);
            auto yBlock = yPlane.getBlock(row*2, col*2, numRows*2, numCols*2);

            // Search for the best match in a local area.
            u32 bestX = -1, bestY = -1, bestAAD = -1;
            std::vector<std::vector<int>> bestDiffs;
            int returnThreshold = 5; // Return early if we get a really close match.
            auto localSearchCoordinates = getLocalSearchCoordinates(row, col, scaledHeight, scaledWidth);
            for (auto coord : localSearchCoordinates) {
                auto x = coord.first;
                auto y = coord.second;

                // Pull out the specified blocks of the previous image
                auto prevCbBlock = prevCbPlane.getBlock(y, x, numRows, numCols);
                auto prevCrBlock = prevCrPlane.getBlock(y, x, numRows, numCols);
                auto prevYBlock = prevYPlane.getBlock(y*2, x*2, numRows*2, numCols*2);

                // Compute diffs
                auto yDiffs = diff(yBlock, prevYBlock);
                auto cbDiffs = diff(cbBlock, prevCbBlock);
                auto crDiffs = diff(crBlock, prevCrBlock);

                // Determine if the diffs found are actually good
                auto totalCount = yDiffs.capacity() + cbDiffs.capacity() + crDiffs.capacity();
                auto totalSum = sumAbsValues(yDiffs.data) + sumAbsValues(cbDiffs.data) + sumAbsValues(crDiffs.data);
                auto averageAbsoluteDifference = totalSum / totalCount;

                if (averageAbsoluteDifference < bestAAD) {
                    bestAAD = averageAbsoluteDifference;
                    bestX = x;
                    bestY = y;
                    bestDiffs = {yDiffs.data, cbDiffs.data, crDiffs.data};
                    if (bestAAD < returnThreshold) {
                        break; // If we get a really close match then kick-out early and use it.
                    }
                }
            }

            // If the average size of the diffs given by the match found isn't significantly smaller
            // than the original data, then don't predict the block.
            u32 totalSum = sumAbsValues(yPlane.data) + sumAbsValues(cbPlane.data) + sumAbsValues(crPlane.data);
            u32 totalCount = yPlane.capacity() + cbPlane.capacity() + crPlane.capacity();
            u32 minimumPredictionThreshold = (totalSum/totalCount)/2;
            if (bestAAD < minimumPredictionThreshold) {
                // Replace the original data for the macroblock with the pre-computed diffs.
                matrix::Matrix<int> yDiff(numRows*2, numCols*2, bestDiffs.at(0));
                yPlane.insertBlock(row*2, col*2, yDiff);
                matrix::Matrix<int> cbDiff(numRows, numCols, bestDiffs.at(1));
                cbPlane.insertBlock(row, col, cbDiff);
                matrix::Matrix<int> crDiff(numRows, numCols, bestDiffs.at(2));
                crPlane.insertBlock(row, col, crDiff);

                // Compute the vector from the block being predicted to the block in the previous frame
                // and store the header.
                int motionVectorX = (int)bestX - (int)col;
                int motionVectorY = (int)bestY - (int)row;
                macroblockHeaders.push_back({true, motionVectorX, motionVectorY});
            }
            else {
                // Use the original data for the macroblock.
                macroblockHeaders.push_back({false, 0, 0});
            }
        }
    }

    // Run the DCT on the (partially) predicted planes.
    auto encodedYPlane = dct::transform(yPlane, dct::quantize::luminance(), qualityLevel);
    auto encodedCbPlane = dct::transform(cbPlane, dct::quantize::chromanance(), qualityLevel);
    auto encodedCrPlane = dct::transform(crPlane, dct::quantize::chromanance(), qualityLevel);

    return decode::CompressedPFrame(height, width, encodedYPlane, encodedCbPlane, encodedCrPlane, macroblockHeaders);
}

void pushMacroblock(OutputBitStream& outputBitStream, decode::MacroblockHeader header) {
    if (header.predicted) {
        // Signal that the macroblock is predicted
        outputBitStream.push_bit(1);

        // Push a variable-bit encoding for the motion vector components
        auto encodedX = delta::encode(header.motionVectorX);
        auto encodedY = delta::encode(header.motionVectorY);
        outputBitStream.push_bits_msb_first(encodedX);
        outputBitStream.push_bits_msb_first(encodedY);
    }
    else {
        // Signal the the macroblock is not predicted
        outputBitStream.push_bit(0);
    }
}

void pushPFrame(OutputBitStream& outputBitStream, decode::CompressedPFrame& pFrame) {
    // Push the header for each macroblock first, then push the planes
    for (const auto& header : pFrame.macroblockHeaders) {
        pushMacroblock(outputBitStream, header);
    }

    writeBlocks(pFrame.y, outputBitStream);
    writeBlocks(pFrame.cb, outputBitStream);
    writeBlocks(pFrame.cr, outputBitStream);
}

void compress(u32 width, u32 height, dct::QualityLevel qualityLevel, YUVStreamReader& reader, OutputBitStream& outputBitStream){
    // Push metadata
    outputBitStream.push_u32(height);
    outputBitStream.push_u32(width);
    sendQualityLevel(qualityLevel, outputBitStream);

    // Read the first frame and encode it as an I-Frame
    assert (reader.read_next_frame());
    outputBitStream.push_bit(1); // Continuation flag
    auto iFrame = getIFrame(reader.frame(), qualityLevel);
    pushIFrame(outputBitStream, iFrame);

    // Continually store the __decoded__ version of each frame in order to
    // generate P-Frames
    auto previous = decode::IFrameToYCbCr(iFrame, qualityLevel);

    // Read each frame of video, encode it, and push the encoding.
    while (reader.read_next_frame()){
        // Push a one bit flag to indicate there is another frame of video
        outputBitStream.push_bit(1);

        // Read in the next frame, encode it, and push the encoding
        YUVFrame420& nextFrame = reader.frame();
        auto pFrame = getPFrame(nextFrame, previous, qualityLevel);
        pushPFrame(outputBitStream, pFrame);

        // Cache the decoded version of the last frame for future P-Frames
        auto decoded = decode::PFrameToYCbCr(pFrame, previous, qualityLevel);
        previous = decoded;
    }

    outputBitStream.push_bit(0); // Flag to indicate end of data
    outputBitStream.flush_to_byte();
}

//int NOT_MAIN(int argc, char** argv) { // todo remove this comment
int main(int argc, char** argv) {
    if (argc < 4){
        std::cerr << "Usage: " << argv[0] << " <width> <height> <low/medium/high>" << std::endl;
        return 1;
    }
    u32 width = std::stoi(argv[1]);
    u32 height = std::stoi(argv[2]);
    dct::QualityLevel qualityLevel = getQualityLevel(argv[3]);

    YUVStreamReader reader {std::cin, width, height};
    OutputBitStream outputBitStream {std::cout};

    compress(width, height, qualityLevel, reader, outputBitStream);

    srand(time(NULL)); // todo remove this unless needed.

    return 0;
}


// todo remove  dummy main() for debugging
//int main(int argc, char** argv) {
int dummy(int argc, char** argv) {
    u32 width = 352, height = 288;
    auto qualityLevel = dct::med;

    auto filepath = "/home/jamie/csc485/CSC485B-Data-Compression/uvid/raw_videos/flower_352x288.raw";
    std::cerr << "DUMMY MAIN RUNNING. Using hard-coded input file:\n\t" << filepath << std::endl;
    std::fstream infile(filepath);

    YUVStreamReader reader{infile, width, height};
    OutputBitStream outputBitStream{std::cout};

    compress(width, height, qualityLevel, reader, outputBitStream);

    return 0;
}
