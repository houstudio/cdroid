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

#ifndef __FRAMESQUENCE_WEBP_H__
#define __FRAMESQUENCE_WEBP_H__

#include <image-decoders/framesequence.h>
struct WebPDecoderConfig;
struct WebPIterator;
namespace cdroid{
// Parser for a possibly-animated WebP bitstream.
class WebPFrameSequence : public FrameSequence {
private:
    uint8_t* mDataBytes;
    size_t mDataSize;
    struct WebPDemuxer* mDemux;
    int mLoopCount;
    uint32_t mFormatFlags;
    // mIsKeyFrame[i] is true if ith canvas can be constructed without decoding any prior frames.
    bool* mIsKeyFrame;
    static Registry mWebp;
private:
    void constructDependencyChain();
    WebPFrameSequence(const WebPFrameSequence&)=default;
public:
    class WebPFrameSequenceState;
    static constexpr uint32_t RIFF_HEADER_SIZE = 12;/*Size of the RIFF header ("RIFFnnnnWEBP")*/
public:
    WebPFrameSequence(cdroid::Context*,const std::string&resid);
    virtual ~WebPFrameSequence();

    int getWidth() const override;
    int getHeight() const override;
    bool isOpaque() const override;
    int getFrameCount() const override;
    int getDefaultLoopCount() const override{
        return mLoopCount;
    }

    FrameSequenceState* createState() const override;
    WebPDemuxer* getDemuxer() const { return mDemux; }
    bool isKeyFrame(size_t frameNr) const { return mIsKeyFrame[frameNr]; }
    static bool isWEBP(const uint8_t*,uint32_t);
};

// Produces frames of a possibly-animated WebP file for display.
class WebPFrameSequence::WebPFrameSequenceState: public FrameSequenceState {
private:
    const WebPFrameSequence& mFrameSequence;
    WebPDecoderConfig* mDecoderConfig;
    uint32_t* mPreservedBuffer;
    void initializeFrame(const WebPIterator& currIter, uint32_t* currBuffer, int currStride,
            const WebPIterator& prevIter, const uint32_t* prevBuffer, int prevStride);
    bool decodeFrame(const WebPIterator& iter, uint32_t* currBuffer, int currStride,
            const WebPIterator& prevIter, const uint32_t* prevBuffer, int prevStride);
public:
    WebPFrameSequenceState(const WebPFrameSequence& frameSequence);
    virtual ~WebPFrameSequenceState();

    // Returns frame's delay time in milliseconds.
    virtual long drawFrame(int frameNr,uint32_t* outputPtr, int outputPixelStride, int previousFrameNr);

};
}/*endof namespace*/
#endif //__FRAMESQUENCE_WEBP_H__
