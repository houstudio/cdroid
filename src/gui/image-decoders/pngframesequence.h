/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#ifndef __FRAMESQUENCE_PNG_H__
#define __FRAMESQUENCE_PNG_H__
#include <vector>
#include <cairomm/surface.h>
#include <image-decoders/imagedecoder.h>
#include <image-decoders/framesequence.h>
namespace cdroid{

class PngFrameSequence : public FrameSequence {
private:
    int32_t mImageWidth;
    int32_t mImageHeight;
    int32_t mFrameCount;
    uint32_t mDataSize;
    uint8_t *mDataBytes;
    int mLoopCount;
    uint32_t mBgColor;
public:
    class ApngFrame;
    class PngFrameSequenceState;
    friend PngFrameSequenceState;
    /*PNG_MAGIC "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A"*/
    static constexpr int PNG_HEADER_SIZE = 8;
public:
    PngFrameSequence(std::istream&);
    ~PngFrameSequence()override;

    int getWidth() const override;
    int getHeight() const override;
    bool isOpaque() const override;
    int getFrameCount() const override;
    int getDefaultLoopCount() const override{
        return mLoopCount;
    }

    virtual FrameSequenceState* createState() const override;
    uint32_t getBackgroundColor() const { return mBgColor; }
    static bool isPNG(const uint8_t*,uint32_t);
};

class PngFrameSequence::PngFrameSequenceState : public FrameSequenceState {
private:
    png_structp png_ptr;
    png_infop png_info;
    uint8_t*mBytesAt;
    int32_t mFrameIndex;
    uint8_t*mFrame;
    uint8_t*mBuffer;
    uint8_t*mPrevFrame;
    const PngFrameSequence& mFrameSequence;
    void resetPngIO();
public:
    PngFrameSequenceState(const PngFrameSequence& frameSequence);
    ~PngFrameSequenceState()override;
    // returns frame's delay time in ms
    long drawFrame(int frameNr, uint32_t* outputPtr, int outputPixelStride, int previousFrameNr)override;
};

}/*endof namespace*/
#endif //__FRAMESQUENCE_PNG_H__

