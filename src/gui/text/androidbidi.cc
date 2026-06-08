/*
 * Copyright (C) 2010 The Android Open Source Project
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
#include <text/layout.h>
#include <text/androidbidi.h>
#include <unicode/ubidi.h>
namespace cdroid {
// Helper function to convert UTF-32 to UTF-16
static void convertUtf32ToUtf16(const char32_t* data, size_t size, std::vector<uint16_t>* outUtf16) {
    outUtf16->reserve(size);
    for (size_t i = 0; i < size; i++) {
        char32_t cp = data[i];
        if (cp <= 0xFFFF) {
            outUtf16->push_back(static_cast<uint16_t>(cp));
        } else {
            // Surrogate pair for characters outside BMP
            cp -= 0x10000;
            outUtf16->push_back(static_cast<uint16_t>(0xD800 | ((cp >> 10) & 0x3FF)));
            outUtf16->push_back(static_cast<uint16_t>(0xDC00 | (cp & 0x3FF)));
        }
    }
}

int AndroidBidi::bidi(int dir, const std::vector<char32_t>& chs, std::vector<uint8_t>& chInfo) {
    if (chs.empty() || chInfo.empty()) {
        //throw new NullPointerException();
    }

    const int length = chs.size();
    if (chInfo.size() < length) {
        //throw new IndexOutOfBoundsException();
    }

    uint8_t paraLevel;
    switch (dir) {
        case TextLayout::DIR_REQUEST_RTL: paraLevel = UBiDiDirection::UBIDI_RTL;break;//Bidi.RTL; break;
        case TextLayout::DIR_REQUEST_DEFAULT_LTR: paraLevel = UBIDI_DEFAULT_LTR;break;//Bidi.LEVEL_DEFAULT_LTR; break;
        case TextLayout::DIR_REQUEST_DEFAULT_RTL: paraLevel = UBIDI_DEFAULT_RTL;break;//Bidi.LEVEL_DEFAULT_RTL; break;
        default: paraLevel = UBiDiDirection::UBIDI_LTR;break;//Bidi.LTR; break;
    }

    // Convert UTF-32 to UTF-16 for ICU BiDi (ICU only supports UTF-16)
    std::vector<uint16_t> utf16Text;
    convertUtf32ToUtf16(chs.data(), chs.size(), &utf16Text);

    UErrorCode errorCode = U_ZERO_ERROR;
    UBiDi *pBiDi = ubidi_openSized(utf16Text.size(), 0, &errorCode);
    ubidi_setPara(pBiDi, reinterpret_cast<const UChar*>(utf16Text.data()), utf16Text.size(),
                  paraLevel, nullptr, &errorCode);

    // Convert UTF-16 indices back to UTF-32 indices
    for (int i = 0; i < length; i++) {
        // For characters in BMP, UTF-32 and UTF-16 indices are the same
        // For surrogate pairs, we need special handling
        // For simplicity, we assume basic multilingual plane characters
        chInfo[i] = ubidi_getLevelAt(pBiDi, i);
    }
    const uint8_t result = ubidi_getParaLevel(pBiDi);
    ubidi_close(pBiDi);
    return (result & 0x1) == 0 ? TextLayout::DIR_LEFT_TO_RIGHT : TextLayout::DIR_RIGHT_TO_LEFT;
}

const Directions* AndroidBidi::directions(int dir,const std::vector<uint8_t>& levels, int lstart,
        const std::vector<char32_t>& chars, int cstart, int len) {
    if (len == 0) {
        return &TextLayout::DIRS_ALL_LEFT_TO_RIGHT;
    }

    int baseLevel = dir == TextLayout::DIR_LEFT_TO_RIGHT ? 0 : 1;
    int curLevel = levels[lstart];
    int minLevel = curLevel;
    int runCount = 1;
    for (int i = lstart + 1, e = lstart + len; i < e; ++i) {
        int level = levels[i];
        if (level != curLevel) {
            curLevel = level;
            ++runCount;
        }
    }

    // add final run for trailing counter-directional whitespace
    int visLen = len;
    if ((curLevel & 1) != (baseLevel & 1)) {
        // look for visible end
        while (--visLen >= 0) {
            char ch = chars[cstart + visLen];

            if (ch == '\n') {
                --visLen;
                break;
            }

            if (ch != ' ' && ch != '\t') {
                break;
            }
        }
        ++visLen;
        if (visLen != len) {
            ++runCount;
        }
    }

    if (runCount == 1 && minLevel == baseLevel) {
        // we're done, only one run on this line
        if ((minLevel & 1) != 0) {
            return &TextLayout::DIRS_ALL_RIGHT_TO_LEFT;
        }
        return &TextLayout::DIRS_ALL_LEFT_TO_RIGHT;
    }

    std::vector<int> ld(runCount * 2);
    int maxLevel = minLevel;
    int levelBits = minLevel << TextLayout::RUN_LEVEL_SHIFT;
    {
        // Start of first pair is always 0, we write
        // length then start at each new run, and the
        // last run length after we're done.
        int n = 1;
        int prev = lstart;
        curLevel = minLevel;
        for (int i = lstart, e = lstart + visLen; i < e; ++i) {
            int level = levels[i];
            if (level != curLevel) {
                curLevel = level;
                if (level > maxLevel) {
                    maxLevel = level;
                } else if (level < minLevel) {
                    minLevel = level;
                }
                // XXX ignore run length limit of 2^RUN_LEVEL_SHIFT
                ld[n++] = (i - prev) | levelBits;
                ld[n++] = i - lstart;
                levelBits = curLevel << TextLayout::RUN_LEVEL_SHIFT;
                prev = i;
            }
        }
        ld[n] = (lstart + visLen - prev) | levelBits;
        if (visLen < len) {
            ld[++n] = visLen;
            ld[++n] = (len - visLen) | (baseLevel << TextLayout::RUN_LEVEL_SHIFT);
        }
    }

    // See if we need to swap any runs.
    // If the min level run direction doesn't match the base
    // direction, we always need to swap (at this point
    // we have more than one run).
    // Otherwise, we don't need to swap the lowest level.
    // Since there are no logically adjacent runs at the same
    // level, if the max level is the same as the (new) min
    // level, we have a series of alternating levels that
    // is already in order, so there's no more to do.
    //
    bool swap;
    if ((minLevel & 1) == baseLevel) {
        minLevel += 1;
        swap = maxLevel > minLevel;
    } else {
        swap = runCount > 1;
    }
    if (swap) {
        for (int level = maxLevel - 1; level >= minLevel; --level) {
            for (int i = 0; i < ld.size(); i += 2) {
                if (levels[ld[i]] >= level) {
                    int e = i + 2;
                    while (e < ld.size() && levels[ld[e]] >= level) {
                        e += 2;
                    }
                    for (int low = i, hi = e - 2; low < hi; low += 2, hi -= 2) {
                        int x = ld[low]; ld[low] = ld[hi]; ld[hi] = x;
                        x = ld[low+1]; ld[low+1] = ld[hi+1]; ld[hi+1] = x;
                    }
                    i = e + 2;
                }
            }
        }
    }
    return new Directions(ld);
}
}
