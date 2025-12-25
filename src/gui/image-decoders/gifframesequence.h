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
#ifndef __FRAMESQUENCE_GIF_H__
#define __FRAMESQUENCE_GIF_H__

#include "image-decoders/framesequence.h"
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
    static constexpr uint32_t GIF_HEADER_SIZE = 6;/*GIF87a or GIF89a*/
public:
    GifFrameSequence(std::istream&);
    ~GifFrameSequence()override;

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
    static  bool isGIF(const uint8_t*,uint32_t);
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
    ~GifFrameSequenceState()override;
    // returns frame's delay time in ms
    long drawFrame(int frameNr, uint32_t* outputPtr, int outputPixelStride, int previousFrameNr)override;

};
}/*endof namespace*/
#endif //__FRAMESQUENCE_GIF_H__

