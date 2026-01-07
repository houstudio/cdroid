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
#include <cstring>
#include <porting/cdlog.h>
#include <gui_features.h>
#if ENABLE(WEBP)
#include <core/context.h>
#include <webp/decode.h>
#include <webp/demux.h>
#include <fstream>
#include <image-decoders/webpframesequence.h>

#define CHUNK_HEADER_SIZE  8     // Size of a chunk header.
#define TAG_SIZE           4     // Size of a chunk tag (e.g. "VP8L").
#define MAX_CHUNK_PAYLOAD (~0U - CHUNK_HEADER_SIZE - 1)
#define MKFOURCC(a, b, c, d) ((a) | (b) << 8 | (c) << 16 | (uint32_t)(d) << 24)
////////////////////////////////////////////////////////////////////////////////
// Frame sequence
////////////////////////////////////////////////////////////////////////////////
namespace cdroid{
static uint32_t GetLE32(const uint8_t* const data) {
    return MKFOURCC(data[0], data[1], data[2], data[3]);
}

// Returns true if the frame covers full canvas.
static bool isFullFrame(const WebPIterator& frame, int canvasWidth, int canvasHeight) {
    return (frame.width == canvasWidth && frame.height == canvasHeight);
}

// Returns true if the rectangle defined by 'frame' contains pixel (x, y).
static bool FrameContainsPixel(const WebPIterator& frame, int x, int y) {
    const int left = frame.x_offset;
    const int right = left + frame.width;
    const int top = frame.y_offset;
    const int bottom = top + frame.height;
    return x >= left && x < right && y >= top && y < bottom;
}

// Construct mIsKeyFrame array.
void WebPFrameSequence::constructDependencyChain() {
    const int frameCount = getFrameCount();
    mIsKeyFrame = new bool[frameCount];
    const int canvasWidth = getWidth();
    const int canvasHeight = getHeight();

    WebPIterator prev , curr;

    // Note: WebPDemuxGetFrame() uses base-1 counting.
    int ok = WebPDemuxGetFrame(mDemux, 1, &curr);
    LOGE_IF(!ok, "Could not retrieve frame# 0");
    mIsKeyFrame[0] = true;  // 0th frame is always a key frame.
    for (int i = 1; i < frameCount; i++) {
        prev = curr;
        ok = WebPDemuxGetFrame(mDemux, i + 1, &curr);  // Get ith frame.
        LOGE_IF(!ok, "Could not retrieve frame# %d", i);

        if ((!curr.has_alpha || curr.blend_method == WEBP_MUX_NO_BLEND) &&
                isFullFrame(curr, canvasWidth, canvasHeight)) {
            mIsKeyFrame[i] = true;
        } else {
            mIsKeyFrame[i] = (prev.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND) &&
                    (isFullFrame(prev, canvasWidth, canvasHeight) || mIsKeyFrame[i - 1]);
        }
    }
    WebPDemuxReleaseIterator(&prev);
    WebPDemuxReleaseIterator(&curr);
    for(int i=0;i<frameCount;i++){
        LOGV("Frame# %zu: %s", i, mIsKeyFrame[i] ? "Key frame" : "NOT a key frame");
    }
}

WebPFrameSequence::WebPFrameSequence(std::istream&stream)
      :FrameSequence(stream),mDemux(NULL) , mIsKeyFrame(NULL) {
    // Read RIFF header to get file size.
    uint8_t riff_header[RIFF_HEADER_SIZE];

    if (mStream.read((char*)riff_header, RIFF_HEADER_SIZE).gcount() != RIFF_HEADER_SIZE) {
        LOGE("WebP header load failed");
        return;
    }
    uint32_t readSize = GetLE32(riff_header + TAG_SIZE);
    if (readSize > MAX_CHUNK_PAYLOAD) {
        LOGE("WebP got header size too large");
        return;
    }
    mDataSize = CHUNK_HEADER_SIZE + readSize;
    if(mDataSize < RIFF_HEADER_SIZE) {
        LOGE("WebP file malformed");
        return;
    }
    mDataBytes = new uint8_t[mDataSize];
    std::memcpy((void*)mDataBytes, riff_header, RIFF_HEADER_SIZE);

    // Read rest of the bytes.
    void* remaining_bytes = (void*)(mDataBytes + RIFF_HEADER_SIZE);
    size_t remaining_size = mDataSize - RIFF_HEADER_SIZE;
    if (mStream.read((char*)remaining_bytes, remaining_size).gcount() != remaining_size) {
        LOGE("WebP full load failed");
        return;
    }

    // Construct demux.
    WebPData data={mDataBytes,mDataSize};
    mDemux = WebPDemux(&data);
    if (!mDemux) {
        LOGE("Parsing of WebP container file failed");
        return;
    }
    mLoopCount = WebPDemuxGetI(mDemux, WEBP_FF_LOOP_COUNT);
    mFormatFlags = WebPDemuxGetI(mDemux, WEBP_FF_FORMAT_FLAGS);
    LOGD("WebpFrameSequence created with size = %dx%dx%d, flags = 0x%X",
          getWidth(), getHeight(), getFrameCount(), mFormatFlags);
    constructDependencyChain();
}

int WebPFrameSequence::getWidth() const {
    if (!mDemux) {
        return 0;
    }
    return WebPDemuxGetI(mDemux, WEBP_FF_CANVAS_WIDTH);
}

int WebPFrameSequence::getHeight() const {
    if (!mDemux) {
        return 0;
    }
    return WebPDemuxGetI(mDemux, WEBP_FF_CANVAS_HEIGHT);
}

bool WebPFrameSequence::isOpaque() const {
    return !(mFormatFlags & ALPHA_FLAG);
}

int WebPFrameSequence::getFrameCount() const {
    if (!mDemux) {
        return 0;
    }
    return WebPDemuxGetI(mDemux, WEBP_FF_FRAME_COUNT);
}

WebPFrameSequence::~WebPFrameSequence() {
    WebPDemuxDelete(mDemux);
    delete[] mIsKeyFrame;
    delete[] mDataBytes;
}

FrameSequenceState* WebPFrameSequence::createState() const {
    return new WebPFrameSequenceState(*this);
}

////////////////////////////////////////////////////////////////////////////////
// draw helpers
////////////////////////////////////////////////////////////////////////////////

static bool willBeCleared(const WebPIterator& iter) {
    return iter.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND;
}

// return true if area of 'target' completely covers area of 'covered'
static bool checkIfCover(const WebPIterator& target, const WebPIterator& covered) {
    const int covered_x_max = covered.x_offset + covered.width;
    const int target_x_max = target.x_offset + target.width;
    const int covered_y_max = covered.y_offset + covered.height;
    const int target_y_max = target.y_offset + target.height;
    return target.x_offset <= covered.x_offset
           && covered_x_max <= target_x_max
           && target.y_offset <= covered.y_offset
           && covered_y_max <= target_y_max;
}

// Clear all pixels in a line to transparent.
static void clearLine(uint32_t* dst, int width) {
    std::memset(dst, 0, width * sizeof(*dst));  // Note: Assumes TRANSPARENT == 0x0.
}

// Copy all pixels from 'src' to 'dst'.
static void copyFrame(const uint32_t* src, int srcStride, uint32_t* dst, int dstStride,
        int width, int height) {
    for (int y = 0; y < height; y++) {
        std::memcpy(dst, src, width * sizeof(*dst));
        src += srcStride;
        dst += dstStride;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Frame sequence state
////////////////////////////////////////////////////////////////////////////////

WebPFrameSequence::WebPFrameSequenceState::WebPFrameSequenceState(const WebPFrameSequence& frameSequence) :
        mFrameSequence(frameSequence) {
    mDecoderConfig = new WebPDecoderConfig;
    WebPInitDecoderConfig(mDecoderConfig);
    mDecoderConfig->options.use_threads = 1;
    mDecoderConfig->output.is_external_memory = 1;
    mDecoderConfig->output.colorspace = MODE_bgrA;//Pre-multiplied alpha mode.
    const int canvasWidth = mFrameSequence.getWidth();
    const int canvasHeight = mFrameSequence.getHeight();
    mPreservedBuffer = new uint32_t[canvasWidth * canvasHeight];
}

WebPFrameSequence::WebPFrameSequenceState::~WebPFrameSequenceState() {
    delete[] mPreservedBuffer;
    delete mDecoderConfig;
}

void WebPFrameSequence::WebPFrameSequenceState::initializeFrame(const WebPIterator& currIter, uint32_t* currBuffer,
        int currStride, const WebPIterator& prevIter, const uint32_t* prevBuffer, int prevStride) {
    const int canvasWidth = mFrameSequence.getWidth();
    const int canvasHeight = mFrameSequence.getHeight();
    const bool currFrameIsKeyFrame = mFrameSequence.isKeyFrame(currIter.frame_num - 1);

    if (currFrameIsKeyFrame) {  // Clear canvas.
        for (int y = 0; y < canvasHeight; y++) {
            uint32_t* dst = currBuffer + y * currStride;
            clearLine(dst, canvasWidth);
        }
    } else {
        // Preserve previous frame as starting state of current frame.
        copyFrame(prevBuffer, prevStride, currBuffer, currStride, canvasWidth, canvasHeight);

        // Dispose previous frame rectangle to Background if needed.
        bool prevFrameCompletelyCovered =
                (!currIter.has_alpha || currIter.blend_method == WEBP_MUX_NO_BLEND) &&
                checkIfCover(currIter, prevIter);
        if ((prevIter.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND) &&
                !prevFrameCompletelyCovered) {
            uint32_t* dst = currBuffer + prevIter.x_offset + prevIter.y_offset * currStride;
            for (int j = 0; j < prevIter.height; j++) {
                clearLine(dst, prevIter.width);
                dst += currStride;
            }
        }
    }
}

bool WebPFrameSequence::WebPFrameSequenceState::decodeFrame(const WebPIterator& currIter, uint32_t* currBuffer,
        int currStride, const WebPIterator& prevIter, const uint32_t* prevBuffer, int prevStride) {
    uint32_t* dst = currBuffer + currIter.x_offset + currIter.y_offset * currStride;
    mDecoderConfig->output.u.RGBA.rgba = (uint8_t*)dst;
    mDecoderConfig->output.u.RGBA.stride = currStride * 4;
    mDecoderConfig->output.u.RGBA.size = mDecoderConfig->output.u.RGBA.stride * currIter.height;

    const WebPData& currFrame = currIter.fragment;
    if (WebPDecode(currFrame.bytes, currFrame.size, mDecoderConfig) != VP8_STATUS_OK) {
        return false;
    }

    //const int canvasWidth = mFrameSequence.getWidth();
    //const int canvasHeight = mFrameSequence.getHeight();
    const bool currFrameIsKeyFrame = mFrameSequence.isKeyFrame(currIter.frame_num - 1);
    // During the decoding of current frame, we may have set some pixels to be transparent
    // (i.e. alpha < 255). However, the value of each of these pixels should have been determined
    // by blending it against the value of that pixel in the previous frame if WEBP_MUX_BLEND was
    // specified. So, we correct these pixels based on disposal method of the previous frame and
    // the previous frame buffer.
    if (currIter.blend_method == WEBP_MUX_BLEND && !currFrameIsKeyFrame) {
        if (prevIter.dispose_method == WEBP_MUX_DISPOSE_NONE) {
            for (int y = 0; y < currIter.height; y++) {
                const int canvasY = currIter.y_offset + y;
                for (int x = 0; x < currIter.width; x++) {
                    const int canvasX = currIter.x_offset + x;
                    uint32_t& currPixel = currBuffer[canvasY * currStride + canvasX];
                    // FIXME: Use alpha-blending when alpha is between 0 and 255.
                    if (!(currPixel & COLOR_8888_ALPHA_MASK)) {
                        const uint32_t prevPixel = prevBuffer[canvasY * prevStride + canvasX];
                        currPixel = prevPixel;
                    }
                }
            }
        } else {  // prevIter.dispose_method == WEBP_MUX_DISPOSE_BACKGROUND
            // Need to restore transparent pixels to as they were just after frame initialization.
            // That is:
            //   * Transparent if it belongs to previous frame rectangle <-- This is a no-op.
            //   * Pixel in the previous canvas otherwise <-- Need to restore.
            for (int y = 0; y < currIter.height; y++) {
                const int canvasY = currIter.y_offset + y;
                for (int x = 0; x < currIter.width; x++) {
                    const int canvasX = currIter.x_offset + x;
                    uint32_t& currPixel = currBuffer[canvasY * currStride + canvasX];
                    // FIXME: Use alpha-blending when alpha is between 0 and 255.
                    if (!(currPixel & COLOR_8888_ALPHA_MASK)
                            && !FrameContainsPixel(prevIter, canvasX, canvasY)) {
                        const uint32_t prevPixel = prevBuffer[canvasY * prevStride + canvasX];
                        currPixel = prevPixel;
                    }
                }
            }
        }
    }
    return true;
}

long WebPFrameSequence::WebPFrameSequenceState::drawFrame(int frameNr,
        uint32_t* outputPtr, int outputPixelStride, int previousFrameNr) {
    WebPDemuxer* demux = mFrameSequence.getDemuxer();
    LOGE_IF(!demux, "Cannot drawFrame, mDemux is NULL");

    LOGV("drawFrame called for frame# %d, previous frame# %d", frameNr, previousFrameNr);

    const int canvasWidth = mFrameSequence.getWidth();
    const int canvasHeight = mFrameSequence.getHeight();

    // Find the first frame to be decoded.
    int start = std::max(previousFrameNr + 1, 0);
    int earliestRequired = frameNr;
    while (earliestRequired > start) {
        if (mFrameSequence.isKeyFrame(earliestRequired)) {
            start = earliestRequired;
            break;
        }
        earliestRequired--;
    }

    WebPIterator currIter;
    WebPIterator prevIter;
    int ok = WebPDemuxGetFrame(demux, start, &currIter);  // Get frame number 'start - 1'.
    LOGE_IF(!ok, "Could not retrieve frame# %d", start - 1);

    // Use preserve buffer only if needed.
    uint32_t* prevBuffer = (frameNr == 0) ? outputPtr : mPreservedBuffer;
    int prevStride = (frameNr == 0) ? outputPixelStride : canvasWidth;
    uint32_t* currBuffer = outputPtr;
    int currStride = outputPixelStride;

    for (int i = start; i <= frameNr; i++) {
        prevIter = currIter;
        ok = WebPDemuxGetFrame(demux, i + 1, &currIter);  // Get ith frame.
        LOGE_IF(!ok, "Could not retrieve frame# %d", i);
        LOGV("producing frame %d (has_alpha = %d, dispose = %s, blend = %s, duration = %d)", i ,
              currIter.has_alpha, (currIter.dispose_method == WEBP_MUX_DISPOSE_NONE) ? "none" : "background",
              (currIter.blend_method == WEBP_MUX_BLEND) ? "yes" : "no", currIter.duration);
        // We swap the prev/curr buffers as we go.
        uint32_t* tmpBuffer = prevBuffer;
        prevBuffer = currBuffer;
        currBuffer = tmpBuffer;

        int tmpStride = prevStride;
        prevStride = currStride;
        currStride = tmpStride;

        LOGV("prev = %p, curr = %p, out = %p, tmp = %p", prevBuffer, currBuffer, outputPtr, mPreservedBuffer);
        // Process this frame.
        initializeFrame(currIter, currBuffer, currStride, prevIter, prevBuffer, prevStride);

        if (i == frameNr || !willBeCleared(currIter)) {
            if (!decodeFrame(currIter, currBuffer, currStride, prevIter, prevBuffer, prevStride)) {
                LOGE("Error decoding frame# %d", i);
                return -1;
            }
        }
    }

    if (outputPtr != currBuffer) {
        copyFrame(currBuffer, currStride, outputPtr, outputPixelStride, canvasWidth, canvasHeight);
    }

    // Return last frame's delay.
    const int frameCount = mFrameSequence.getFrameCount();
    const int lastFrame = (frameNr + frameCount - 1) % frameCount;
    ok = WebPDemuxGetFrame(demux, lastFrame + 1, &currIter);
    LOGE_IF(!ok, "Could not retrieve frame# %d", lastFrame);
    const int lastFrameDelay = currIter.duration;

    WebPDemuxReleaseIterator(&currIter);
    WebPDemuxReleaseIterator(&prevIter);

    return lastFrameDelay;
}

////////////////////////////////////////////////////////////////////////////////
// Registry
////////////////////////////////////////////////////////////////////////////////

bool WebPFrameSequence::isWEBP(const uint8_t* header,uint32_t header_size) {
    const uint8_t* const header_str = (const uint8_t*)header;
    return (header_size >= RIFF_HEADER_SIZE) &&
            !std::memcmp("RIFF", header_str, 4) &&
            !std::memcmp("WEBP", header_str + 8, 4);
}

}/*endof namespace*/
#endif/*ENABLE_WEBP*/
