/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "minikin/Measurement.h"

#include <cfloat>
#include <cmath>

#include "BidiUtils.h"
#include "LayoutSplitter.h"
#include "minikin/BoundsCache.h"
#include "minikin/GraphemeBreak.h"

namespace {
bool isAsciiOrBidiControlCharacter(uint16_t c) {
    return (0x0000 <= c && c <= 0x001F)                  // ASCII control characters
           || c == 0x061C || c == 0x200E || c == 0x200F  // BiDi control characters
           || (0x202A <= c && c <= 0x202E) || (0x2066 <= c && c <= 0x2069);
}

}  // namespace

namespace minikin {

// These could be considered helper methods of layout, but need only be loosely coupled, so
// are separate.

/**
 * Return the unsigned advance of the given offset from the run start.
 *
 * @param advances the computed advances of the characters in buf. The advance of
 * the i-th character in buf is stored at index (i - layoutStart) in this array.
 * @param buf the text stored in utf-16 format.
 * @param layoutStart the start index of the character that is laid out.
 * @param start the start index of the run.
 * @param count the number of the characters in this run.
 * @param offset the target offset to compute the index. It should be in the
 * range of [start, start + count).
 * @return the unsigned advance from the run start to the given offset.
 */
static float getRunAdvance(const float* advances, const uint16_t* buf, size_t layoutStart,
                           size_t start, size_t count, size_t offset) {
    float advance = 0.0f;
    size_t lastCluster = start;
    float clusterWidth = 0.0f;
    for (size_t i = start; i < offset; i++) {
        float charAdvance = advances[i - layoutStart];
        if (charAdvance != 0.0f) {
            advance += charAdvance;
            lastCluster = i;
            clusterWidth = charAdvance;
        }
    }
    if (offset < start + count && !isAsciiOrBidiControlCharacter(buf[offset]) &&
        advances[offset - layoutStart] == 0.0f) {
        // In the middle of a cluster, distribute width of cluster so that each grapheme cluster
        // gets an equal share.
        // TODO: get caret information out of font when that's available
        size_t nextCluster;
        for (nextCluster = offset + 1; nextCluster < start + count; nextCluster++) {
            if (advances[nextCluster - layoutStart] != 0.0f ||
                isAsciiOrBidiControlCharacter(buf[nextCluster])) {
                break;
            }
        }
        int numGraphemeClusters = 0;
        int numGraphemeClustersAfter = 0;
        for (size_t i = lastCluster; i < nextCluster; i++) {
            bool isAfter = i >= offset;
            if (GraphemeBreak::isGraphemeBreak(advances + (start - layoutStart), buf, start, count,
                                               i)) {
                numGraphemeClusters++;
                if (isAfter) {
                    numGraphemeClustersAfter++;
                }
            }
        }
        if (numGraphemeClusters > 0) {
            advance -= clusterWidth * numGraphemeClustersAfter / numGraphemeClusters;
        }
    }
    return advance;
}

/**
 * Helper method that distribute the advance to ligature characters.
 * When ligature is applied, the first character in the ligature is assigned with the entire width.
 * This method will evenly distribute the advance to each grapheme in the ligature.
 *
 * @param advances the computed advances of the characters in buf. The advance of
 * the i-th character in buf is stored at index (i - start) in this array. This
 * method will update this array so that advances is distributed evenly for
 * ligature characters.
 * @param buf the text stored in utf-16 format.
 * @param start the start index of the run.
 * @param count the number of the characters in this run.
 */
void distributeAdvances(float* advances, const uint16_t* buf, size_t start, size_t count) {
    size_t clusterStart = start;
    while (clusterStart < start + count) {
        float clusterAdvance = advances[clusterStart - start];
        size_t clusterEnd;
        for (clusterEnd = clusterStart + 1; clusterEnd < start + count; clusterEnd++) {
            if (advances[clusterEnd - start] != 0.0f ||
                isAsciiOrBidiControlCharacter(buf[clusterEnd])) {
                break;
            }
        }
        size_t numGraphemeClusters = 0;
        for (size_t i = clusterStart; i < clusterEnd; i++) {
            if (GraphemeBreak::isGraphemeBreak(advances, buf, start, count, i)) {
                numGraphemeClusters++;
            }
        }
        // When there are more than one grapheme in this cluster, ligature is applied.
        // And we will distribute the width to each grapheme.
        if (numGraphemeClusters > 1) {
            for (size_t i = clusterStart; i < clusterEnd; ++i) {
                if (GraphemeBreak::isGraphemeBreak(advances, buf, start, count, i)) {
                    // Only distribute the advance to the first character of the cluster.
                    advances[i - start] = clusterAdvance / numGraphemeClusters;
                }
            }
        }
        clusterStart = clusterEnd;
    }
}

float getRunAdvance(const float* advances, const uint16_t* buf, size_t start, size_t count,
                    size_t offset) {
    return getRunAdvance(advances, buf, start, start, count, offset);
}

/**
 * Essentially the inverse of getRunAdvance. Compute the value of offset for which the
 * measured caret comes closest to the provided advance param, and which is on a grapheme
 * cluster boundary.
 *
 * The actual implementation fast-forwards through clusters to get "close", then does a finer-grain
 * search within the cluster and grapheme breaks.
 */
size_t getOffsetForAdvance(const float* advances, const uint16_t* buf, size_t start, size_t count,
                           float advance) {
    float x = 0.0f, xLastClusterStart = 0.0f, xSearchStart = 0.0f;
    size_t lastClusterStart = start, searchStart = start;
    for (size_t i = start; i < start + count; i++) {
        if (GraphemeBreak::isGraphemeBreak(advances, buf, start, count, i)) {
            searchStart = lastClusterStart;
            xSearchStart = xLastClusterStart;
        }
        float width = advances[i - start];
        if (width != 0.0f) {
            lastClusterStart = i;
            xLastClusterStart = x;
            x += width;
            if (x > advance) {
                break;
            }
        }
    }
    size_t best = searchStart;
    float bestDist = FLT_MAX;
    for (size_t i = searchStart; i <= start + count; i++) {
        if (GraphemeBreak::isGraphemeBreak(advances, buf, start, count, i)) {
            // "getRunAdvance(layout, buf, start, count, i) - advance" but more efficient
            float delta = getRunAdvance(advances, buf, start, searchStart, count - searchStart, i)

                          + xSearchStart - advance;
            if (std::abs(delta) < bestDist) {
                bestDist = std::abs(delta);
                best = i;
            }
            if (delta >= 0.0f) {
                break;
            }
        }
    }
    return best;
}

struct BoundsComposer {
    BoundsComposer() : mAdvance(0) {}

    void operator()(const MinikinRect& rect, float advance) {
        MinikinRect tmp = rect;
        tmp.offset(mAdvance, 0);
        mBounds.join(tmp);
        mAdvance += advance;
    }

    float mAdvance;
    MinikinRect mBounds;
};

void getBounds(const U16StringPiece& str, const Range& range, Bidi bidiFlag,
               const MinikinPaint& paint, StartHyphenEdit startHyphen, EndHyphenEdit endHyphen,
               MinikinRect* out) {
    BoundsComposer bc;
    for (const BidiText::RunInfo info : BidiText(str, range, bidiFlag)) {
        for (const auto it/*[context, piece]*/ : LayoutSplitter(str, info.range, info.isRtl)) {
            auto context=it.first;
            auto piece=it.second;
            const StartHyphenEdit pieceStartHyphen =
                    (piece.getStart() == range.getStart()) ? startHyphen : StartHyphenEdit::NO_EDIT;
            const EndHyphenEdit pieceEndHyphen =
                    (piece.getEnd() == range.getEnd()) ? endHyphen : EndHyphenEdit::NO_EDIT;
            BoundsCache::getInstance().getOrCreate(str.substr(context), piece - context.getStart(),
                                                   paint, info.isRtl, pieceStartHyphen,
                                                   pieceEndHyphen, bc);
            // Increment word spacing for spacer
            if (piece.getLength() == 1 && isWordSpace(str[piece.getStart()])) {
                bc.mAdvance += paint.wordSpacing;
            }
        }
    }
    *out = bc.mBounds;
}

struct ExtentComposer {
    ExtentComposer() {}

    void operator()(const LayoutPiece& layoutPiece, const MinikinPaint&) {
        extent.extendBy(layoutPiece.extent());
    }

    MinikinExtent extent;
};

MinikinExtent getFontExtent(const U16StringPiece& textBuf, const Range& range, Bidi bidiFlag,
                            const MinikinPaint& paint) {
    ExtentComposer composer;
    for (const BidiText::RunInfo info : BidiText(textBuf, range, bidiFlag)) {
        for (const auto it/*[context, piece]*/ : LayoutSplitter(textBuf, info.range, info.isRtl)) {
            auto context=it.first;
            auto piece=it.second;
            LayoutCache::getInstance().getOrCreate(
                    textBuf.substr(context), piece - context.getStart(), paint, info.isRtl,
                    StartHyphenEdit::NO_EDIT, EndHyphenEdit::NO_EDIT, composer);
        }
    }
    return composer.extent;
}

}  // namespace minikin
