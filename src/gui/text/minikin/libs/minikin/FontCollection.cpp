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

#define LOG_TAG "Minikin"

#include "minikin/FontCollection.h"

#include <log/log.h>
#include <unicode/unorm2.h>

#include <algorithm>
#include <unordered_set>

#include "Locale.h"
#include "LocaleListCache.h"
#include "MinikinInternal.h"
#include "minikin/Characters.h"
#include "minikin/Emoji.h"
//#include "minikin/FontFileParser.h"

using std::vector;

namespace minikin {

template <typename T>
static inline T max(T a, T b) {
    return a > b ? a : b;
}

const uint32_t EMOJI_STYLE_VS = 0xFE0F;
const uint32_t TEXT_STYLE_VS = 0xFE0E;

static std::atomic<uint32_t> gNextCollectionId = {0};

namespace {

inline bool isEmojiBreak(uint32_t prevCh, uint32_t ch) {
    return !(isEmojiModifier(ch) || (isRegionalIndicator(prevCh) && isRegionalIndicator(ch)) ||
             isKeyCap(ch) || isTagChar(ch) || ch == CHAR_ZWJ || prevCh == CHAR_ZWJ);
}

// Lower is better
uint32_t getGlyphScore(U16StringPiece text, uint32_t start, uint32_t end,
                       const HbFontUniquePtr& font) {
    HbBufferUniquePtr buffer(hb_buffer_create());
    hb_buffer_set_direction(buffer.get(), HB_DIRECTION_LTR);
    hb_buffer_add_utf16(buffer.get(), text.data() + start, end - start, 0, end - start);
    hb_buffer_guess_segment_properties(buffer.get());

    unsigned int numGlyphs = -1;
    hb_shape(font.get(), buffer.get(), nullptr, 0);
    hb_glyph_info_t* info = hb_buffer_get_glyph_infos(buffer.get(), &numGlyphs);

    // HarfBuzz squashed unsupported tag sequence into first emoji glyph. So, we cannot use glyph
    // count for the font selection score. Give extra score if the base score is different from the
    // first glyph.
    if (numGlyphs == 1) {
        constexpr uint32_t TAG_SEQUENCE_FALLBACK_PENALTY = 0x10000;

        uint32_t ch = 0;
        const uint16_t* string = text.data();
        const uint32_t string_size = text.size();
        uint32_t readLength = 0;

        U16_NEXT(string, readLength, string_size, ch);
        if (U_IS_SURROGATE(ch)) {
            return numGlyphs;  // Broken surrogate pair.
        }

        if (readLength >= string_size) {
            return numGlyphs;  // No more characters remaining.
        }

        uint32_t nextCh = 0;
        U16_NEXT(string, readLength, string_size, nextCh);

        if (!isTagChar(nextCh)) {
            return numGlyphs;  // Not a tag sequence.
        }

        uint32_t composedGlyphId = info[0].codepoint;

        // Shape only the first base emoji.
        hb_buffer_reset(buffer.get());
        hb_buffer_set_direction(buffer.get(), HB_DIRECTION_LTR);
        hb_buffer_add_codepoints(buffer.get(), &ch, 1, 0, 1);
        hb_buffer_guess_segment_properties(buffer.get());

        unsigned int numGlyphs = -1;
        hb_shape(font.get(), buffer.get(), nullptr, 0);
        info = hb_buffer_get_glyph_infos(buffer.get(), &numGlyphs);

        if (numGlyphs != 1) {
            // If the single code point of the first base emoji is decomposed to multiple glyphs,
            // we don't support it.
            return numGlyphs;
        }

        uint32_t baseGlyphId = info[0].codepoint;
        if (composedGlyphId == baseGlyphId) {
            return numGlyphs + TAG_SEQUENCE_FALLBACK_PENALTY;
        } else {
            return numGlyphs;
        }
    }

    return numGlyphs;
}

}  // namespace

// static
std::shared_ptr<FontCollection> FontCollection::create(std::shared_ptr<FontFamily>&& typeface) {
    std::vector<std::shared_ptr<FontFamily>> typefaces;
    typefaces.push_back(typeface);
    return create(typefaces);
}

// static
std::shared_ptr<FontCollection> FontCollection::create(
        const vector<std::shared_ptr<FontFamily>>& typefaces) {
    // TODO(b/174672300): Revert back to make_shared.
    return std::shared_ptr<FontCollection>(new FontCollection(typefaces));
}

FontCollection::FontCollection(const vector<std::shared_ptr<FontFamily>>& typefaces)
        : mMaxChar(0), mSupportedAxes(nullptr) {
    init(typefaces);
}

void FontCollection::init(const vector<std::shared_ptr<FontFamily>>& typefaces) {
    mId = gNextCollectionId++;
    vector<uint32_t> lastChar;
    size_t nTypefaces = typefaces.size();
    const FontStyle defaultStyle;
    auto families = std::make_shared<vector<std::shared_ptr<FontFamily>>>();
    std::unordered_set<AxisTag> supportedAxesSet;
    for (size_t i = 0; i < nTypefaces; i++) {
        const std::shared_ptr<FontFamily>& family = typefaces[i];
        if (family->getClosestMatch(defaultStyle).font == nullptr) {
            continue;
        }
        const SparseBitSet& coverage = family->getCoverage();
        families->emplace_back(family);
        if (family->hasVSTable()) {
            mVSFamilyVec.push_back(family);
        }
        mMaxChar = max(mMaxChar, coverage.length());
        lastChar.push_back(coverage.nextSetBit(0));

        for (size_t i = 0; i < family->getSupportedAxesCount(); i++) {
            supportedAxesSet.insert(family->getSupportedAxisAt(i));
        }
    }
    // mMaybeSharedFamilies is not shared.
    mMaybeSharedFamilies = families;
    mFamilyCount = families->size();
    mFamilyIndices = nullptr;
    MINIKIN_ASSERT(mFamilyCount > 0, "Font collection must have at least one valid typeface");
    MINIKIN_ASSERT(mFamilyCount <= MAX_FAMILY_COUNT,
                   "Font collection may only have up to %d font families.", MAX_FAMILY_COUNT);
    // Although OpenType supports up to 2^16-1 axes per font,
    // mSupportedAxesCount may exceed 2^16-1 as we have multiple fonts.
    mSupportedAxesCount = static_cast<uint32_t>(supportedAxesSet.size());
    if (mSupportedAxesCount > 0) {
        mSupportedAxes = sortedArrayFromSet(supportedAxesSet);
    }
    size_t nPages = (mMaxChar + kPageMask) >> kLogCharsPerPage;
    // TODO: Use variation selector map for mRanges construction.
    // A font can have a glyph for a base code point and variation selector pair but no glyph for
    // the base code point without variation selector. The family won't be listed in the range in
    // this case.
    mOwnedRanges = std::make_unique<Range[]>(nPages);
    mRanges = mOwnedRanges.get();
    mRangesCount = nPages;
    for (size_t i = 0; i < nPages; i++) {
        Range* range = &mOwnedRanges[i];
        range->start = mOwnedFamilyVec.size();
        for (size_t j = 0; j < getFamilyCount(); j++) {
            if (lastChar[j] < (i + 1) << kLogCharsPerPage) {
                const std::shared_ptr<FontFamily>& family = getFamilyAt(j);
                mOwnedFamilyVec.push_back(static_cast<uint8_t>(j));
                uint32_t nextChar = family->getCoverage().nextSetBit((i + 1) << kLogCharsPerPage);
                lastChar[j] = nextChar;
            }
        }
        range->end = mOwnedFamilyVec.size();
    }
    // See the comment in Range for more details.
    LOG_ALWAYS_FATAL_IF(mOwnedFamilyVec.size() >= 0xFFFF,
                        "Exceeded the maximum indexable cmap coverage.");
    mFamilyVec = mOwnedFamilyVec.data();
    mFamilyVecCount = mOwnedFamilyVec.size();
}

FontCollection::FontCollection(
        BufferReader* reader,
        const std::shared_ptr<std::vector<std::shared_ptr<FontFamily>>>& families)
        : mSupportedAxes(nullptr) {
    mId = gNextCollectionId++;
    mMaxChar = reader->read<uint32_t>();
    mMaybeSharedFamilies = families;
    std::tie(mFamilyIndices, mFamilyCount) = reader->readArray<uint32_t>();
    for (size_t i = 0; i < getFamilyCount(); i++) {
        const auto& family = getFamilyAt(i);
        if (family->hasVSTable()) mVSFamilyVec.emplace_back(family);
    }
    // Range is two packed uint16_t
    static_assert(sizeof(Range) == 4);
    std::tie(mRanges, mRangesCount) = reader->readArray<Range>();
    std::tie(mFamilyVec, mFamilyVecCount) = reader->readArray<uint8_t>();
    const auto& kv/*[axesPtr, axesCount]*/ = reader->readArray<AxisTag>();
    auto axesPtr  = kv.first;
    auto axesCount= kv.second;
    mSupportedAxesCount = axesCount;
    if (axesCount > 0) {
        mSupportedAxes = std::unique_ptr<AxisTag[]>(new AxisTag[axesCount]);
        std::copy(axesPtr, axesPtr + axesCount, mSupportedAxes.get());
    }
}

void FontCollection::writeTo(BufferWriter* writer,
                             const std::unordered_map<std::shared_ptr<FontFamily>, uint32_t>&
                                     fontFamilyToIndexMap) const {
    writer->write<uint32_t>(mMaxChar);
    std::vector<uint32_t> indices;
    indices.reserve(getFamilyCount());
    for (size_t i = 0; i < getFamilyCount(); ++i) {
        const std::shared_ptr<FontFamily>& fontFamily = getFamilyAt(i);
        auto it = fontFamilyToIndexMap.find(fontFamily);
        if (it == fontFamilyToIndexMap.end()) {
            ALOGE("fontFamily not found in fontFamilyToIndexMap");
        } else {
            indices.push_back(it->second);
        }
    }
    writer->writeArray<uint32_t>(indices.data(), indices.size());
    writer->writeArray<Range>(mRanges, mRangesCount);
    writer->writeArray<uint8_t>(mFamilyVec, mFamilyVecCount);
    // No need to serialize mVSFamilyVec as it can be reconstructed easily from mFamilies.
    writer->writeArray<AxisTag>(mSupportedAxes.get(), mSupportedAxesCount);
}

// static
std::vector<std::shared_ptr<FontCollection>> FontCollection::readVector(BufferReader* reader) {
    auto allFontFamilies = std::make_shared<std::vector<std::shared_ptr<FontFamily>>>(
            FontFamily::readVector(reader));
    uint32_t count = reader->read<uint32_t>();
    std::vector<std::shared_ptr<FontCollection>> fontCollections;
    fontCollections.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        fontCollections.emplace_back(new FontCollection(reader, allFontFamilies));
    }
    return fontCollections;
}

// static
void FontCollection::writeVector(
        BufferWriter* writer, const std::vector<std::shared_ptr<FontCollection>>& fontCollections) {
    std::vector<std::shared_ptr<FontFamily>> allFontFamilies;
    // Note: operator== for shared_ptr compares raw pointer values.
    std::unordered_map<std::shared_ptr<FontFamily>, uint32_t> fontFamilyToIndexMap;
    collectAllFontFamilies(fontCollections, &allFontFamilies, &fontFamilyToIndexMap);

    FontFamily::writeVector(writer, allFontFamilies);
    writer->write<uint32_t>(fontCollections.size());
    for (const auto& fontCollection : fontCollections) {
        fontCollection->writeTo(writer, fontFamilyToIndexMap);
    }
}

// static
void FontCollection::collectAllFontFamilies(
        const std::vector<std::shared_ptr<FontCollection>>& fontCollections,
        std::vector<std::shared_ptr<FontFamily>>* outAllFontFamilies,
        std::unordered_map<std::shared_ptr<FontFamily>, uint32_t>* outFontFamilyToIndexMap) {
    for (const auto& fontCollection : fontCollections) {
        for (size_t i = 0; i < fontCollection->getFamilyCount(); ++i) {
            const std::shared_ptr<FontFamily>& fontFamily = fontCollection->getFamilyAt(i);
            bool inserted =
                    outFontFamilyToIndexMap->emplace(fontFamily, outAllFontFamilies->size()).second;
            if (inserted) {
                outAllFontFamilies->push_back(fontFamily);
            }
        }
    }
}

// Special scores for the font fallback.
const uint32_t kUnsupportedFontScore = 0;
const uint32_t kFirstFontScore = UINT32_MAX;

// Calculates a font score.
// The score of the font family is based on three subscores.
//  - Coverage Score: How well the font family covers the given character or variation sequence.
//  - Locale Score: How well the font family is appropriate for the locale.
//  - Variant Score: Whether the font family matches the variant. Note that this variant is not the
//    one in BCP47. This is our own font variant (e.g., elegant, compact).
//
// Then, there is a priority for these three subscores as follow:
//   Coverage Score > Locale Score > Variant Score
// The returned score reflects this priority order.
//
// Note that there are two special scores.
//  - kUnsupportedFontScore: When the font family doesn't support the variation sequence or even its
//    base character.
//  - kFirstFontScore: When the font is the first font family in the collection and it supports the
//    given character or variation sequence.
uint32_t FontCollection::calcFamilyScore(uint32_t ch, uint32_t vs, FamilyVariant variant,
                                         uint32_t localeListId,
                                         const std::shared_ptr<FontFamily>& fontFamily) const {
    const uint32_t coverageScore = calcCoverageScore(ch, vs, localeListId, fontFamily);
    if (coverageScore == kFirstFontScore || coverageScore == kUnsupportedFontScore) {
        // No need to calculate other scores.
        return coverageScore;
    }

    const uint32_t localeScore = calcLocaleMatchingScore(localeListId, *fontFamily);
    const uint32_t variantScore = calcVariantMatchingScore(variant, *fontFamily);

    // Subscores are encoded into 31 bits representation to meet the subscore priority.
    // The highest 2 bits are for coverage score, then following 28 bits are for locale score,
    // then the last 1 bit is for variant score.
    return coverageScore << 29 | localeScore << 1 | variantScore;
}

// Returns true if
//  - the fontFamily is a developer specified custom fallback.
//  - no custom fallback is provided and the fontFamily is a default fallback.
bool FontCollection::isPrimaryFamily(const std::shared_ptr<FontFamily>& fontFamily) const {
    // If the font family is provided by developers, it is primary.
    if (fontFamily->isCustomFallback()) {
        return true;
    }

    if (getFamilyAt(0)->isCustomFallback()) {
        return false;
    } else {
        return fontFamily->isDefaultFallback();
    }
}

// Calculates a font score based on variation sequence coverage.
// - Returns kUnsupportedFontScore if the font doesn't support the variation sequence or its base
//   character.
// - Returns kFirstFontScore if the font family is the first font family in the collection and it
//   supports the given character or variation sequence.
// - Returns 3 if the font family supports the variation sequence.
// - Returns 2 if the vs is a color variation selector (U+FE0F) and if the font is an emoji font.
// - Returns 2 if the vs is a text variation selector (U+FE0E) and if the font is not an emoji font.
// - Returns 1 if the variation selector is not specified or if the font family only supports the
//   variation sequence's base character.
uint32_t FontCollection::calcCoverageScore(uint32_t ch, uint32_t vs, uint32_t localeListId,
                                           const std::shared_ptr<FontFamily>& fontFamily) const {
    const bool hasVSGlyph = (vs != 0) && fontFamily->hasGlyph(ch, vs);
    if (!hasVSGlyph && !fontFamily->getCoverage().get(ch)) {
        // The font doesn't support either variation sequence or even the base character.
        return kUnsupportedFontScore;
    }

    if ((vs == 0 || hasVSGlyph) && isPrimaryFamily(fontFamily)) {
        // If the first font family supports the given character or variation sequence, always use
        // it.
        return kFirstFontScore;
    }

    if (vs != 0 && hasVSGlyph) {
        return 3;
    }

    bool colorEmojiRequest;
    if (vs == EMOJI_STYLE_VS) {
        colorEmojiRequest = true;
    } else if (vs == TEXT_STYLE_VS) {
        colorEmojiRequest = false;
    } else {
        switch (LocaleListCache::getById(localeListId).getEmojiStyle()) {
            case EmojiStyle::EMOJI:
                colorEmojiRequest = true;
                break;
            case EmojiStyle::TEXT:
                colorEmojiRequest = false;
                break;
            case EmojiStyle::EMPTY:
            case EmojiStyle::DEFAULT:
            default:
                // Do not give any extra score for the default emoji style.
                return 1;
                break;
        }
    }

    return colorEmojiRequest == fontFamily->isColorEmojiFamily() ? 2 : 1;
}

// Calculate font scores based on the script matching, subtag matching and primary locale matching.
//
// 1. If only the font's language matches or there is no matches between requested font and
//    supported font, then the font obtains a score of 0.
// 2. Without a match in language, considering subtag may change font's EmojiStyle over script,
//    a match in subtag gets a score of 2 and a match in scripts gains a score of 1.
// 3. Regarding to two elements matchings, language-and-subtag matching has a score of 4, while
//    language-and-script obtains a socre of 3 with the same reason above.
//
// If two locales in the requested list have the same locale score, the font matching with higher
// priority locale gets a higher score. For example, in the case the user requested locale list is
// "ja-Jpan,en-Latn". The score of for the font of "ja-Jpan" gets a higher score than the font of
// "en-Latn".
//
// To achieve score calculation with priorities, the locale score is determined as follows:
//   LocaleScore = s(0) * 5^(m - 1) + s(1) * 5^(m - 2) + ... + s(m - 2) * 5 + s(m - 1)
// Here, m is the maximum number of locales to be compared, and s(i) is the i-th locale's matching
// score. The possible values of s(i) are 0, 1, 2, 3 and 4.
uint32_t FontCollection::calcLocaleMatchingScore(uint32_t userLocaleListId,
                                                 const FontFamily& fontFamily) {
    const LocaleList& localeList = LocaleListCache::getById(userLocaleListId);
    const LocaleList& fontLocaleList = LocaleListCache::getById(fontFamily.localeListId());

    const size_t maxCompareNum = std::min(localeList.size(), FONT_LOCALE_LIMIT);
    uint32_t score = 0;
    for (size_t i = 0; i < maxCompareNum; ++i) {
        score = score * 5u + localeList[i].calcScoreFor(fontLocaleList);
    }
    return score;
}

// Calculates a font score based on variant ("compact" or "elegant") matching.
//  - Returns 1 if the font doesn't have variant or the variant matches with the text style.
//  - No score if the font has a variant but it doesn't match with the text style.
uint32_t FontCollection::calcVariantMatchingScore(FamilyVariant variant,
                                                  const FontFamily& fontFamily) {
    const FamilyVariant familyVariant = fontFamily.variant();
    if (familyVariant == FamilyVariant::DEFAULT) {
        return 1;
    }
    if (familyVariant == variant) {
        return 1;
    }
    if (variant == FamilyVariant::DEFAULT && familyVariant == FamilyVariant::COMPACT) {
        // If default is requested, prefer compat variation.
        return 1;
    }
    return 0;
}

// Implement heuristic for choosing best-match font. Here are the rules:
// 1. If first font in the collection has the character, it wins.
// 2. Calculate a score for the font family. See comments in calcFamilyScore for the detail.
// 3. Highest score wins, with ties resolved to the first font.
// This method never returns nullptr.
FontCollection::FamilyMatchResult FontCollection::getFamilyForChar(uint32_t ch, uint32_t vs,
                                                                   uint32_t localeListId,
                                                                   FamilyVariant variant) const {
    if (ch >= mMaxChar) {
        return FamilyMatchResult::Builder().add(0).build();
    }

    Range range = mRanges[ch >> kLogCharsPerPage];

    if (vs != 0) {
        range = {0, static_cast<uint16_t>(getFamilyCount())};
    }

    uint32_t bestScore = kUnsupportedFontScore;
    FamilyMatchResult::Builder builder;

    for (size_t i = range.start; i < range.end; i++) {
        const uint8_t familyIndex = vs == 0 ? mFamilyVec[i] : i;
        const std::shared_ptr<FontFamily>& family = getFamilyAt(familyIndex);
        const uint32_t score = calcFamilyScore(ch, vs, variant, localeListId, family);
        if (score == kFirstFontScore) {
            // If the first font family supports the given character or variation sequence, always
            // use it.
            return builder.add(familyIndex).build();
        }
        if (score != kUnsupportedFontScore && score >= bestScore) {
            if (score > bestScore) {
                builder.reset();
                bestScore = score;
            }
            builder.add(familyIndex);
        }
    }
    if (builder.empty()) {
        UErrorCode errorCode = U_ZERO_ERROR;
        const UNormalizer2* normalizer = unorm2_getNFDInstance(&errorCode);
        if (U_SUCCESS(errorCode)) {
            UChar decomposed[4];
            int len = unorm2_getRawDecomposition(normalizer, ch, decomposed, 4, &errorCode);
            if (U_SUCCESS(errorCode) && len > 0) {
                int off = 0;
                U16_NEXT_UNSAFE(decomposed, off, ch);
                return getFamilyForChar(ch, vs, localeListId, variant);
            }
        }
        return FamilyMatchResult::Builder().add(0).build();
    }
    return builder.build();
}

// Characters where we want to continue using existing font run for (or stick to the next run if
// they start a string), even if the font does not support them explicitly. These are handled
// properly by Minikin or HarfBuzz even if the font does not explicitly support them and it's
// usually meaningless to switch to a different font to display them.
static bool doesNotNeedFontSupport(uint32_t c) {
    return c == 0x00AD                      // SOFT HYPHEN
           || c == 0x034F                   // COMBINING GRAPHEME JOINER
           || c == 0x061C                   // ARABIC LETTER MARK
           || (0x200C <= c && c <= 0x200F)  // ZERO WIDTH NON-JOINER..RIGHT-TO-LEFT MARK
           || (0x202A <= c && c <= 0x202E)  // LEFT-TO-RIGHT EMBEDDING..RIGHT-TO-LEFT OVERRIDE
           || (0x2066 <= c && c <= 0x2069)  // LEFT-TO-RIGHT ISOLATE..POP DIRECTIONAL ISOLATE
           || c == 0xFEFF                   // BYTE ORDER MARK
           || isVariationSelector(c);
}

// Characters where we want to continue using existing font run instead of
// recomputing the best match in the fallback list.
static const uint32_t stickyAllowlist[] = {
        '!',    ',', '-', '.', ':', ';', '?',
        0x00A0,  // NBSP
        0x2010,  // HYPHEN
        0x2011,  // NB_HYPHEN
        0x202F,  // NNBSP
        0x2640,  // FEMALE_SIGN,
        0x2642,  // MALE_SIGN,
        0x2695,  // STAFF_OF_AESCULAPIUS
};

static bool isStickyAllowlisted(uint32_t c) {
    for (size_t i = 0; i < sizeof(stickyAllowlist) / sizeof(stickyAllowlist[0]); i++) {
        if (stickyAllowlist[i] == c) return true;
    }
    return false;
}

static inline bool isCombining(uint32_t c) {
    return (U_GET_GC_MASK(c) & U_GC_M_MASK) != 0;
}

bool FontCollection::hasVariationSelector(uint32_t baseCodepoint,
                                          uint32_t variationSelector) const {
    if (!isVariationSelector(variationSelector)) {
        return false;
    }
    if (baseCodepoint >= mMaxChar) {
        return false;
    }

    // Currently mRanges can not be used here since it isn't aware of the variation sequence.
    for (size_t i = 0; i < mVSFamilyVec.size(); i++) {
        if (mVSFamilyVec[i]->hasGlyph(baseCodepoint, variationSelector)) {
            return true;
        }
    }

    // Even if there is no cmap format 14 subtable entry for the given sequence, should return true
    // for <char, text presentation selector> case since we have special fallback rule for the
    // sequence. Note that we don't need to restrict this to already standardized variation
    // sequences, since Unicode is adding variation sequences more frequently now and may even move
    // towards allowing text and emoji variation selectors on any character.
    if (variationSelector == TEXT_STYLE_VS) {
        for (size_t i = 0; i < getFamilyCount(); ++i) {
            const std::shared_ptr<FontFamily>& family = getFamilyAt(i);
            if (!family->isColorEmojiFamily() && family->hasGlyph(baseCodepoint, 0)) {
                return true;
            }
        }
    }

    return false;
}

constexpr uint32_t REPLACEMENT_CHARACTER = 0xFFFD;

FontCollection::FamilyMatchResult FontCollection::FamilyMatchResult::intersect(
        FontCollection::FamilyMatchResult l, FontCollection::FamilyMatchResult r) {
    if (l == r) {
        return l;
    }

    uint32_t li = 0;
    uint32_t ri = 0;
    FamilyMatchResult::Builder b;
    while (li < l.size() && ri < r.size()) {
        if (l[li] < r[ri]) {
            li++;
        } else if (l[li] > r[ri]) {
            ri++;
        } else {  // l[li] == r[ri]
            b.add(l[li]);
            li++;
            ri++;
        }
    }
    return b.build();
}

std::vector<FontCollection::Run> FontCollection::itemize(U16StringPiece text, FontStyle,
                                                         uint32_t localeListId,
                                                         FamilyVariant familyVariant,
                                                         uint32_t runMax) const {
    const uint16_t* string = text.data();
    const uint32_t string_size = text.size();

    FamilyMatchResult lastFamilyIndices = FamilyMatchResult();

    if (string_size == 0) {
        return std::vector<Run>();
    }

    const uint32_t kEndOfString = 0xFFFFFFFF;
    std::vector<Run> result;
    Run* run = nullptr;

    uint32_t nextCh = 0;
    uint32_t prevCh = 0;
    size_t nextUtf16Pos = 0;
    size_t readLength = 0;
    U16_NEXT(string, readLength, string_size, nextCh);
    if (U_IS_SURROGATE(nextCh)) {
        nextCh = REPLACEMENT_CHARACTER;
    }

    do {
        const uint32_t ch = nextCh;
        const size_t utf16Pos = nextUtf16Pos;
        nextUtf16Pos = readLength;
        if (readLength < string_size) {
            U16_NEXT(string, readLength, string_size, nextCh);
            if (U_IS_SURROGATE(nextCh)) {
                nextCh = REPLACEMENT_CHARACTER;
            }
        } else {
            nextCh = kEndOfString;
        }

        bool shouldContinueRun = false;
        if (doesNotNeedFontSupport(ch)) {
            // Always continue if the character is a format character not needed to be in the font.
            shouldContinueRun = true;
        } else if (!lastFamilyIndices.empty() && (isStickyAllowlisted(ch) || isCombining(ch))) {
            // Continue using existing font as long as it has coverage and is whitelisted.

            const std::shared_ptr<FontFamily>& lastFamily = getFamilyAt(lastFamilyIndices[0]);
            if (lastFamily->isColorEmojiFamily()) {
                // If the last family is color emoji font, find the longest family.
                shouldContinueRun = false;
                for (uint8_t ix : lastFamilyIndices) {
                    shouldContinueRun |= getFamilyAt(ix)->getCoverage().get(ch);
                }
            } else {
                shouldContinueRun = lastFamily->getCoverage().get(ch);
            }
        }

        if (!shouldContinueRun) {
            FamilyMatchResult familyIndices = getFamilyForChar(
                    ch, isVariationSelector(nextCh) ? nextCh : 0, localeListId, familyVariant);
            bool breakRun;
            if (utf16Pos == 0 || lastFamilyIndices.empty()) {
                breakRun = true;
            } else {
                const std::shared_ptr<FontFamily>& lastFamily = getFamilyAt(lastFamilyIndices[0]);
                if (lastFamily->isColorEmojiFamily()) {
                    FamilyMatchResult intersection =
                            FamilyMatchResult::intersect(familyIndices, lastFamilyIndices);
                    if (intersection.empty()) {
                        breakRun = true;  // None of last family can draw the given char.
                    } else {
                        breakRun = isEmojiBreak(prevCh, ch);
                        if (!breakRun) {
                            // To select sequence supported families, update family indices with the
                            // intersection between the supported families between prev char and
                            // current char.
                            familyIndices = intersection;
                            lastFamilyIndices = intersection;
                            run->familyMatch = intersection;
                        }
                    }
                } else {
                    breakRun = familyIndices[0] != lastFamilyIndices[0];
                }
            }

            if (breakRun) {
                size_t start = utf16Pos;
                // Workaround for combining marks and emoji modifiers until we implement
                // per-cluster font selection: if a combining mark or an emoji modifier is found in
                // a different font that also supports the previous character, attach previous
                // character to the new run. U+20E3 COMBINING ENCLOSING KEYCAP, used in emoji, is
                // handled properly by this since it's a combining mark too.
                if (utf16Pos != 0 &&
                    (isCombining(ch) || (isEmojiModifier(ch) && isEmojiBase(prevCh)))) {
                    for (uint8_t ix : familyIndices) {
                        if (getFamilyAt(ix)->getCoverage().get(prevCh)) {
                            const size_t prevChLength = U16_LENGTH(prevCh);
                            if (run != nullptr) {
                                run->end -= prevChLength;
                                if (run->start == run->end) {
                                    result.pop_back();
                                }
                            }
                            start -= prevChLength;
                            break;
                        }
                    }
                }
                if (lastFamilyIndices.empty()) {
                    // This is the first family ever assigned. We are either seeing the very first
                    // character (which means start would already be zero), or we have only seen
                    // characters that don't need any font support (which means we need to adjust
                    // start to be 0 to include those characters).
                    start = 0;
                }
                result.push_back({familyIndices, static_cast<int>(start), 0});
                run = &result.back();
                lastFamilyIndices = run->familyMatch;
            }
        }
        prevCh = ch;
        if (run != nullptr) {
            run->end = nextUtf16Pos;  // exclusive
        }

        // Stop searching the remaining characters if the result length gets runMax + 2.
        // When result.size gets runMax + 2 here, the run between [0, runMax) was finalized.
        // If the result.size() equals to runMax, the run may be still expanding.
        // if the result.size() equals to runMax + 2, the last run may be removed and the last run
        // may be exntended the previous run with above workaround.
        if (result.size() >= 2 && runMax == result.size() - 2) {
            break;
        }
    } while (nextCh != kEndOfString);

    if (lastFamilyIndices.empty()) {
        // No character needed any font support, so it doesn't really matter which font they end up
        // getting displayed in. We put the whole string in one run, using the first font.
        result.push_back(
                {FamilyMatchResult::Builder().add(0).build(), 0, static_cast<int>(string_size)});
    }

    if (result.size() > runMax) {
        // The itemization has terminated since it reaches the runMax. Remove last unfinalized runs.
        return std::vector<Run>(result.begin(), result.begin() + runMax);
    }

    return result;
}

FakedFont FontCollection::getBestFont(U16StringPiece text, const Run& run, FontStyle style) {
    uint8_t bestIndex = 0;
    uint32_t bestScore = 0xFFFFFFFF;

    const std::shared_ptr<FontFamily>& family = getFamilyAt(run.familyMatch[0]);
    if (family->isColorEmojiFamily() && run.familyMatch.size() > 1) {
        for (size_t i = 0; i < run.familyMatch.size(); ++i) {
            const std::shared_ptr<FontFamily>& family = getFamilyAt(run.familyMatch[i]);
            const HbFontUniquePtr& font = family->getFont(0)->baseFont();
            uint32_t score = getGlyphScore(text, run.start, run.end, font);

            if (score < bestScore) {
                bestIndex = run.familyMatch[i];
                bestScore = score;
            }
        }
    } else {
        bestIndex = run.familyMatch[0];
    }
    return getFamilyAt(bestIndex)->getClosestMatch(style);
}

FakedFont FontCollection::baseFontFaked(FontStyle style) {
    return getFamilyAt(0)->getClosestMatch(style);
}

std::shared_ptr<FontCollection> FontCollection::createCollectionWithVariation(
        const std::vector<FontVariation>& variations) {
    if (variations.empty() || mSupportedAxesCount == 0) {
        return nullptr;
    }

    bool hasSupportedAxis = false;
    for (const FontVariation& variation : variations) {
        if (std::binary_search(mSupportedAxes.get(), mSupportedAxes.get() + mSupportedAxesCount,
                               variation.axisTag)) {
            hasSupportedAxis = true;
            break;
        }
    }
    if (!hasSupportedAxis) {
        // None of variation axes are supported by this font collection.
        return nullptr;
    }

    std::vector<std::shared_ptr<FontFamily>> families;
    for (size_t i = 0; i < getFamilyCount(); ++i) {
        const std::shared_ptr<FontFamily>& family = getFamilyAt(i);
        std::shared_ptr<FontFamily> newFamily = family->createFamilyWithVariation(variations);
        if (newFamily) {
            families.push_back(newFamily);
        } else {
            families.push_back(family);
        }
    }

    return std::shared_ptr<FontCollection>(new FontCollection(families));
}

std::shared_ptr<FontCollection> FontCollection::createCollectionWithFamilies(
        std::vector<std::shared_ptr<FontFamily>>&& families) const {
    families.reserve(families.size() + getFamilyCount());
    for (size_t i = 0; i < getFamilyCount(); i++) {
        families.push_back(getFamilyAt(i));
    }
    return FontCollection::create(families);
}

uint32_t FontCollection::getId() const {
    return mId;
}

}  // namespace minikin
