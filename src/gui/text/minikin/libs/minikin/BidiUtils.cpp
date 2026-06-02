/*
 * Copyright (C) 2018 The Android Open Source Project
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

#define LOG_TAG "Minikin"

#include "BidiUtils.h"
#include <stdio.h>
#include <algorithm>

#include <unicode/ubidi.h>
#include <unicode/utf16.h>

#include "minikin/Emoji.h"

#include "MinikinInternal.h"

namespace minikin {

// Helper function to convert UTF-32 to UTF-16
// Also builds mapping tables between UTF-32 and UTF-16 indices
static void convertUtf32ToUtf16(const char32_t* data, size_t size,
                                std::vector<uint16_t>* outUtf16,
                                std::vector<uint32_t>* outUtf32ToUtf16,
                                std::vector<uint32_t>* outUtf16ToUtf32) {
    outUtf16->reserve(size);
    outUtf32ToUtf16->resize(size + 1);
    outUtf16ToUtf32->reserve(size * 2);  // Worst case: all surrogate pairs
    
    uint32_t utf16Offset = 0;
    for (size_t i = 0; i < size; i++) {
        (*outUtf32ToUtf16)[i] = utf16Offset;
        char32_t cp = data[i];
        if (cp <= 0xFFFF) {
            outUtf16->push_back(static_cast<uint16_t>(cp));
            outUtf16ToUtf32->push_back(static_cast<uint32_t>(i));
            utf16Offset++;
        } else {
            // Surrogate pair for characters outside BMP
            cp -= 0x10000;
            outUtf16->push_back(static_cast<uint16_t>(0xD800 | ((cp >> 10) & 0x3FF)));
            outUtf16->push_back(static_cast<uint16_t>(0xDC00 | (cp & 0x3FF)));
            outUtf16ToUtf32->push_back(static_cast<uint32_t>(i));
            outUtf16ToUtf32->push_back(static_cast<uint32_t>(i));  // Both surrogates map to same UTF-32 index
            utf16Offset += 2;
        }
    }
    (*outUtf32ToUtf16)[size] = utf16Offset;
}

static inline UBiDiLevel bidiToUBidiLevel(Bidi bidi) {
    switch (bidi) {
        case Bidi::LTR:
            return 0x00;
        case Bidi::RTL:
            return 0x01;
        case Bidi::DEFAULT_LTR:
            return UBIDI_DEFAULT_LTR;
        case Bidi::DEFAULT_RTL:
            return UBIDI_DEFAULT_RTL;
        case Bidi::FORCE_LTR:
        case Bidi::FORCE_RTL:
            MINIKIN_NOT_REACHED("FORCE_LTR/FORCE_RTL can not be converted to UBiDiLevel.");
            return 0x00;
        default:
            MINIKIN_NOT_REACHED("Unknown Bidi value.");
            return 0x00;
    }
}

BidiText::RunInfo BidiText::getRunInfoAt(uint32_t runOffset) const {
    MINIKIN_ASSERT(runOffset < mRunCount, "Out of range access. %d/%d", runOffset, mRunCount);
    if (mRunCount == 1) {
        // Single run. No need to interact with UBiDi.
        return {mRange, mIsRtl};
    }

    int32_t startRun = -1;
    int32_t lengthRun = -1;
    const UBiDiDirection runDir = ubidi_getVisualRun(mBidi.get(), runOffset, &startRun, &lengthRun);
    if (startRun == -1 || lengthRun == -1) {
        ALOGE("invalid visual run");
        return {Range::invalidRange(), false};
    }
    
    // Convert UTF-16 offsets to UTF-32 offsets
    const uint32_t utf32Start = mUtf16ToUtf32[static_cast<size_t>(startRun)];
    const uint32_t utf32End = mUtf16ToUtf32[static_cast<size_t>(startRun + lengthRun - 1)] + 1;
    
    const uint32_t runStart = std::max(utf32Start, mRange.getStart());
    const uint32_t runEnd = std::min(utf32End, mRange.getEnd());
    if (runEnd <= runStart) {
        // skip the empty run.
        return {Range::invalidRange(), false};
    }
    return {Range(runStart, runEnd), (runDir == UBIDI_RTL)};
}

BidiText::BidiText(const U32StringPiece& textBuf, const Range& range, Bidi bidiFlags)
        : mRange(range), mIsRtl(isRtl(bidiFlags)), mRunCount(1 /* by default, single run */) {
    if (isOverride(bidiFlags)) {
        // force single run.
        return;
    }

    mBidi.reset(ubidi_open());
    if (!mBidi) {
        ALOGE("error creating bidi object");
        return;
    }
    UErrorCode status = U_ZERO_ERROR;
    // Set callbacks to override bidi classes of new emoji
    ubidi_setClassCallback(mBidi.get(), emojiBidiOverride, nullptr, nullptr, nullptr, &status);
    if (!U_SUCCESS(status)) {
        ALOGE("error setting bidi callback function, status = %d", status);
        return;
    }

    // Convert UTF-32 to UTF-16 for ICU BiDi (ICU only supports UTF-16)
    const char32_t* data = textBuf.data();
    std::vector<uint16_t> utf16Text;
    convertUtf32ToUtf16(data, textBuf.size(), &utf16Text, &mUtf32ToUtf16, &mUtf16ToUtf32);

    const UBiDiLevel bidiReq = bidiToUBidiLevel(bidiFlags);
    ubidi_setPara(mBidi.get(), reinterpret_cast<const UChar*>(utf16Text.data()), utf16Text.size(),
                  bidiReq, nullptr, &status);
    if (!U_SUCCESS(status)) {
        ALOGE("error calling ubidi_setPara, status = %d", status);
        return;
    }
    // RTL paragraphs get an odd level, while LTR paragraphs get an even level,
    const bool paraIsRTL = ubidi_getParaLevel(mBidi.get()) & 0x01;
    const ssize_t rc = ubidi_countRuns(mBidi.get(), &status);
    if (!U_SUCCESS(status) || rc < 0) {
        ALOGW("error counting bidi runs, status = %d", status);
        return;
    }
    if (rc == 0) {
        mIsRtl = paraIsRTL;
        return;
    }
    if (rc == 1) {
        // If the paragraph is a single run, override the paragraph dirction with the run
        // (actually the whole text) direction.
        const UBiDiDirection runDir = ubidi_getVisualRun(mBidi.get(), 0, nullptr, nullptr);
        mIsRtl = (runDir == UBIDI_RTL);
        return;
    }
    mRunCount = rc;
}

}  // namespace minikin
