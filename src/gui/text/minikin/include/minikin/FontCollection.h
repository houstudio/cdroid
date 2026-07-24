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

#ifndef MINIKIN_FONT_COLLECTION_H
#define MINIKIN_FONT_COLLECTION_H

//#include <gtest/gtest_prod.h>

#include <memory>
#include <unordered_map>
#include <vector>

#include "minikin/Buffer.h"
#include "minikin/Font.h"
#include "minikin/FontFamily.h"
#include "minikin/MinikinFont.h"
#include "minikin/U16StringPiece.h"

namespace minikin {

// The maximum number of font families.
constexpr uint32_t MAX_FAMILY_COUNT = 254;

class FontCollection {
public:
    static std::shared_ptr<FontCollection> create(
            const std::vector<std::shared_ptr<FontFamily>>& typefaces);
    static std::shared_ptr<FontCollection> create(std::shared_ptr<FontFamily>&& typeface);

    static std::vector<std::shared_ptr<FontCollection>> readVector(BufferReader* reader);
    static void writeVector(BufferWriter* writer,
                            const std::vector<std::shared_ptr<FontCollection>>& fontCollections);

    // Helper class for representing font family match result in packed bits.
    struct FamilyMatchResult {
    public:
        struct Builder {
        public:
            Builder() : mSize(0), mBits(0) {}

            Builder& add(uint8_t x) {
                if (mSize >= 7) /*[[unlikely]]*/ {
                        return *this;
                    }
                mBits = mBits | (static_cast<uint64_t>(x) << (8 * mSize));
                mSize++;
                return *this;
            }

            Builder& reset() {
                mSize = 0;
                mBits = 0;
                return *this;
            }

            uint8_t size() const { return mSize; }

            bool empty() const { return size() == 0; }

            FamilyMatchResult build() {
                return FamilyMatchResult(mBits | (static_cast<uint64_t>(mSize) << 56));
            }

        private:
            uint8_t mSize;
            uint64_t mBits;
        };

        // Helper class for iterating FamilyMatchResult
        class iterator {
        public:
            inline bool operator==(const iterator& o) const {
                return mOffset == o.mOffset && mResult == o.mResult;
            }

            inline bool operator!=(const iterator& o) const { return !(*this == o); }
            inline uint8_t operator*() const { return mResult[mOffset]; }
            inline iterator& operator++() {
                mOffset++;
                return *this;
            }

        private:
            friend struct FamilyMatchResult;
            iterator(const FamilyMatchResult& result, uint32_t offset)
                    : mResult(result), mOffset(offset) {}
            const FamilyMatchResult& mResult;
            uint32_t mOffset;
        };

        // Create empty FamilyMatchResult.
        FamilyMatchResult() : mBits(0) {}

        inline uint8_t size() const { return static_cast<uint8_t>(mBits >> 56); }

        inline uint8_t operator[](uint32_t pos) const {
            return static_cast<uint8_t>(mBits >> (pos * 8));
        }

        inline bool empty() const { return size() == 0; }

        inline bool operator==(const FamilyMatchResult& o) const { return mBits == o.mBits; }

        // Returns the common family indices between l and r.
        static FamilyMatchResult intersect(FamilyMatchResult l, FamilyMatchResult r);

        // Iterator
        inline iterator begin() const { return iterator(*this, 0); }
        inline iterator end() const { return iterator(*this, size()); }

        FamilyMatchResult(const FamilyMatchResult& o) = default;
        FamilyMatchResult& operator=(const FamilyMatchResult& o) = default;

    private:
        explicit FamilyMatchResult(uint64_t bits) : mBits(bits) {}
        uint64_t mBits;
    };

    struct Run {
        FamilyMatchResult familyMatch;
        int start;
        int end;
    };

    FakedFont getBestFont(U16StringPiece textBuf, const Run& run, FontStyle style);

    // Perform the itemization until given max runs.
    std::vector<Run> itemize(U16StringPiece text, FontStyle style, uint32_t localeListId,
                             FamilyVariant familyVariant, uint32_t runMax) const;

    // Perform the itemization until end of the text.
    std::vector<Run> itemize(U16StringPiece text, FontStyle style, uint32_t localeListId,
                             FamilyVariant familyVariant) const {
        return itemize(text, style, localeListId, familyVariant, text.size());
    }

    // Returns true if there is a glyph for the code point and variation selector pair.
    // Returns false if no fonts have a glyph for the code point and variation
    // selector pair, or invalid variation selector is passed.
    bool hasVariationSelector(uint32_t baseCodepoint, uint32_t variationSelector) const;

    // Get base font with fakery information (fake bold could affect metrics)
    FakedFont baseFontFaked(FontStyle style);

    // Creates new FontCollection based on this collection while applying font variations. Returns
    // nullptr if none of variations apply to this collection.
    std::shared_ptr<FontCollection> createCollectionWithVariation(
            const std::vector<FontVariation>& variations);
    // Creates new FontCollection that uses the specified families as top families and
    // families from this FontCollection as fallback.
    std::shared_ptr<FontCollection> createCollectionWithFamilies(
            std::vector<std::shared_ptr<FontFamily>>&& families) const;

    size_t getSupportedAxesCount() const { return mSupportedAxesCount; }
    AxisTag getSupportedAxisAt(size_t index) const { return mSupportedAxes[index]; }

    uint32_t getId() const;

    size_t getFamilyCount() const { return mFamilyCount; }

    const std::shared_ptr<FontFamily>& getFamilyAt(size_t index) const {
        if (mFamilyIndices != nullptr) {
            index = mFamilyIndices[index];
        }
        return (*mMaybeSharedFamilies)[index];
    }

private:
    //FRIEND_TEST(FontCollectionTest, bufferTest);

    explicit FontCollection(const std::vector<std::shared_ptr<FontFamily>>& typefaces);
    FontCollection(
            BufferReader* reader,
            const std::shared_ptr<std::vector<std::shared_ptr<FontFamily>>>& allFontFamilies);
    // Write fields of the instance, using fontFamilyToIndexMap for finding
    // indices for FontFamily.
    void writeTo(BufferWriter* writer,
                 const std::unordered_map<std::shared_ptr<FontFamily>, uint32_t>&
                         fontFamilyToIndexMap) const;
    static void collectAllFontFamilies(
            const std::vector<std::shared_ptr<FontCollection>>& fontCollections,
            std::vector<std::shared_ptr<FontFamily>>* outAllFontFamilies,
            std::unordered_map<std::shared_ptr<FontFamily>, uint32_t>* outFontFamilyToIndexMap);

    static const int kLogCharsPerPage = 8;
    static const int kPageMask = (1 << kLogCharsPerPage) - 1;

    // mFamilyVec holds the indices of the family (as in getFamilyAt()) and
    // mRanges holds the range of indices of mFamilyVec.
    // The maximum number of pages is 0x10FF (U+10FFFF >> 8). The maximum number of
    // the fonts is 0xFF. Thus, technically the maximum length of mFamilyVec is 0x10EE01
    // (0x10FF * 0xFF). However, in practice, 16-bit integers are enough since most fonts supports
    // only limited range of code points.
    struct Range {
        uint16_t start;
        uint16_t end;
    };

    // Initialize the FontCollection.
    void init(const std::vector<std::shared_ptr<FontFamily>>& typefaces);

    FamilyMatchResult getFamilyForChar(uint32_t ch, uint32_t vs, uint32_t localeListId,
                                       FamilyVariant variant) const;

    uint32_t calcFamilyScore(uint32_t ch, uint32_t vs, FamilyVariant variant, uint32_t localeListId,
                             const std::shared_ptr<FontFamily>& fontFamily) const;

    uint32_t calcCoverageScore(uint32_t ch, uint32_t vs, uint32_t localeListId,
                               const std::shared_ptr<FontFamily>& fontFamily) const;

    bool isPrimaryFamily(const std::shared_ptr<FontFamily>& fontFamily) const;

    static uint32_t calcLocaleMatchingScore(uint32_t userLocaleListId,
                                            const FontFamily& fontFamily);

    static uint32_t calcVariantMatchingScore(FamilyVariant variant, const FontFamily& fontFamily);

    // unique id for this font collection (suitable for cache key)
    uint32_t mId;

    // Highest UTF-32 code point that can be mapped
    uint32_t mMaxChar;

    // This vector has pointers to the all font family instances in this collection.
    // This vector can't be empty.
    // This vector may be shared with other font collections.
    // (1) When shared, this vector is a union of all font family instances
    //     shared by multiple font collections.
    //     mFamilyIndices will be non-null in this case.
    //     The i-th family in this collection will be
    //     mMaybeSharedFamilies[mFamilyIndices[i]].
    // (2) When not shared, mFamilyIndices will be null and
    //     the i-th family in this collection will be mMaybeSharedFamilies[i].
    // Use getFamilyAt(i) to access the i-th font in this family.
    std::shared_ptr<std::vector<std::shared_ptr<FontFamily>>> mMaybeSharedFamilies;
    uint32_t mFamilyCount;
    const uint32_t* mFamilyIndices;

    // Following two vectors are pre-calculated tables for resolving coverage faster.
    // For example, to iterate over all fonts which support Unicode code point U+XXYYZZ,
    // iterate font families index from mFamilyVec[mRanges[0xXXYY].start] to
    // mFamilyVec[mRange[0xXXYY].end] instead of whole mFamilies.
    // This vector contains indices into mFamilies.
    // This vector can't be empty.
    uint32_t mRangesCount;
    const Range* mRanges;
    uint32_t mFamilyVecCount;
    const uint8_t* mFamilyVec;

    // This vector has pointers to the font family instances which have cmap 14 subtables.
    std::vector<std::shared_ptr<FontFamily>> mVSFamilyVec;

    // Set of supported axes in this collection.
    uint32_t mSupportedAxesCount;
    // mSupportedAxes is sorted.
    std::unique_ptr<AxisTag[]> mSupportedAxes;

    // Owns allocated memory if this class is created from font families, otherwise these are
    // nullptr.
    std::unique_ptr<Range[]> mOwnedRanges;
    std::vector<uint8_t> mOwnedFamilyVec;
};

}  // namespace minikin

#endif  // MINIKIN_FONT_COLLECTION_H
