/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include "minikin/FontFileParser.h"

#define LOG_TAG "Minikin"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <hb-ot.h>
#include <hb.h>

#include "MinikinInternal.h"
#include "minikin/MinikinFont.h"

namespace minikin {

namespace {

class SafeFontBufferReader {
public:
    SafeFontBufferReader(const void* buffer, size_t size)
            : mBuffer(reinterpret_cast<const uint8_t*>(buffer)),
              mSize(size),
              mPos(0),
              mError(false) {}

    template <typename T>
    T readBE() {
        if (mError) return T();

        if ((mSize - mPos) < sizeof(T)) {
            mError = true;
            return T();
        }
        const T* data = reinterpret_cast<const T*>(mBuffer + mPos);
        mPos += sizeof(T);
        return *data;
    }

    uint16_t readU16() {
        if (mError) return 0;

        if ((mSize - mPos) < 2) {
            mError = true;
            return 0;
        }
        uint16_t out = ((uint32_t)mBuffer[mPos]) << 8 | ((uint32_t)mBuffer[mPos + 1]);
        mPos += 2;
        return out;
    };

    uint32_t readU32() {
        if (mError) return 0;

        if ((mSize - mPos) < 4) {
            mError = true;
            return 0;
        }

        uint32_t out = ((uint32_t)mBuffer[mPos]) << 24 | ((uint32_t)mBuffer[mPos + 1]) << 16 |
                       ((uint32_t)mBuffer[mPos + 2]) << 8 | ((uint32_t)mBuffer[mPos + 3]);
        mPos += 4;
        return out;
    };

    void seek(size_t pos) {
        if (mError) return;

        if (pos > mSize) {
            mError = true;
        } else {
            mPos = pos;
        }
    }

    size_t remaining() const {
        if (mError) return 0;
        return mSize - mPos;
    }

    bool error() const { return mError; }

private:
    const uint8_t* mBuffer;
    size_t mSize;
    size_t mPos;
    bool mError;
};

bool isPostScriptNameAllowedChar(char c) {
    // OpenType spec says only ASCII codes 33 to 126, ecept for the '[', ']', '(', ')', '{', '}',
    // '<', '>', '/', '%'.
    if (!(33 <= c && c <= 126)) {
        return false;
    }
    if (c == '[' || c == ']' || c == '(' || c == ')' || c == '{' || c == '}' || c == '<' ||
        c == '>' || c == '/' || c == '%') {
        return false;
    }

    return true;
}

}  // namespace

// static
bool FontFileParser::analyzeFontRevision(const uint8_t* head_data, size_t head_size,
                                         uint32_t* out) {
    SafeFontBufferReader reader(head_data, head_size);

    if (reader.remaining() < 8) {
        return false;  // At least head table has 8 bytes, for version and fontRevision
    }

    uint32_t majorVersion = reader.readU16();
    if (reader.error()) return false;
    uint32_t minorVersion = reader.readU16();
    if (reader.error()) return false;

    // Invalid head table header.
    if (majorVersion != 1 && minorVersion != 0) return false;

    *out = reader.readU32();
    if (reader.error()) return false;
    return true;
}

// static
bool FontFileParser::checkPSName(const std::string& psName) {
    if (psName.size() > 63) return false;

    for (auto c : psName) {
        if (!isPostScriptNameAllowedChar(c)) {
            return false;
        }
    }
    return true;
}

FontFileParser::FontFileParser(const void* buffer, size_t size, uint32_t index)
        : mFace(makeHbFace(buffer, size, index)) {}

FontFileParser::FontFileParser(const HbFaceUniquePtr& face)
        : mFace(hb_face_reference(face.get())) {}

FontFileParser::FontFileParser(const HbFontUniquePtr& font)
        : mFace(hb_face_reference(hb_font_get_face(font.get()))) {}

FontFileParser::~FontFileParser() {}

// static
HbFaceUniquePtr FontFileParser::makeHbFace(const void* buffer, size_t size, uint32_t index) {
    HbBlobUniquePtr blob(hb_blob_create(reinterpret_cast<const char*>(buffer), size,
                                        HB_MEMORY_MODE_READONLY, nullptr, nullptr));
    return HbFaceUniquePtr(hb_face_create(blob.get(), index));
}

std::optional<uint32_t> FontFileParser::getFontRevision() const {
    if (!mFace) return std::optional<uint32_t>();

    HbBlob headTable(mFace, MinikinFont::MakeTag('h', 'e', 'a', 'd'));
    if (!headTable) return std::optional<uint32_t>();

    uint32_t out = 0;
    if (!analyzeFontRevision(headTable.get(), headTable.size(), &out)) {
        return std::optional<uint32_t>();
    }

    return out;
}

std::optional<std::string> FontFileParser::getPostScriptName() const {
    if (!mFace) return std::optional<std::string>();

    unsigned int size = 64;  // PostScript name is up to 63 characters.
    char buf[64] = {};

    uint32_t result = hb_ot_name_get_utf8(mFace.get(), HB_OT_NAME_ID_POSTSCRIPT_NAME,
                                          HB_LANGUAGE_INVALID, &size, buf);

    if (result == 0) {  // not found.
        return std::optional<std::string>();
    }

    std::string out(buf, size);

    if (!checkPSName(out)) {  // Contains invalid characters.
        return std::optional<std::string>();
    }

    return out;
}

std::optional<bool> FontFileParser::isPostScriptType1Font() const {
    if (!mFace) return std::optional<bool>();

    HbBlob cffTable(mFace, MinikinFont::MakeTag('C', 'F', 'F', ' '));
    HbBlob cff2Table(mFace, MinikinFont::MakeTag('C', 'F', 'F', '2'));
    return cffTable || cff2Table;
}

}  // namespace minikin
