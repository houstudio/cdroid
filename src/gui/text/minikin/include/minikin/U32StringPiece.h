/*
 * Copyright (C) 2017 The Android Open Source Project
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

#ifndef MINIKIN_U32STRING_PIECE_H
#define MINIKIN_U32STRING_PIECE_H

#include <vector>
#include <algorithm>

#include "minikin/Range.h"

namespace minikin {

class U32StringPiece {
public:
    U32StringPiece() : mData(nullptr), mLength(0) {}
    U32StringPiece(const char32_t* data, uint32_t length) : mData(data), mLength(length) {}
    U32StringPiece(const std::vector<char32_t>& v)  // Intentionally not explicit.
            : mData(v.data()), mLength(static_cast<uint32_t>(v.size())) {}
    template <uint32_t length>
    U32StringPiece(char32_t const (&data)[length]) : mData(data), mLength(length) {}

    U32StringPiece(const U32StringPiece&) = default;
    U32StringPiece& operator=(const U32StringPiece&) = default;

    inline const char32_t* data() const { return mData; }
    inline uint32_t size() const { return mLength; }
    inline uint32_t length() const { return mLength; }

    // Undefined behavior if pos is out of range.
    inline const char32_t& at(uint32_t pos) const { return mData[pos]; }
    inline const char32_t& operator[](uint32_t pos) const { return mData[pos]; }

    inline U32StringPiece substr(const Range& range) const {
        return U32StringPiece(mData + range.getStart(), range.getLength());
    }

    inline bool hasChar(char32_t c) const {
        const char32_t* end = mData + mLength;
        return std::find(mData, end, c) != end;
    }

private:
    const char32_t* mData;
    uint32_t mLength;
};

}  // namespace minikin

#endif  // MINIKIN_U32STRING_PIECE_H