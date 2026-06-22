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

#ifndef MINIKIN_FONT_H
#define MINIKIN_FONT_H

//#include <gtest/gtest_prod.h>

#include <atomic>
#include <memory>
#include <unordered_set>

#include "minikin/Buffer.h"
#include "minikin/FontStyle.h"
#include "minikin/FontVariation.h"
#include "minikin/HbUtils.h"
#include "minikin/LocaleList.h"
#include "minikin/Macros.h"
#include "minikin/MinikinFont.h"

namespace minikin {

class Font;

// attributes representing transforms (fake bold, fake italic) to match styles
class FontFakery {
public:
    FontFakery() : mFakeBold(false), mFakeItalic(false) {}
    FontFakery(bool fakeBold, bool fakeItalic) : mFakeBold(fakeBold), mFakeItalic(fakeItalic) {}
    // TODO: want to support graded fake bolding
    bool isFakeBold() { return mFakeBold; }
    bool isFakeItalic() { return mFakeItalic; }
    inline bool operator==(const FontFakery& o) const {
        return mFakeBold == o.mFakeBold && mFakeItalic == o.mFakeItalic;
    }
    inline bool operator!=(const FontFakery& o) const { return !(*this == o); }

private:
    bool mFakeBold;
    bool mFakeItalic;
};

struct FakedFont {
    inline bool operator==(const FakedFont& o) const {
        return font == o.font && fakery == o.fakery;
    }
    inline bool operator!=(const FakedFont& o) const { return !(*this == o); }

    // ownership is the enclosing FontCollection
    // FakedFont will be stored in the LayoutCache. It is not a good idea too keep font instance
    // even if the enclosing FontCollection, i.e. Typeface is GC-ed. The layout cache is only
    // purged when it is overflown, thus intentionally keep only reference.
    const std::shared_ptr<Font>& font;
    FontFakery fakery;
};

// Represents a single font file.
class Font {
public:
    class Builder {
    public:
        Builder(const std::shared_ptr<MinikinFont>& typeface) : mTypeface(typeface) {}

        // Override the font style. If not called, info from OS/2 table is used.
        Builder& setStyle(FontStyle style) {
            mWeight = style.weight();
            mSlant = style.slant();
            mIsWeightSet = mIsSlantSet = true;
            return *this;
        }

        // Override the font weight. If not called, info from OS/2 table is used.
        Builder& setWeight(uint16_t weight) {
            mWeight = weight;
            mIsWeightSet = true;
            return *this;
        }

        // Override the font slant. If not called, info from OS/2 table is used.
        Builder& setSlant(FontStyle::Slant slant) {
            mSlant = slant;
            mIsSlantSet = true;
            return *this;
        }

        Builder& setLocaleListId(uint32_t id) {
            mLocaleListId = id;
            return *this;
        }

        std::shared_ptr<Font> build();

    private:
        std::shared_ptr<MinikinFont> mTypeface;
        uint16_t mWeight = static_cast<uint16_t>(FontStyle::Weight::NORMAL);
        FontStyle::Slant mSlant = FontStyle::Slant::UPRIGHT;
        uint32_t mLocaleListId = kEmptyLocaleListId;
        bool mIsWeightSet = false;
        bool mIsSlantSet = false;
    };

    explicit Font(BufferReader* reader);
    void writeTo(BufferWriter* writer) const;

    Font(Font&& o) noexcept;
    Font& operator=(Font&& o) noexcept;
    ~Font();
    // This locale list is just for API compatibility. This is not used in font selection or family
    // fallback.
    uint32_t getLocaleListId() const { return mLocaleListId; }
    const std::shared_ptr<MinikinFont>& typeface() const;
    inline FontStyle style() const { return mStyle; }
    const HbFontUniquePtr& baseFont() const;
    BufferReader typefaceMetadataReader() const { return mTypefaceMetadataReader; }

    std::unordered_set<AxisTag> getSupportedAxes() const;

private:
    // ExternalRefs holds references to objects provided by external libraries.
    // Because creating these external objects is costly,
    // ExternalRefs is lazily created if Font was created by readFrom().
    class ExternalRefs {
    public:
        ExternalRefs(std::shared_ptr<MinikinFont>&& typeface, HbFontUniquePtr&& baseFont)
                : mTypeface(std::move(typeface)), mBaseFont(std::move(baseFont)) {}

        std::shared_ptr<MinikinFont> mTypeface;
        HbFontUniquePtr mBaseFont;
    };

    // Use Builder instead.
    Font(std::shared_ptr<MinikinFont>&& typeface, FontStyle style, HbFontUniquePtr&& baseFont,
         uint32_t localeListId)
            : mExternalRefsHolder(new ExternalRefs(std::move(typeface), std::move(baseFont))),
              mStyle(style),
              mLocaleListId(localeListId),
              mTypefaceMetadataReader(nullptr) {}

    void resetExternalRefs(ExternalRefs* refs);

    const ExternalRefs* getExternalRefs() const;

    static HbFontUniquePtr prepareFont(const std::shared_ptr<MinikinFont>& typeface);
    static FontStyle analyzeStyle(const HbFontUniquePtr& font);

    // Lazy-initialized if created by readFrom().
    mutable std::atomic<ExternalRefs*> mExternalRefsHolder;
    FontStyle mStyle;
    uint32_t mLocaleListId;

    // Non-null if created by readFrom().
    BufferReader mTypefaceMetadataReader;

    // Stop copying.
    Font(const Font& o) = delete;
    Font& operator=(const Font& o) = delete;

    //FRIEND_TEST(FontTest, MoveConstructorTest);
    //FRIEND_TEST(FontTest, MoveAssignmentTest);
};

}  // namespace minikin

#endif  // MINIKIN_FONT_H
