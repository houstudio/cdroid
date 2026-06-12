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

#ifndef MINIKIN_FONT_FILE_PARSER_H
#define MINIKIN_FONT_FILE_PARSER_H

#include "minikin/HbUtils.h"

#include <optional>
#include <string>

namespace minikin {

// FontFileParser provides various parser logic for OpenType font file.
class FontFileParser {
public:
    // This class does not take an ownership of buffer. Caller must free it.
    FontFileParser(const void* buffer, size_t size, uint32_t index);
    explicit FontFileParser(const HbFaceUniquePtr& face);
    explicit FontFileParser(const HbFontUniquePtr& font);

    virtual ~FontFileParser();

    std::optional<uint32_t> getFontRevision() const;
    std::optional<std::string> getPostScriptName() const;
    std::optional<bool> isPostScriptType1Font() const;

protected:  // protected for testing purposes.
    static bool analyzeFontRevision(const uint8_t* head_data, size_t head_size, uint32_t* out);
    static bool checkPSName(const std::string& psName);

private:
    HbFaceUniquePtr mFace;

    static HbFaceUniquePtr makeHbFace(const void* buffer, size_t size, uint32_t index);
};

}  // namespace minikin

#endif  // MINIKIN_FONT_FILE_PARSER_H
