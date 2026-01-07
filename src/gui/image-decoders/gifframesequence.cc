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
#include <gui/gui_features.h>
#if ENABLE(GIF)
#include <gif_lib.h>
#include <image-decoders/gifframesequence.h>

namespace cdroid{

#define ARGB_TO_COLOR8888(a, r, g, b) (uint32_t(a) << 24 | uint32_t(r) << 16 | uint32_t(g) << 8 | (b))

static int streamReader(GifFileType* fileType, GifByteType* out, int size) {
    std::istream* stream = (std::istream*) fileType->UserData;
    return stream->read((char*)out, size).gcount();
}

static uint32_t gifColorToColor8888(const GifColorType& color) {
    return ARGB_TO_COLOR8888(0xff, color.Red, color.Green, color.Blue);
}

static bool willBeCleared(const GraphicsControlBlock& gcb) {
    return gcb.DisposalMode == DISPOSE_BACKGROUND || gcb.DisposalMode == DISPOSE_PREVIOUS;
}

////////////////////////////////////////////////////////////////////////////////
// Frame sequence
////////////////////////////////////////////////////////////////////////////////

GifFrameSequence::GifFrameSequence(std::istream&stream)
      :FrameSequence(stream), mLoopCount(1), mBgColor(COLOR_TRANSPARENT),
      mPreservedFrames(NULL), mRestoringFrames(NULL) {
    mGif = DGifOpen(&mStream, streamReader, NULL);
    if (!mGif) {
        LOGW("Gif load failed");
        return;
    }

    if (DGifSlurp(mGif) != GIF_OK) {
        LOGW("Gif slurp failed");
        DGifCloseFile(mGif, NULL);
        mGif = NULL;
        return;
    }

    long durationMs = 0;
    int lastUnclearedFrame = -1;
    mPreservedFrames = new bool[mGif->ImageCount];
    mRestoringFrames = new int[mGif->ImageCount];

    GraphicsControlBlock gcb;
    for (int i = 0; i < mGif->ImageCount; i++) {
        const SavedImage& image = mGif->SavedImages[i];

        // find the loop extension pair
        for (int j = 0; (j + 1) < image.ExtensionBlockCount; j++) {
            ExtensionBlock* eb1 = image.ExtensionBlocks + j;
            ExtensionBlock* eb2 = image.ExtensionBlocks + j + 1;
            if (eb1->Function == APPLICATION_EXT_FUNC_CODE
                    // look for "NETSCAPE2.0" app extension
                    && eb1->ByteCount == 11
                    && !memcmp((const char*)(eb1->Bytes), "NETSCAPE2.0", 11)
                    // verify extension contents and get loop count
                    && eb2->Function == CONTINUE_EXT_FUNC_CODE
                    && eb2->ByteCount == 3 && eb2->Bytes[0] == 1) {
                mLoopCount = (int)(eb2->Bytes[2] << 8) + (int)(eb2->Bytes[1]);
            }
        }

        DGifSavedExtensionToGCB(mGif, i, &gcb);

        // timing
        durationMs += gcb.DelayTime * 10;//getDelayMs(gcb);

        // preserve logic
        mPreservedFrames[i] = false;
        mRestoringFrames[i] = -1;
        if (gcb.DisposalMode == DISPOSE_PREVIOUS && lastUnclearedFrame >= 0) {
            mPreservedFrames[lastUnclearedFrame] = true;
            mRestoringFrames[i] = lastUnclearedFrame;
        }
        if (!willBeCleared(gcb)) {
            lastUnclearedFrame = i;
        }
    }

    LOGV("GifFrameSequence created with size %d %d, frames %d dur %ld",
            mGif->SWidth, mGif->SHeight, mGif->ImageCount, durationMs);
    for (int i = 0; i < mGif->ImageCount; i++) {
        DGifSavedExtensionToGCB(mGif, i, &gcb);
        LOGV("Frame %d - must preserve %d, restore point %d, trans color %d",
                i, mPreservedFrames[i], mRestoringFrames[i], gcb.TransparentColor);
    }

    if (mGif->SColorMap) {
        // calculate bg color
        GraphicsControlBlock gcb;
        DGifSavedExtensionToGCB(mGif, 0, &gcb);
        if (gcb.TransparentColor == NO_TRANSPARENT_COLOR) {
            mBgColor = gifColorToColor8888(mGif->SColorMap->Colors[mGif->SBackGroundColor]);
        }
    }
}

GifFrameSequence::~GifFrameSequence() {
    if (mGif) {
        DGifCloseFile(mGif, NULL);
    }
    delete[] mPreservedFrames;
    delete[] mRestoringFrames;
}

int GifFrameSequence::getWidth() const {
    return mGif ? mGif->SWidth : 0;
}

int GifFrameSequence::getHeight() const {
    return mGif ? mGif->SHeight : 0;
}

bool GifFrameSequence::isOpaque() const {
    return (mBgColor & COLOR_8888_ALPHA_MASK) == COLOR_8888_ALPHA_MASK;
}

int GifFrameSequence::getFrameCount() const {
    return mGif ? mGif->ImageCount : 0;
}

FrameSequenceState* GifFrameSequence::createState() const {
    return new GifFrameSequenceState(*this);
}

////////////////////////////////////////////////////////////////////////////////
// draw helpers
////////////////////////////////////////////////////////////////////////////////

// return true if area of 'target' is completely covers area of 'covered'
static bool checkIfCover(const GifImageDesc& target, const GifImageDesc& covered) {
    return target.Left <= covered.Left
            && covered.Left + covered.Width <= target.Left + target.Width
            && target.Top <= covered.Top
            && covered.Top + covered.Height <= target.Top + target.Height;
}

static void copyLine(uint32_t* dst, const unsigned char* src, const ColorMapObject* cmap,
                     int transparent, int width) {
    for (; width > 0; width--, src++, dst++) {
        if (*src != transparent && *src < cmap->ColorCount) {
            *dst = gifColorToColor8888(cmap->Colors[*src]);
        }
    }
}

static void setLineColor(uint32_t* dst, uint32_t color, int width) {
    for (; width > 0; width--, dst++) {
        *dst = color;
    }
}

static void getCopySize(const GifImageDesc& imageDesc, int maxWidth, int maxHeight,
        GifWord& copyWidth, GifWord& copyHeight) {
    copyWidth = imageDesc.Width;
    if (imageDesc.Left + copyWidth > maxWidth) {
        copyWidth = maxWidth - imageDesc.Left;
    }
    copyHeight = imageDesc.Height;
    if (imageDesc.Top + copyHeight > maxHeight) {
        copyHeight = maxHeight - imageDesc.Top;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Frame sequence state
////////////////////////////////////////////////////////////////////////////////

GifFrameSequence::GifFrameSequenceState::GifFrameSequenceState(const GifFrameSequence& frameSequence) :
    mFrameSequence(frameSequence), mPreserveBuffer(NULL), mPreserveBufferFrame(-1) {
}

GifFrameSequence::GifFrameSequenceState::~GifFrameSequenceState() {
       delete[] mPreserveBuffer;
}

void GifFrameSequence::GifFrameSequenceState::savePreserveBuffer(uint32_t* outputPtr, int outputPixelStride, int frameNr) {
    if (frameNr == mPreserveBufferFrame) return;

    mPreserveBufferFrame = frameNr;
    const int width = mFrameSequence.getWidth();
    const int height = mFrameSequence.getHeight();
    if (!mPreserveBuffer) {
        mPreserveBuffer = new uint32_t[width * height];
    }
    for (int y = 0; y < height; y++) {
        std::memcpy(mPreserveBuffer + width * y,
                outputPtr + outputPixelStride * y,
                width * 4);
    }
}

void GifFrameSequence::GifFrameSequenceState::restorePreserveBuffer(uint32_t* outputPtr, int outputPixelStride) {
    const int width = mFrameSequence.getWidth();
    const int height = mFrameSequence.getHeight();
    if (!mPreserveBuffer) {
        LOGD("preserve buffer not allocated! ah!");
        return;
    }
    for (int y = 0; y < height; y++) {
        std::memcpy(outputPtr + outputPixelStride * y,
                mPreserveBuffer + width * y,
                width * 4);
    }
}

long GifFrameSequence::GifFrameSequenceState::drawFrame(int frameNr,
        uint32_t* outputPtr, int outputPixelStride, int previousFrameNr) {

    GifFileType* gif = mFrameSequence.getGif();
    if (!gif) {
        LOGD("Cannot drawFrame, mGif is NULL");
        return -1;
    }

    LOGV("drawFrame on %p nr %d on addr %p, previous frame nr %d",this, frameNr, outputPtr, previousFrameNr);

    const int height = mFrameSequence.getHeight();
    const int width = mFrameSequence.getWidth();

    GraphicsControlBlock gcb;

    int start = std::max(previousFrameNr + 1, 0);

    for (int i = std::max(start - 1, 0); i < frameNr; i++) {
        int neededPreservedFrame = mFrameSequence.getRestoringFrame(i);
        if (neededPreservedFrame >= 0 && (mPreserveBufferFrame != neededPreservedFrame)) {
            LOGV("frame %d needs frame %d preserved, but %d is currently, so drawing from scratch",
                    i, neededPreservedFrame, mPreserveBufferFrame);
            start = 0;
        }
    }

    for (int i = start; i <= frameNr; i++) {
        DGifSavedExtensionToGCB(gif, i, &gcb);
        const SavedImage& frame = gif->SavedImages[i];

        bool frameOpaque = gcb.TransparentColor == NO_TRANSPARENT_COLOR;
        LOGV("producing frame %d, drawing frame %d (opaque %d, disp %d, del %d)",
                frameNr, i, frameOpaque, gcb.DisposalMode, gcb.DelayTime);
        if (i == 0) {
            //clear bitmap
            const uint32_t bgColor = mFrameSequence.getBackgroundColor();
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    outputPtr[y * outputPixelStride + x] = bgColor;
                }
            }
        } else {
            GraphicsControlBlock prevGcb;
            DGifSavedExtensionToGCB(gif, i - 1, &prevGcb);
            const SavedImage& prevFrame = gif->SavedImages[i - 1];
            bool prevFrameDisposed = willBeCleared(prevGcb);

            bool newFrameOpaque = gcb.TransparentColor == NO_TRANSPARENT_COLOR;
            bool prevFrameCompletelyCovered = newFrameOpaque
                    && checkIfCover(frame.ImageDesc, prevFrame.ImageDesc);

            if (prevFrameDisposed && !prevFrameCompletelyCovered) {
                switch (prevGcb.DisposalMode) {
                case DISPOSE_BACKGROUND: {
                    uint32_t* dst = outputPtr + prevFrame.ImageDesc.Left +
                            prevFrame.ImageDesc.Top * outputPixelStride;

                    GifWord copyWidth, copyHeight;
                    getCopySize(prevFrame.ImageDesc, width, height, copyWidth, copyHeight);
                    for (; copyHeight > 0; copyHeight--) {
                        setLineColor(dst, COLOR_TRANSPARENT, copyWidth);
                        dst += outputPixelStride;
                    }
                } break;
                case DISPOSE_PREVIOUS: {
                    restorePreserveBuffer(outputPtr, outputPixelStride);
                } break;
                }
            }

            if (mFrameSequence.getPreservedFrame(i - 1)) {
                // currently drawn frame will be restored by a following DISPOSE_PREVIOUS draw, so
                // we preserve it
                savePreserveBuffer(outputPtr, outputPixelStride, i - 1);
            }
        }

        bool willBeCleared = gcb.DisposalMode == DISPOSE_BACKGROUND
                || gcb.DisposalMode == DISPOSE_PREVIOUS;
        if (i == frameNr || !willBeCleared) {
            const ColorMapObject* cmap = gif->SColorMap;
            if (frame.ImageDesc.ColorMap) {
                cmap = frame.ImageDesc.ColorMap;
            }

            LOGW_IF((cmap == nullptr)||(cmap->ColorCount != (1 << cmap->BitsPerPixel)),
                "Warning: potentially corrupt color map");

            const unsigned char* src = (unsigned char*)frame.RasterBits;
            uint32_t* dst = outputPtr + frame.ImageDesc.Left +
                frame.ImageDesc.Top * outputPixelStride;
            GifWord copyWidth, copyHeight;
            getCopySize(frame.ImageDesc, width, height, copyWidth, copyHeight);
            for (; copyHeight > 0; copyHeight--) {
                copyLine(dst, src, cmap, gcb.TransparentColor, copyWidth);
                src += frame.ImageDesc.Width;
                dst += outputPixelStride;
            }
        }
    }

    // return last frame's delay
    const int maxFrame = gif->ImageCount;
    const int lastFrame = (frameNr + maxFrame - 1) % maxFrame;
    DGifSavedExtensionToGCB(gif, lastFrame, &gcb);
    return gcb.DelayTime * 10;//getDelayMs(gcb);
}

////////////////////////////////////////////////////////////////////////////////
// Registry
////////////////////////////////////////////////////////////////////////////////

bool GifFrameSequence::isGIF(const uint8_t* header,uint32_t header_size) {
    return !std::memcmp(GIF_STAMP, header, GIF_STAMP_LEN)
            || !std::memcmp(GIF87_STAMP, header, GIF_STAMP_LEN)
            || !std::memcmp(GIF89_STAMP, header, GIF_STAMP_LEN);
}

}/*endof namespace*/
#endif/*ENABLE_GIF*/
