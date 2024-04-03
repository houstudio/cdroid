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

#ifndef __FRAMESQUENCE_GIF_H__
#define __FRAMESQUENCE_GIF_H__

#include "framesequence.h"
namespace cdroid{
class GifFrameSequence : public FrameSequence {
private:
    struct GifFileType* mGif;
    int mLoopCount;
    uint32_t mBgColor;
    // array of bool per frame - if true, frame data is used by a later DISPOSE_PREVIOUS frame
    bool* mPreservedFrames;
    // array of ints per frame - if >= 0, points to the index of the preserve that frame needs
    int* mRestoringFrames;
public:
    class GifFrameSequenceState;
public:
    GifFrameSequence(std::istream* stream);
    virtual ~GifFrameSequence();

    int getWidth() const override;
    int getHeight() const override;
    bool isOpaque() const override;
    int getFrameCount() const override;
    int getDefaultLoopCount() const override{
        return mLoopCount;
    }

    virtual FrameSequenceState* createState() const override;

    GifFileType* getGif() const { return mGif; }
    uint32_t getBackgroundColor() const { return mBgColor; }
    bool getPreservedFrame(int frameIndex) const { return mPreservedFrames[frameIndex]; }
    int getRestoringFrame(int frameIndex) const { return mRestoringFrames[frameIndex]; }
};

class GifFrameSequence::GifFrameSequenceState : public FrameSequenceState {
private:
    const GifFrameSequence& mFrameSequence;
    uint32_t* mPreserveBuffer;
    int mPreserveBufferFrame;
    void savePreserveBuffer(uint32_t* outputPtr, int outputPixelStride, int frameNr);
    void restorePreserveBuffer(uint32_t* outputPtr, int outputPixelStride);
public:
    GifFrameSequenceState(const GifFrameSequence& frameSequence);
    virtual ~GifFrameSequenceState();
    // returns frame's delay time in ms
    virtual long drawFrame(int frameNr, uint32_t* outputPtr, int outputPixelStride, int previousFrameNr);

};
}/*endof namespace*/
#endif //__FRAMESQUENCE_GIF_H__

