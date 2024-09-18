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
    PngFrameSequence(Context*,const std::string&);
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
    virtual ~PngFrameSequenceState();
    // returns frame's delay time in ms
    virtual long drawFrame(int frameNr, uint32_t* outputPtr, int outputPixelStride, int previousFrameNr);
};

}/*endof namespace*/
#endif //__FRAMESQUENCE_PNG_H__

