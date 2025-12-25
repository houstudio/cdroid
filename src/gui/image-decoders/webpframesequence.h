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
    WebPFrameSequence(std::istream&);
    ~WebPFrameSequence()override;

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
    ~WebPFrameSequenceState()override;

    // Returns frame's delay time in milliseconds.
    long drawFrame(int frameNr,uint32_t* outputPtr, int outputPixelStride, int previousFrameNr)override;

};
}/*endof namespace*/
#endif //__FRAMESQUENCE_WEBP_H__
