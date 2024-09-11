/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __FRAMESQUENCE_PNG_H__
#define __FRAMESQUENCE_PNG_H__
#include <vector>
#include <cairomm/surface.h>
#include <image-decoders/framesequence.h>
namespace cdroid{
typedef struct png_struct_def* png_structp;
typedef struct png_info_def* png_infop;

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
public:
    PngFrameSequence(std::istream* stream);
    virtual ~PngFrameSequence();

    int getWidth() const override;
    int getHeight() const override;
    bool isOpaque() const override;
    int getFrameCount() const override;
    int getDefaultLoopCount() const override{
        return mLoopCount;
    }

    virtual FrameSequenceState* createState() const override;

    uint32_t getBackgroundColor() const { return mBgColor; }
};

class PngFrameSequence::PngFrameSequenceState : public FrameSequenceState {
private:
    struct ApngFrame{
        uint32_t x;
        uint32_t y;
        uint32_t width;
        uint32_t height;
        uint32_t delay;
        uint8_t dop;
        uint8_t bop;
    };
    png_structp png_ptr;
    png_infop png_info;
    uint8_t*mBytesAt;
    int32_t mFrameIndex;
    uint8_t*mCurrBase;
    uint8_t*mBaseBuffer;
    uint8_t*mRenderBuffer;
    const PngFrameSequence& mFrameSequence;
    void resetPngIO();
    void blend2Render(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    void fill2Render(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    void copyArea2Base(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    void clearBaseArea(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
public:
    PngFrameSequenceState(const PngFrameSequence& frameSequence);
    virtual ~PngFrameSequenceState();
    // returns frame's delay time in ms
    virtual long drawFrame(int frameNr, uint32_t* outputPtr, int outputPixelStride, int previousFrameNr);
};

}/*endof namespace*/
#endif //__FRAMESQUENCE_PNG_H__

