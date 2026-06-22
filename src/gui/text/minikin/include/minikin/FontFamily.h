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

#ifndef MINIKIN_FONT_FAMILY_H
#define MINIKIN_FONT_FAMILY_H

#include <memory>
#include <string>
#include <vector>

#include "minikin/FamilyVariant.h"
#include "minikin/Font.h"
#include "minikin/FontStyle.h"
#include "minikin/HbUtils.h"
#include "minikin/Macros.h"
#include "minikin/SparseBitSet.h"

namespace minikin {

class FontFamily {
public:
    static std::shared_ptr<FontFamily> create(std::vector<std::shared_ptr<Font>>&& fonts);
    static std::shared_ptr<FontFamily> create(FamilyVariant variant,
                                              std::vector<std::shared_ptr<Font>>&& fonts);
    static std::shared_ptr<FontFamily> create(uint32_t localeListId, FamilyVariant variant,
                                              std::vector<std::shared_ptr<Font>>&& fonts,
                                              bool isCustomFallback, bool isDefaultFallback);

    FontFamily(FontFamily&&) = default;
    FontFamily& operator=(FontFamily&&) = default;

    static std::vector<std::shared_ptr<FontFamily>> readVector(BufferReader* reader);
    static void writeVector(BufferWriter* writer,
                            const std::vector<std::shared_ptr<FontFamily>>& families);

    FakedFont getClosestMatch(FontStyle style) const;

    uint32_t localeListId() const { return mLocaleListId; }
    FamilyVariant variant() const { return mVariant; }

    // API's for enumerating the fonts in a family. These don't guarantee any particular order
    size_t getNumFonts() const { return mFontsCount; }
    const Font* getFont(size_t index) const { return mFonts[index].get(); }
    const std::shared_ptr<Font>& getFontRef(size_t index) const { return mFonts[index]; }
    FontStyle getStyle(size_t index) const { return mFonts[index]->style(); }
    bool isColorEmojiFamily() const { return mIsColorEmoji; }
    size_t getSupportedAxesCount() const { return mSupportedAxesCount; }
    AxisTag getSupportedAxisAt(size_t index) const { return mSupportedAxes[index]; }
    bool isCustomFallback() const { return mIsCustomFallback; }
    bool isDefaultFallback() const { return mIsDefaultFallback; }

    // Get Unicode coverage.
    const SparseBitSet& getCoverage() const { return mCoverage; }

    // Returns true if the font has a glyph for the code point and variation selector pair.
    // Caller should acquire a lock before calling the method.
    bool hasGlyph(uint32_t codepoint, uint32_t variationSelector) const;

    // Returns true if this font family has a variaion sequence table (cmap format 14 subtable).
    bool hasVSTable() const { return mCmapFmt14CoverageCount != 0; }

    // Creates new FontFamily based on this family while applying font variations. Returns nullptr
    // if none of variations apply to this family.
    std::shared_ptr<FontFamily> createFamilyWithVariation(
            const std::vector<FontVariation>& variations) const;

private:
    FontFamily(uint32_t localeListId, FamilyVariant variant,
               std::vector<std::shared_ptr<Font>>&& fonts, bool isCustomFallback,
               bool isDefaultFallback);
    explicit FontFamily(BufferReader* reader, const std::shared_ptr<std::vector<Font>>& fonts);

    void writeTo(BufferWriter* writer, uint32_t* fontIndex) const;

    void computeCoverage();

    // Note: to minimize padding, small member fields are grouped at the end.
    std::unique_ptr<std::shared_ptr<Font>[]> mFonts;
    // mSupportedAxes is sorted.
    std::unique_ptr<AxisTag[]> mSupportedAxes;
    SparseBitSet mCoverage;
    std::unique_ptr<SparseBitSet[]> mCmapFmt14Coverage;
    uint32_t mLocaleListId;  // 4 bytes
    uint32_t mFontsCount;    // 4 bytes
    // OpenType supports up to 2^16-1 (uint16) axes.
    // https://docs.microsoft.com/en-us/typography/opentype/spec/fvar
    uint16_t mSupportedAxesCount;      // 2 bytes
    uint16_t mCmapFmt14CoverageCount;  // 2 bytes
    FamilyVariant mVariant;            // 1 byte
    bool mIsColorEmoji;                // 1 byte
    bool mIsCustomFallback;            // 1 byte
    bool mIsDefaultFallback;           // 1 byte

    MINIKIN_PREVENT_COPY_AND_ASSIGN(FontFamily);
};

}  // namespace minikin

#endif  // MINIKIN_FONT_FAMILY_H
