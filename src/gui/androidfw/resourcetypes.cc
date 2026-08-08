/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
//
// ResStringPool implementation — port of AOSP
// frameworks/base/libs/androidfw/ResourceTypes.cpp. See resourcetypes.h for the
// adaptation notes (incfs/base::expected/Mutex stripped, raw pointers, identity
// endian, self-contained utf8<->utf16).
#include "resourcetypes.h"

#include <cstdlib>
#include <cstring>
#include <porting/cdlog.h>   // unified project logging (LOGW/LOGD/...); backed by libtvhal
#include <sys/types.h> // ssize_t
#include <algorithm>   // std::lower_bound (sparse type entries)
#include "LocaleData.h"   // vendored android::localeData* (locale matching engine)

namespace cdroid {

// LOGW (and friends) come from <porting/cdlog.h> above: a printf-style macro
// backed by cdlog::LogMessage in libtvhal. Used for the ALOGW warnings ported
// from AOSP ResourceTypes.cpp.

// ---------------------------------------------------------------------------
// Self-contained UTF-8 <-> UTF-16 conversion.
// Equivalent to libutils/Unicode.cpp utf8_to_utf16{,_length}, kept local so this
// module builds with zero external dependencies.
// ---------------------------------------------------------------------------

// Number of bytes of the UTF-8 codepoint whose first byte is b.
static inline int utf8_codepoint_len(uint8_t b) {
    if ((b & 0x80U) == 0) return 1;
    if ((b & 0xE0U) == 0xC0U) return 2;
    if ((b & 0xF0U) == 0xE0U) return 3;
    if ((b & 0xF8U) == 0xF0U) return 4;
    return 1; // invalid lead byte: consume one, caller will skip
}

// Decode the u8len-byte UTF-8 sequence at u8cur into a UCS-4 codepoint.
static inline uint32_t utf8_to_utf32_codepoint(const uint8_t* u8cur, int u8len) {
    switch (u8len) {
        case 1: return u8cur[0];
        case 2: return ((uint32_t)(u8cur[0] & 0x1FU) << 6)  | (u8cur[1] & 0x3FU);
        case 3: return ((uint32_t)(u8cur[0] & 0x0FU) << 12) | ((uint32_t)(u8cur[1] & 0x3FU) << 6) | (u8cur[2] & 0x3FU);
        case 4: return ((uint32_t)(u8cur[0] & 0x07U) << 18) | ((uint32_t)(u8cur[1] & 0x3FU) << 12)
                     | ((uint32_t)(u8cur[2] & 0x3FU) << 6) | (u8cur[3] & 0x3FU);
        default: return u8cur[0];
    }
}

// Number of char16_t needed to represent the UTF-8 string; -1 if malformed.
static ssize_t utf8_to_utf16_length(const uint8_t* u8str, size_t u8len) {
    const uint8_t* const u8end = u8str + u8len;
    const uint8_t* u8cur = u8str;
    size_t u16measuredLen = 0;
    while (u8cur < u8end) {
        u16measuredLen++;
        int u8charLen = utf8_codepoint_len(*u8cur);
        if (u8cur + u8charLen - 1 >= u8end) return -1; // malformed: runs past end
        uint32_t codepoint = utf8_to_utf32_codepoint(u8cur, u8charLen);
        if (codepoint > 0xFFFF) u16measuredLen++; // surrogate pair
        u8cur += u8charLen;
    }
    if (u8cur != u8end) return -1;
    return (ssize_t)u16measuredLen;
}

// Write the UTF-8 string as UTF-16 into dst (up to dstLen char16_t). Returns the
// number of char16_t written (no NUL terminator appended).
static size_t utf8_to_utf16(const uint8_t* src, size_t srcLen, char16_t* dst, size_t dstLen) {
    const uint8_t* const u8end = src + srcLen;
    const uint8_t* u8cur = src;
    const char16_t* const u16end = dst + dstLen;
    char16_t* u16cur = dst;
    while (u8cur < u8end && u16cur < u16end) {
        size_t u8clen = utf8_codepoint_len(*u8cur);
        uint32_t codepoint = utf8_to_utf32_codepoint(u8cur, (int)u8clen);
        if (codepoint <= 0xFFFF) {
            *u16cur++ = (char16_t)codepoint;
        } else {
            codepoint -= 0x10000;
            *u16cur++ = (char16_t)((codepoint >> 10) + 0xD800);
            if (u16cur >= u16end) return (size_t)(u16cur - dst - 1); // no room for low surrogate
            *u16cur++ = (char16_t)((codepoint & 0x3FF) + 0xDC00);
        }
        u8cur += u8clen;
    }
    return (size_t)(u16cur - dst);
}

// ---------------------------------------------------------------------------
// Chunk validation (port of ResourceTypes.cpp validate_chunk).
// ---------------------------------------------------------------------------

static status_t validate_chunk(const ResChunk_header* chunk, size_t minSize,
                               const uint8_t* dataEnd, const char* name) {
    if (!chunk) return BAD_TYPE;
    const uint16_t headerSize = dtohs(chunk->headerSize);
    const uint32_t size = dtohl(chunk->size);
    if (headerSize >= minSize) {
        if (headerSize <= size) {
            if (((headerSize | size) & 0x3U) == 0) {
                if ((size_t)size <= (size_t)(dataEnd - (const uint8_t*)chunk)) {
                    return NO_ERROR;
                }
                LOGW("%s data size 0x%x extends beyond resource end %tu.",
                     name, size, dataEnd - (const uint8_t*)chunk);
                return BAD_TYPE;
            }
            LOGW("%s size 0x%x or headerSize 0x%x is not on an integer boundary.",
                 name, (int)size, (int)headerSize);
            return BAD_TYPE;
        }
        LOGW("%s size 0x%x is smaller than header size 0x%x.", name, size, headerSize);
        return BAD_TYPE;
    }
    LOGW("%s header size 0x%04x is too small.", name, headerSize);
    return BAD_TYPE;
}

// ---------------------------------------------------------------------------
// Length-prefix decoders (port of ResourceTypes.cpp decodeLength overloads).
// The pointer is advanced past the length bytes; the decoded length is returned.
// setTo() has already bounds-checked the pool, so these need no bounds check.
// ---------------------------------------------------------------------------

// UTF-16 length prefix: one or two uint16_t.
static inline size_t decodeLength(const uint16_t** str) {
    size_t len = dtohs((*str)[0]);
    if ((len & 0x8000U) != 0) {
        (*str)++;
        len = ((len & 0x7FFFU) << 16U) | dtohs((*str)[0]);
    }
    (*str)++;
    return len;
}

// UTF-8 length prefix: one or two uint8_t.
static inline size_t decodeLength(const uint8_t** str) {
    size_t len = (*str)[0];
    if ((len & 0x80U) != 0) {
        (*str)++;
        len = ((len & 0x7FU) << 8U) | (*str)[0];
    }
    (*str)++;
    return len;
}

// ---------------------------------------------------------------------------
// ResStringPool
// ---------------------------------------------------------------------------

ResStringPool::ResStringPool()
    : mError(NO_INIT), mOwnedData(nullptr), mHeader(nullptr), mSize(0),
      mEntries(nullptr), mEntryStyles(nullptr), mStrings(nullptr),
      mStringPoolSize(0), mStyles(nullptr), mStylePoolSize(0) {
}

ResStringPool::ResStringPool(const void* data, size_t size, bool copyData)
    : ResStringPool() {
    setTo(data, size, copyData);
}

ResStringPool::~ResStringPool() {
    uninit();
}

void ResStringPool::setToEmpty() {
    uninit();
    mOwnedData = calloc(1, sizeof(ResStringPool_header));
    ResStringPool_header* header = (ResStringPool_header*)mOwnedData;
    mSize = 0;
    mEntries = nullptr;
    mStrings = nullptr;
    mStringPoolSize = 0;
    mEntryStyles = nullptr;
    mStyles = nullptr;
    mStylePoolSize = 0;
    mHeader = header;
}

status_t ResStringPool::setTo(const void* data, size_t size, bool copyData) {
    if (!data || !size) {
        return (mError = BAD_TYPE);
    }

    uninit();

    // The chunk must be at least the size of the string pool header.
    if (size < sizeof(ResStringPool_header)) {
        LOGW("Bad string block: data size %zu is too small to be a string block", size);
        return (mError = BAD_TYPE);
    }

    const ResChunk_header* chunk_header = (const ResChunk_header*)data;
    if (validate_chunk(chunk_header, sizeof(ResStringPool_header),
                       (const uint8_t*)data + size, "ResStringPool_header") != NO_ERROR) {
        LOGW("Bad string block: malformed block dimensions");
        return (mError = BAD_TYPE);
    }

    // CDROID assumes a little-endian host and resources.arsc/AXML is little-endian,
    // so there is no device-endian swap to perform (AOSP's notDeviceEndian branch).

    const uint8_t* base = (const uint8_t*)data;
    if (copyData) {
        mOwnedData = malloc(size);
        if (mOwnedData == nullptr) {
            return (mError = NO_MEMORY);
        }
        memcpy(mOwnedData, data, size);
        base = (const uint8_t*)mOwnedData;
    }

    const ResStringPool_header* header = (const ResStringPool_header*)base;
    mHeader = header;

    if (header->header.headerSize > header->header.size || header->header.size > size) {
        LOGW("Bad string block: header size %d or total size %d is larger than data size %zu",
                (int)header->header.headerSize, (int)header->header.size, size);
        return (mError = BAD_TYPE);
    }

    mSize = header->header.size;
    mEntries = (const uint32_t*)(base + header->header.headerSize);

    if (header->stringCount > 0) {
        // uint32 overflow / bounds check on the string offset table.
        if ((header->stringCount * sizeof(uint32_t) < header->stringCount)
            || (header->header.headerSize + (header->stringCount * sizeof(uint32_t))) > size) {
            LOGW("Bad string block: entry of %d items extends past data size %zu",
                    (int)(header->header.headerSize + (header->stringCount * sizeof(uint32_t))), size);
            return (mError = BAD_TYPE);
        }

        size_t charSize = (header->flags & ResStringPool_header::UTF8_FLAG)
                          ? sizeof(uint8_t) : sizeof(uint16_t);

        // There should be at least space for the smallest string (length + NUL).
        if (header->stringsStart >= (mSize - sizeof(uint16_t))) {
            LOGW("Bad string block: string pool starts at %d, after total size %d",
                    (int)header->stringsStart, (int)header->header.size);
            return (mError = BAD_TYPE);
        }

        mStrings = base + header->stringsStart;
        if (header->styleCount == 0) {
            mStringPoolSize = (mSize - header->stringsStart) / charSize;
        } else {
            if (header->stylesStart >= (mSize - sizeof(uint16_t))) {
                LOGW("Bad style block: style block starts at %d past data size of %d",
                        (int)header->stylesStart, (int)header->header.size);
                return (mError = BAD_TYPE);
            }
            if (header->stylesStart <= header->stringsStart) {
                LOGW("Bad style block: style block starts at %d, before strings at %d",
                        (int)header->stylesStart, (int)header->stringsStart);
                return (mError = BAD_TYPE);
            }
            mStringPoolSize = (header->stylesStart - header->stringsStart) / charSize;
        }

        if (mStringPoolSize == 0) {
            LOGW("Bad string block: stringCount is %d but pool size is 0",
                  (int)header->stringCount);
            return (mError = BAD_TYPE);
        }

        // Check the last string is NUL-terminated.
        if (header->flags & ResStringPool_header::UTF8_FLAG) {
            const uint8_t* end = (const uint8_t*)mStrings + (mStringPoolSize - 1);
            if (end[0] != 0) {
                LOGW("Bad string block: last string is not 0-terminated");
                return (mError = BAD_TYPE);
            }
        } else {
            const uint16_t* end = (const uint16_t*)mStrings + (mStringPoolSize - 1);
            if (end[0] != 0) {
                LOGW("Bad string block: last string is not 0-terminated");
                return (mError = BAD_TYPE);
            }
        }
    } else {
        mStrings = nullptr;
        mStringPoolSize = 0;
    }

    if (header->styleCount > 0) {
        mEntryStyles = mEntries + header->stringCount;
        if (mEntryStyles < mEntries) { // integer overflow finding styles
            LOGW("Bad string block: integer overflow finding styles");
            return (mError = BAD_TYPE);
        }
        if ((size_t)((const uint8_t*)mEntryStyles - base) > size) {
            LOGW("Bad string block: entry of %d styles extends past data size %zu",
                    (int)((const uint8_t*)mEntryStyles - base), size);
            return (mError = BAD_TYPE);
        }
        mStyles = (const uint32_t*)(base + header->stylesStart);
        if (header->stylesStart >= header->header.size) {
            LOGW("Bad string block: style pool starts %d, after total size %d",
                    (int)header->stylesStart, (int)header->header.size);
            return (mError = BAD_TYPE);
        }
        mStylePoolSize = (header->header.size - header->stylesStart) / sizeof(uint32_t);

        // Check the last style is END-terminated.
        ResStringPool_span endSpan;
        endSpan.name.index = htodl(ResStringPool_span::END);
        endSpan.firstChar = htodl(ResStringPool_span::END);
        endSpan.lastChar = htodl(ResStringPool_span::END);
        const uint32_t spanWords = sizeof(endSpan) / sizeof(uint32_t);
        const ResStringPool_span* stylesEnd =
                (const ResStringPool_span*)(mStyles + (mStylePoolSize - spanWords));
        if (memcmp(stylesEnd, &endSpan, sizeof(endSpan)) != 0) {
            LOGW("Bad string block: last style is not 0xFFFFFFFF-terminated");
            return (mError = BAD_TYPE);
        }
    } else {
        mEntryStyles = nullptr;
        mStyles = nullptr;
        mStylePoolSize = 0;
    }

    return (mError = NO_ERROR);
}

status_t ResStringPool::getError() const {
    return mError;
}

void ResStringPool::uninit() {
    mError = NO_INIT;
    mCache.clear();
    if (mOwnedData) {
        free(mOwnedData);
        mOwnedData = nullptr;
    }
    mHeader = nullptr;
    mEntries = nullptr;
    mStrings = nullptr;
    mEntryStyles = nullptr;
    mStyles = nullptr;
    mSize = 0;
    mStringPoolSize = 0;
    mStylePoolSize = 0;
}

const char16_t* ResStringPool::stringAt(size_t idx, size_t* outLen) const {
    if (mError == NO_ERROR && idx < mHeader->stringCount) {
        const bool isUTF8 = (mHeader->flags & ResStringPool_header::UTF8_FLAG) != 0;
        const uint32_t off = mEntries[idx] / (isUTF8 ? sizeof(uint8_t) : sizeof(uint16_t));
        if (off < (mStringPoolSize - 1)) {
            if (!isUTF8) {
                const uint16_t* strings = (const uint16_t*)mStrings;
                const uint16_t* str = strings + off;

                size_t u16len = decodeLength(&str);

                if ((uint32_t)(str + u16len - strings) < mStringPoolSize) {
                    // Reject malformed (non-null-terminated) strings.
                    const uint16_t* nullAddress = str + u16len;
                    if (nullAddress[0] != 0x0000) {
                        LOGW("Bad string block: string #%zu is not null-terminated", idx);
                        return nullptr;
                    }
                    if (outLen) *outLen = u16len;
                    return reinterpret_cast<const char16_t*>(str);
                } else {
                    LOGW("Bad string block: string #%zu extends to %tu, past end at %u",
                            idx, str + u16len - strings, (unsigned)mStringPoolSize);
                }
            } else {
                const uint8_t* strings = (const uint8_t*)mStrings;
                const uint8_t* u8str = strings + off;

                size_t u16len = decodeLength(&u8str); // UTF-16 char count
                size_t u8len = decodeLength(&u8str);  // UTF-8 byte count

                if ((uint32_t)(u8str + u8len - strings) < mStringPoolSize) {
                    // Cache hit.
                    if (idx < mCache.size() && !mCache[idx].empty()) {
                        if (outLen) *outLen = mCache[idx].size();
                        return mCache[idx].c_str();
                    }

                    size_t decodedLen = 0;
                    const char* decoded = stringDecodeAt(idx, u8str, u8len, &decodedLen);
                    if (!decoded) return nullptr;

                    // Since AAPT truncated lengths longer than 0x7FFF, check that the
                    // bits remaining after truncation at least match the actual length.
                    ssize_t actualLen = utf8_to_utf16_length((const uint8_t*)decoded, decodedLen);
                    if (actualLen < 0 || ((size_t)actualLen & 0x7FFFU) != u16len) {
                        LOGW("Bad string block: string #%zu decoded length is not correct %zd vs %zu",
                                idx, actualLen, u16len);
                        return nullptr;
                    }

                    if (idx >= mCache.size()) mCache.resize(mHeader->stringCount);
                    mCache[idx].resize((size_t)actualLen);
                    utf8_to_utf16((const uint8_t*)decoded, decodedLen,
                                  &mCache[idx][0], (size_t)actualLen);

                    if (outLen) *outLen = (size_t)actualLen;
                    return mCache[idx].c_str();
                } else {
                    LOGW("Bad string block: string #%zu extends to %tu, past end at %u",
                            idx, u8str + u8len - strings, (unsigned)mStringPoolSize);
                }
            }
        } else {
            LOGW("Bad string block: string #%zu entry is at %u, past end at %u",
                    idx, (unsigned)(off * sizeof(uint16_t)), (unsigned)(mStringPoolSize * sizeof(uint16_t)));
        }
    }
    return nullptr;
}

const char* ResStringPool::string8At(size_t idx, size_t* outLen) const {
    if (mError == NO_ERROR && idx < mHeader->stringCount) {
        if ((mHeader->flags & ResStringPool_header::UTF8_FLAG) == 0) {
            return nullptr;
        }

        const uint32_t off = mEntries[idx] / sizeof(char);
        if (off < (mStringPoolSize - 1)) {
            const uint8_t* strings = (const uint8_t*)mStrings;
            const uint8_t* str = strings + off;

            // Decode (and skip) the UTF-16 length, then the UTF-8 byte length.
            size_t u16len = decodeLength(&str);
            (void)u16len;
            size_t u8len = decodeLength(&str);

            if ((uint32_t)(str + u8len - strings) < mStringPoolSize) {
                size_t decodedLen = 0;
                const char* decoded = stringDecodeAt(idx, str, u8len, &decodedLen);
                if (!decoded) return nullptr;
                if (outLen) *outLen = decodedLen;
                return decoded;
            } else {
                LOGW("Bad string block: string #%zu extends to %tu, past end at %u",
                        idx, str + u8len - strings, (unsigned)mStringPoolSize);
            }
        } else {
            LOGW("Bad string block: string #%zu entry is at %u, past end at %u",
                    idx, (unsigned)(off * sizeof(uint16_t)), (unsigned)(mStringPoolSize * sizeof(uint16_t)));
        }
    }
    return nullptr;
}

const char* ResStringPool::stringDecodeAt(size_t idx, const uint8_t* str, size_t encLen,
                                          size_t* outLen) const {
    const uint8_t* strings = (const uint8_t*)mStrings;
    size_t i = 0, end = encLen;
    while ((uint32_t)(str + end - strings) < mStringPoolSize) {
        const uint8_t* nullAddress = str + end;
        if (nullAddress[0] == 0x00) {
            if (i != 0) {
                LOGW("Bad string block: string #%zu is truncated (actual length is %zu)", idx, end);
            }
            if (outLen) *outLen = end;
            return (const char*)str;
        }
        end = (++i << (sizeof(uint8_t) * 8 * 2 - 1)) | encLen;
    }
    LOGW("Bad string block: string #%zu is not null-terminated", idx);
    return nullptr;
}

const ResStringPool_span* ResStringPool::styleAt(size_t idx) const {
    if (mError == NO_ERROR && idx < mHeader->styleCount) {
        const uint32_t off = mEntryStyles[idx] / sizeof(uint32_t);
        if (off < mStylePoolSize) {
            return (const ResStringPool_span*)(mStyles + off);
        } else {
            LOGW("Bad string block: style #%zu entry is at %u, past end at %u",
                    idx, (unsigned)(off * sizeof(uint32_t)), (unsigned)(mStylePoolSize * sizeof(uint32_t)));
        }
    }
    return nullptr;
}

size_t ResStringPool::size() const {
    return (mError == NO_ERROR) ? mHeader->stringCount : 0;
}

size_t ResStringPool::styleCount() const {
    return (mError == NO_ERROR) ? mHeader->styleCount : 0;
}

size_t ResStringPool::bytes() const {
    return (mError == NO_ERROR) ? mHeader->header.size : 0;
}

bool ResStringPool::isSorted() const {
    return mHeader && (mHeader->flags & ResStringPool_header::SORTED_FLAG) != 0;
}

bool ResStringPool::isUTF8() const {
    return mHeader && (mHeader->flags & ResStringPool_header::UTF8_FLAG) != 0;
}

// ===========================================================================
// Res_value
// ===========================================================================

void Res_value::copyFrom_dtoh(const Res_value& src) {
    size = dtohs(src.size);
    res0 = src.res0;
    dataType = src.dataType;
    data = dtohl(src.data);
}

// ===========================================================================
// ResTable_config — configuration matching engine (port of ResourceTypes.cpp).
// ===========================================================================

// ---------------------------------------------------------------------------
// DynamicRefTable — build-time -> run-time package id translation
// (port of ResourceTypes.cpp). load() / staged aliases deferred.
// ---------------------------------------------------------------------------

DynamicRefTable::DynamicRefTable() : DynamicRefTable(0, false) {}

DynamicRefTable::DynamicRefTable(uint8_t packageId, bool appAsLib)
    : mAssignedPackageId(packageId), mAppAsLib(appAsLib) {
    memset(mLookupTable, 0, sizeof(mLookupTable));
    mLookupTable[APP_PACKAGE_ID] = APP_PACKAGE_ID;
    mLookupTable[SYS_PACKAGE_ID] = SYS_PACKAGE_ID;
}

void DynamicRefTable::addMapping(uint8_t buildPackageId, uint8_t runtimePackageId) {
    mLookupTable[buildPackageId] = runtimePackageId;
}

status_t DynamicRefTable::addMapping(const std::string& packageName, uint8_t packageId) {
    auto it = mEntries.find(packageName);
    if (it == mEntries.end()) return UNKNOWN_ERROR;
    mLookupTable[it->second] = packageId;
    return NO_ERROR;
}

status_t DynamicRefTable::addMappings(const DynamicRefTable& other) {
    if (mAssignedPackageId != other.mAssignedPackageId) return UNKNOWN_ERROR;
    for (const auto& kv : other.mEntries) {
        auto it = mEntries.find(kv.first);
        if (it == mEntries.end()) {
            mEntries[kv.first] = kv.second;
        } else if (it->second != kv.second) {
            return UNKNOWN_ERROR;
        }
    }
    for (size_t i = 0; i < 256; i++) {
        if (mLookupTable[i] != other.mLookupTable[i]) {
            if (mLookupTable[i] == 0) mLookupTable[i] = other.mLookupTable[i];
            else if (other.mLookupTable[i] != 0) return UNKNOWN_ERROR;
        }
    }
    return NO_ERROR;
}

status_t DynamicRefTable::lookupResourceId(uint32_t* resId) const {
    uint32_t res = *resId;
    size_t packageId = Res_GETPACKAGE(res) + 1;  // 1-based package byte

    if (!Res_VALIDID(res)) return NO_ERROR;  // null/invalid: nothing to do

    if (packageId == SYS_PACKAGE_ID || (packageId == APP_PACKAGE_ID && !mAppAsLib)) {
        // App and framework package ids are absolute.
        *resId = res;
        return NO_ERROR;
    }
    if (packageId == 0 || (packageId == APP_PACKAGE_ID && mAppAsLib)) {
        // A shared library accessing its own local resource: fix up to our id.
        *resId = (0xFFFFFF & res) | (((uint32_t)mAssignedPackageId) << 24);
        return NO_ERROR;
    }

    uint8_t translatedId = mLookupTable[packageId];
    if (translatedId == 0) {
        LOGW("DynamicRefTable(0x%02x): No mapping for build-time package ID 0x%02x.",
             (uint8_t)mAssignedPackageId, (uint8_t)packageId);
        return UNKNOWN_ERROR;
    }
    *resId = (res & 0x00ffffff) | (((uint32_t)translatedId) << 24);
    return NO_ERROR;
}

bool DynamicRefTable::requiresLookup(const Res_value* value) const {
    if ((value->dataType == Res_value::TYPE_REFERENCE ||
         value->dataType == Res_value::TYPE_ATTRIBUTE) &&
        (mAppAsLib || (Res_GETPACKAGE(value->data) + 1) == 0)) {
        return true;
    }
    return value->dataType == Res_value::TYPE_DYNAMIC_ATTRIBUTE ||
           value->dataType == Res_value::TYPE_DYNAMIC_REFERENCE;
}

status_t DynamicRefTable::lookupResourceValue(Res_value* value) const {
    if (!requiresLookup(value)) return NO_ERROR;

    uint8_t resolvedType = Res_value::TYPE_REFERENCE;
    switch (value->dataType) {
        case Res_value::TYPE_ATTRIBUTE:        resolvedType = Res_value::TYPE_ATTRIBUTE; break;
        case Res_value::TYPE_REFERENCE:        break;
        case Res_value::TYPE_DYNAMIC_ATTRIBUTE: resolvedType = Res_value::TYPE_ATTRIBUTE; break;
        case Res_value::TYPE_DYNAMIC_REFERENCE: break;
        default: return NO_ERROR;
    }
    status_t err = lookupResourceId(&value->data);
    if (err != NO_ERROR) return err;
    value->dataType = resolvedType;
    return NO_ERROR;
}



// Unpack a packed 2-byte language/region into a NUL-terminated char[4]. Returns
// the number of significant chars (0, 2, or 3).
static size_t unpackLanguageOrRegion(const char in[2], const char base, char out[4]) {
    if (in[0] & 0x80) {
        // Packed three-letter code.
        const uint8_t first  = in[1] & 0x1f;
        const uint8_t second = ((in[1] & 0xe0) >> 5) + ((in[0] & 0x03) << 3);
        const uint8_t third  = (in[0] & 0x7c) >> 2;
        out[0] = (char)(first + base);
        out[1] = (char)(second + base);
        out[2] = (char)(third + base);
        out[3] = 0;
        return 3;
    }
    if (in[0]) {
        memcpy(out, in, 2);
        memset(out + 2, 0, 2);
        return 2;
    }
    memset(out, 0, 4);
    return 0;
}

// Pack a language/region string into the 2-byte form (in place into out[2]).
static void packLanguageOrRegion(const char* in, const char base, char out[2]) {
    if (in[2] == 0 || in[2] == '-') {
        out[0] = in[0];
        out[1] = in[1];
    } else {
        uint8_t first  = (uint8_t)(in[0] - base) & 0x007f;
        uint8_t second = (uint8_t)(in[1] - base) & 0x007f;
        uint8_t third  = (uint8_t)(in[2] - base) & 0x007f;
        out[0] = (char)(0x80 | (third << 2) | (second >> 3));
        out[1] = (char)((second << 5) | first);
    }
}

void ResTable_config::packLanguage(const char* language) {
    packLanguageOrRegion(language, 'a', this->language);
}
void ResTable_config::packRegion(const char* region) {
    packLanguageOrRegion(region, '0', this->country);
}
size_t ResTable_config::unpackLanguage(char language[4]) const {
    return unpackLanguageOrRegion(this->language, 'a', language);
}
size_t ResTable_config::unpackRegion(char region[4]) const {
    return unpackLanguageOrRegion(this->country, '0', region);
}

void ResTable_config::copyFromDeviceNoSwap(const ResTable_config& o) {
    const size_t sz = dtohl(o.size);
    if (sz >= sizeof(ResTable_config)) {
        *this = o;
    } else {
        memcpy(this, &o, sz);
        memset(((uint8_t*)this) + sz, 0, sizeof(ResTable_config) - sz);
    }
}

void ResTable_config::copyFromDtoH(const ResTable_config& o) {
    copyFromDeviceNoSwap(o);
    size = sizeof(ResTable_config);
    mcc = dtohs(mcc);
    mnc = dtohs(mnc);
    density = dtohs(density);
    screenWidth = dtohs(screenWidth);
    screenHeight = dtohs(screenHeight);
    sdkVersion = dtohs(sdkVersion);
    minorVersion = dtohs(minorVersion);
    smallestScreenWidthDp = dtohs(smallestScreenWidthDp);
    screenWidthDp = dtohs(screenWidthDp);
    screenHeightDp = dtohs(screenHeightDp);
}

// Compare locale (language+region packed uint32, then script/variant/numsys).
static int compareLocales(const ResTable_config& l, const ResTable_config& r) {
    if (l.locale != r.locale) {
        return (l.locale > r.locale) ? 1 : -1;
    }
    const char emptyScript[4] = {'\0', '\0', '\0', '\0'};
    const char* lScript = l.localeScriptWasComputed ? emptyScript : l.localeScript;
    const char* rScript = r.localeScriptWasComputed ? emptyScript : r.localeScript;
    int script = memcmp(lScript, rScript, sizeof(l.localeScript));
    if (script) return script;
    int variant = memcmp(l.localeVariant, r.localeVariant, sizeof(l.localeVariant));
    if (variant) return variant;
    return memcmp(l.localeNumberingSystem, r.localeNumberingSystem,
                  sizeof(l.localeNumberingSystem));
}

int ResTable_config::compare(const ResTable_config& o) const {
    if (imsi != o.imsi) return (imsi > o.imsi) ? 1 : -1;
    int32_t d = compareLocales(*this, o);
    if (d < 0) return -1;
    if (d > 0) return 1;
    if (screenType != o.screenType) return (screenType > o.screenType) ? 1 : -1;
    if (input != o.input) return (input > o.input) ? 1 : -1;
    if (screenSize != o.screenSize) return (screenSize > o.screenSize) ? 1 : -1;
    if (version != o.version) return (version > o.version) ? 1 : -1;
    if (screenLayout != o.screenLayout) return (screenLayout > o.screenLayout) ? 1 : -1;
    if (screenLayout2 != o.screenLayout2) return (screenLayout2 > o.screenLayout2) ? 1 : -1;
    if (colorMode != o.colorMode) return (colorMode > o.colorMode) ? 1 : -1;
    if (uiMode != o.uiMode) return (uiMode > o.uiMode) ? 1 : -1;
    if (smallestScreenWidthDp != o.smallestScreenWidthDp)
        return (smallestScreenWidthDp > o.smallestScreenWidthDp) ? 1 : -1;
    if (screenSizeDp != o.screenSizeDp) return (screenSizeDp > o.screenSizeDp) ? 1 : -1;
    return 0;
}

int ResTable_config::compareLogical(const ResTable_config& o) const {
    if (mcc != o.mcc) return mcc < o.mcc ? -1 : 1;
    if (mnc != o.mnc) return mnc < o.mnc ? -1 : 1;
    int d = compareLocales(*this, o);
    if (d < 0) return -1;
    if (d > 0) return 1;
    if ((screenLayout & MASK_LAYOUTDIR) != (o.screenLayout & MASK_LAYOUTDIR))
        return (screenLayout & MASK_LAYOUTDIR) < (o.screenLayout & MASK_LAYOUTDIR) ? -1 : 1;
    if (smallestScreenWidthDp != o.smallestScreenWidthDp)
        return smallestScreenWidthDp < o.smallestScreenWidthDp ? -1 : 1;
    if (screenWidthDp != o.screenWidthDp) return screenWidthDp < o.screenWidthDp ? -1 : 1;
    if (screenHeightDp != o.screenHeightDp) return screenHeightDp < o.screenHeightDp ? -1 : 1;
    if (screenWidth != o.screenWidth) return screenWidth < o.screenWidth ? -1 : 1;
    if (screenHeight != o.screenHeight) return screenHeight < o.screenHeight ? -1 : 1;
    if (density != o.density) return density < o.density ? -1 : 1;
    if (orientation != o.orientation) return orientation < o.orientation ? -1 : 1;
    if (touchscreen != o.touchscreen) return touchscreen < o.touchscreen ? -1 : 1;
    if (input != o.input) return input < o.input ? -1 : 1;
    if (screenLayout != o.screenLayout) return screenLayout < o.screenLayout ? -1 : 1;
    if (screenLayout2 != o.screenLayout2) return screenLayout2 < o.screenLayout2 ? -1 : 1;
    if (colorMode != o.colorMode) return colorMode < o.colorMode ? -1 : 1;
    if (uiMode != o.uiMode) return uiMode < o.uiMode ? -1 : 1;
    if (version != o.version) return version < o.version ? -1 : 1;
    return 0;
}

int ResTable_config::diff(const ResTable_config& o) const {
    int diffs = 0;
    if (mcc != o.mcc) diffs |= CONFIG_MCC;
    if (mnc != o.mnc) diffs |= CONFIG_MNC;
    if (orientation != o.orientation) diffs |= CONFIG_ORIENTATION;
    if (density != o.density) diffs |= CONFIG_DENSITY;
    if (touchscreen != o.touchscreen) diffs |= CONFIG_TOUCHSCREEN;
    if (((inputFlags ^ o.inputFlags) & (MASK_KEYSHIDDEN | MASK_NAVHIDDEN)) != 0)
        diffs |= CONFIG_KEYBOARD_HIDDEN;
    if (keyboard != o.keyboard) diffs |= CONFIG_KEYBOARD;
    if (navigation != o.navigation) diffs |= CONFIG_NAVIGATION;
    if (screenSize != o.screenSize) diffs |= CONFIG_SCREEN_SIZE;
    if (version != o.version) diffs |= CONFIG_VERSION;
    if ((screenLayout & MASK_LAYOUTDIR) != (o.screenLayout & MASK_LAYOUTDIR)) diffs |= CONFIG_LAYOUTDIR;
    if ((screenLayout & ~MASK_LAYOUTDIR) != (o.screenLayout & ~MASK_LAYOUTDIR)) diffs |= CONFIG_SCREEN_LAYOUT;
    if ((screenLayout2 & MASK_SCREENROUND) != (o.screenLayout2 & MASK_SCREENROUND)) diffs |= CONFIG_SCREEN_ROUND;
    if ((colorMode & MASK_WIDE_COLOR_GAMUT) != (o.colorMode & MASK_WIDE_COLOR_GAMUT)) diffs |= CONFIG_COLOR_MODE;
    if ((colorMode & MASK_HDR) != (o.colorMode & MASK_HDR)) diffs |= CONFIG_COLOR_MODE;
    if (uiMode != o.uiMode) diffs |= CONFIG_UI_MODE;
    if (smallestScreenWidthDp != o.smallestScreenWidthDp) diffs |= CONFIG_SMALLEST_SCREEN_SIZE;
    if (screenSizeDp != o.screenSizeDp) diffs |= CONFIG_SCREEN_SIZE;
    if (compareLocales(*this, o)) diffs |= CONFIG_LOCALE;
    return diffs;
}

inline int getImportanceScoreOfLocale(const ResTable_config& c) {
    return (c.localeVariant[0] ? 4 : 0)
         + (c.localeScript[0] && !c.localeScriptWasComputed ? 2 : 0)
         + (c.localeNumberingSystem[0] ? 1 : 0);
}

int ResTable_config::isLocaleMoreSpecificThan(const ResTable_config& o) const {
    if (locale || o.locale) {
        if (language[0] != o.language[0]) {
            if (!language[0]) return -1;
            if (!o.language[0]) return 1;
        }
        if (country[0] != o.country[0]) {
            if (!country[0]) return -1;
            if (!o.country[0]) return 1;
        }
    }
    return getImportanceScoreOfLocale(*this) - getImportanceScoreOfLocale(o);
}

bool ResTable_config::isMoreSpecificThan(const ResTable_config& o) const {
    if (imsi || o.imsi) {
        if (mcc != o.mcc) { if (!mcc) return false; if (!o.mcc) return true; }
        if (mnc != o.mnc) { if (!mnc) return false; if (!o.mnc) return true; }
    }
    if (locale || o.locale) {
        const int d = isLocaleMoreSpecificThan(o);
        if (d < 0) return false;
        if (d > 0) return true;
    }
    if (screenLayout || o.screenLayout) {
        if (((screenLayout ^ o.screenLayout) & MASK_LAYOUTDIR) != 0) {
            if (!(screenLayout & MASK_LAYOUTDIR)) return false;
            if (!(o.screenLayout & MASK_LAYOUTDIR)) return true;
        }
    }
    if (smallestScreenWidthDp || o.smallestScreenWidthDp) {
        if (smallestScreenWidthDp != o.smallestScreenWidthDp) {
            if (!smallestScreenWidthDp) return false;
            if (!o.smallestScreenWidthDp) return true;
        }
    }
    if (screenSizeDp || o.screenSizeDp) {
        if (screenWidthDp != o.screenWidthDp) { if (!screenWidthDp) return false; if (!o.screenWidthDp) return true; }
        if (screenHeightDp != o.screenHeightDp) { if (!screenHeightDp) return false; if (!o.screenHeightDp) return true; }
    }
    if (screenLayout || o.screenLayout) {
        if (((screenLayout ^ o.screenLayout) & MASK_SCREENSIZE) != 0) {
            if (!(screenLayout & MASK_SCREENSIZE)) return false;
            if (!(o.screenLayout & MASK_SCREENSIZE)) return true;
        }
        if (((screenLayout ^ o.screenLayout) & MASK_SCREENLONG) != 0) {
            if (!(screenLayout & MASK_SCREENLONG)) return false;
            if (!(o.screenLayout & MASK_SCREENLONG)) return true;
        }
    }
    if (screenLayout2 || o.screenLayout2) {
        if (((screenLayout2 ^ o.screenLayout2) & MASK_SCREENROUND) != 0) {
            if (!(screenLayout2 & MASK_SCREENROUND)) return false;
            if (!(o.screenLayout2 & MASK_SCREENROUND)) return true;
        }
    }
    if (colorMode || o.colorMode) {
        if (((colorMode ^ o.colorMode) & MASK_HDR) != 0) {
            if (!(colorMode & MASK_HDR)) return false;
            if (!(o.colorMode & MASK_HDR)) return true;
        }
        if (((colorMode ^ o.colorMode) & MASK_WIDE_COLOR_GAMUT) != 0) {
            if (!(colorMode & MASK_WIDE_COLOR_GAMUT)) return false;
            if (!(o.colorMode & MASK_WIDE_COLOR_GAMUT)) return true;
        }
    }
    if (orientation != o.orientation) { if (!orientation) return false; if (!o.orientation) return true; }
    if (uiMode || o.uiMode) {
        if (((uiMode ^ o.uiMode) & MASK_UI_MODE_TYPE) != 0) {
            if (!(uiMode & MASK_UI_MODE_TYPE)) return false;
            if (!(o.uiMode & MASK_UI_MODE_TYPE)) return true;
        }
        if (((uiMode ^ o.uiMode) & MASK_UI_MODE_NIGHT) != 0) {
            if (!(uiMode & MASK_UI_MODE_NIGHT)) return false;
            if (!(o.uiMode & MASK_UI_MODE_NIGHT)) return true;
        }
    }
    // density is never 'more specific' (default equals 160).
    if (touchscreen != o.touchscreen) { if (!touchscreen) return false; if (!o.touchscreen) return true; }
    if (input || o.input) {
        if (((inputFlags ^ o.inputFlags) & MASK_KEYSHIDDEN) != 0) {
            if (!(inputFlags & MASK_KEYSHIDDEN)) return false;
            if (!(o.inputFlags & MASK_KEYSHIDDEN)) return true;
        }
        if (((inputFlags ^ o.inputFlags) & MASK_NAVHIDDEN) != 0) {
            if (!(inputFlags & MASK_NAVHIDDEN)) return false;
            if (!(o.inputFlags & MASK_NAVHIDDEN)) return true;
        }
        if (keyboard != o.keyboard) { if (!keyboard) return false; if (!o.keyboard) return true; }
        if (navigation != o.navigation) { if (!navigation) return false; if (!o.navigation) return true; }
    }
    if (screenSize || o.screenSize) {
        if (screenWidth != o.screenWidth) { if (!screenWidth) return false; if (!o.screenWidth) return true; }
        if (screenHeight != o.screenHeight) { if (!screenHeight) return false; if (!o.screenHeight) return true; }
    }
    if (version || o.version) {
        if (sdkVersion != o.sdkVersion) { if (!sdkVersion) return false; if (!o.sdkVersion) return true; }
        if (minorVersion != o.minorVersion) { if (!minorVersion) return false; if (!o.minorVersion) return true; }
    }
    return false;
}

// Codes for specially handled languages/regions.
static const char kEnglish[2]     = {'e', 'n'};
static const char kUnitedStates[2] = {'U', 'S'};
static const char kFilipino[2]   = {'\xAD', '\x05'}; // packed "fil"
static const char kTagalog[2]    = {'t', 'l'};

static inline bool areIdentical(const char c1[2], const char c2[2]) {
    return c1[0] == c2[0] && c1[1] == c2[1];
}
static inline bool langsAreEquivalent(const char l1[2], const char l2[2]) {
    return areIdentical(l1, l2)
        || (areIdentical(l1, kTagalog) && areIdentical(l2, kFilipino))
        || (areIdentical(l1, kFilipino) && areIdentical(l2, kTagalog));
}

bool ResTable_config::isLocaleBetterThan(const ResTable_config& o,
                                         const ResTable_config* requested) const {
    if (requested->locale == 0) return false;
    if (locale == 0 && o.locale == 0) return false;

    if (!langsAreEquivalent(language, o.language)) {
        // One has a matching language, the other doesn't.
        if (areIdentical(requested->language, kEnglish)) {
            if (areIdentical(requested->country, kUnitedStates)) {
                if (language[0] != '\0') {
                    return country[0] == '\0' || areIdentical(country, kUnitedStates);
                } else {
                    return !(o.country[0] == '\0' || areIdentical(o.country, kUnitedStates));
                }
            } else if (android::localeDataIsCloseToUsEnglish(requested->country)) {
                if (language[0] != '\0') {
                    return android::localeDataIsCloseToUsEnglish(country);
                } else {
                    return !android::localeDataIsCloseToUsEnglish(o.country);
                }
            }
        }
        return (language[0] != '\0');
    }

    const int region_comparison = android::localeDataCompareRegions(
            country, o.country,
            requested->language, requested->localeScript, requested->country);
    if (region_comparison != 0) {
        return (region_comparison > 0);
    }

    const bool localeMatches = memcmp(localeVariant, requested->localeVariant,
                                      sizeof(localeVariant)) == 0;
    const bool otherMatches = memcmp(o.localeVariant, requested->localeVariant,
                                     sizeof(localeVariant)) == 0;
    if (localeMatches != otherMatches) {
        return localeMatches;
    }

    const bool localeNumsysMatches = memcmp(localeNumberingSystem,
                                            requested->localeNumberingSystem,
                                            sizeof(localeNumberingSystem)) == 0;
    const bool otherNumsysMatches = memcmp(o.localeNumberingSystem,
                                           requested->localeNumberingSystem,
                                           sizeof(localeNumberingSystem)) == 0;
    if (localeNumsysMatches != otherNumsysMatches) {
        return localeNumsysMatches;
    }

    if (areIdentical(language, requested->language)
            && !areIdentical(o.language, requested->language)) {
        return true;
    }
    return false;
}

bool ResTable_config::isBetterThan(const ResTable_config& o,
                                   const ResTable_config* requested) const {
    if (requested) {
        if (imsi || o.imsi) {
            if ((mcc != o.mcc) && requested->mcc) return (mcc);
            if ((mnc != o.mnc) && requested->mnc) return (mnc);
        }
        if (isLocaleBetterThan(o, requested)) return true;

        if (screenLayout || o.screenLayout) {
            if (((screenLayout ^ o.screenLayout) & MASK_LAYOUTDIR) != 0
                    && (requested->screenLayout & MASK_LAYOUTDIR)) {
                int myLayoutDir = screenLayout & MASK_LAYOUTDIR;
                int oLayoutDir = o.screenLayout & MASK_LAYOUTDIR;
                return (myLayoutDir > oLayoutDir);
            }
        }
        if (smallestScreenWidthDp || o.smallestScreenWidthDp) {
            if (smallestScreenWidthDp != o.smallestScreenWidthDp) {
                return smallestScreenWidthDp > o.smallestScreenWidthDp;
            }
        }
        if (screenSizeDp || o.screenSizeDp) {
            int myDelta = 0, otherDelta = 0;
            if (requested->screenWidthDp) {
                myDelta += requested->screenWidthDp - screenWidthDp;
                otherDelta += requested->screenWidthDp - o.screenWidthDp;
            }
            if (requested->screenHeightDp) {
                myDelta += requested->screenHeightDp - screenHeightDp;
                otherDelta += requested->screenHeightDp - o.screenHeightDp;
            }
            if (myDelta != otherDelta) return myDelta < otherDelta;
        }
        if (screenLayout || o.screenLayout) {
            if (((screenLayout ^ o.screenLayout) & MASK_SCREENSIZE) != 0
                    && (requested->screenLayout & MASK_SCREENSIZE)) {
                int mySL = (screenLayout & MASK_SCREENSIZE);
                int oSL = (o.screenLayout & MASK_SCREENSIZE);
                int fixedMySL = mySL, fixedOSL = oSL;
                if ((requested->screenLayout & MASK_SCREENSIZE) >= SCREENSIZE_NORMAL) {
                    if (fixedMySL == 0) fixedMySL = SCREENSIZE_NORMAL;
                    if (fixedOSL == 0) fixedOSL = SCREENSIZE_NORMAL;
                }
                if (fixedMySL == fixedOSL) {
                    if (mySL == 0) return false;
                    return true;
                }
                if (fixedMySL != fixedOSL) return fixedMySL > fixedOSL;
            }
            if (((screenLayout ^ o.screenLayout) & MASK_SCREENLONG) != 0
                    && (requested->screenLayout & MASK_SCREENLONG)) {
                return (screenLayout & MASK_SCREENLONG);
            }
        }
        if (screenLayout2 || o.screenLayout2) {
            if (((screenLayout2 ^ o.screenLayout2) & MASK_SCREENROUND) != 0 &&
                    (requested->screenLayout2 & MASK_SCREENROUND)) {
                return screenLayout2 & MASK_SCREENROUND;
            }
        }
        if (colorMode || o.colorMode) {
            if (((colorMode ^ o.colorMode) & MASK_WIDE_COLOR_GAMUT) != 0 &&
                    (requested->colorMode & MASK_WIDE_COLOR_GAMUT)) {
                return colorMode & MASK_WIDE_COLOR_GAMUT;
            }
            if (((colorMode ^ o.colorMode) & MASK_HDR) != 0 &&
                    (requested->colorMode & MASK_HDR)) {
                return colorMode & MASK_HDR;
            }
        }
        if ((orientation != o.orientation) && requested->orientation) return (orientation);

        if (uiMode || o.uiMode) {
            if (((uiMode ^ o.uiMode) & MASK_UI_MODE_TYPE) != 0
                    && (requested->uiMode & MASK_UI_MODE_TYPE)) {
                return (uiMode & MASK_UI_MODE_TYPE);
            }
            if (((uiMode ^ o.uiMode) & MASK_UI_MODE_NIGHT) != 0
                    && (requested->uiMode & MASK_UI_MODE_NIGHT)) {
                return (uiMode & MASK_UI_MODE_NIGHT);
            }
        }

        if (screenType || o.screenType) {
            if (density != o.density) {
                // Use DENSITY_MEDIUM (160) if none specified.
                const int thisDensity = density ? (int)density : (int)DENSITY_MEDIUM;
                const int otherDensity = o.density ? (int)o.density : (int)DENSITY_MEDIUM;
                if (thisDensity == DENSITY_ANY) return true;
                if (otherDensity == DENSITY_ANY) return false;
                int requestedDensity = requested->density;
                if (requested->density == 0 || requested->density == DENSITY_ANY) {
                    requestedDensity = DENSITY_MEDIUM;
                }
                int h = thisDensity;
                int l = otherDensity;
                bool bImBigger = true;
                if (l > h) { int t = h; h = l; l = t; bImBigger = false; }
                if (requestedDensity >= h) return bImBigger;   // request >= both: take h
                if (l >= requestedDensity) return !bImBigger;  // request <= both: take l
                // scaling down is 2x better than up
                if (((2 * l) - requestedDensity) * h > requestedDensity * requestedDensity) {
                    return !bImBigger;
                }
                return bImBigger;
            }
            if ((touchscreen != o.touchscreen) && requested->touchscreen) return (touchscreen);
        }

        if (input || o.input) {
            const int keysHidden = inputFlags & MASK_KEYSHIDDEN;
            const int oKeysHidden = o.inputFlags & MASK_KEYSHIDDEN;
            if (keysHidden != oKeysHidden) {
                const int reqKeysHidden = requested->inputFlags & MASK_KEYSHIDDEN;
                if (reqKeysHidden) {
                    if (!keysHidden) return false;
                    if (!oKeysHidden) return true;
                    if (reqKeysHidden == keysHidden) return true;
                    if (reqKeysHidden == oKeysHidden) return false;
                }
            }
            const int navHidden = inputFlags & MASK_NAVHIDDEN;
            const int oNavHidden = o.inputFlags & MASK_NAVHIDDEN;
            if (navHidden != oNavHidden) {
                const int reqNavHidden = requested->inputFlags & MASK_NAVHIDDEN;
                if (reqNavHidden) {
                    if (!navHidden) return false;
                    if (!oNavHidden) return true;
                }
            }
            if ((keyboard != o.keyboard) && requested->keyboard) return (keyboard);
            if ((navigation != o.navigation) && requested->navigation) return (navigation);
        }

        if (screenSize || o.screenSize) {
            int myDelta = 0, otherDelta = 0;
            if (requested->screenWidth) {
                myDelta += requested->screenWidth - screenWidth;
                otherDelta += requested->screenWidth - o.screenWidth;
            }
            if (requested->screenHeight) {
                myDelta += requested->screenHeight - screenHeight;
                otherDelta += requested->screenHeight - o.screenHeight;
            }
            if (myDelta != otherDelta) return myDelta < otherDelta;
        }

        if (version || o.version) {
            if ((sdkVersion != o.sdkVersion) && requested->sdkVersion) return (sdkVersion > o.sdkVersion);
            if ((minorVersion != o.minorVersion) && requested->minorVersion) return (minorVersion);
        }
        return false;
    }
    return isMoreSpecificThan(o);
}

bool ResTable_config::match(const ResTable_config& settings) const {
    if (imsi != 0) {
        if (mcc != 0 && mcc != settings.mcc) return false;
        if (mnc != 0 && mnc != settings.mnc) return false;
    }
    if (locale != 0) {
        if (!langsAreEquivalent(language, settings.language)) return false;

        bool countriesMustMatch = false;
        char computed_script[4];
        const char* script = nullptr;
        if (settings.localeScript[0] == '\0') {
            countriesMustMatch = true;
        } else {
            if (localeScript[0] == '\0' && !localeScriptWasComputed) {
                android::localeDataComputeScript(computed_script, language, country);
                if (computed_script[0] == '\0') {
                    countriesMustMatch = true;
                } else {
                    script = computed_script;
                }
            } else {
                script = localeScript;
            }
        }

        if (countriesMustMatch) {
            if (country[0] != '\0' && !areIdentical(country, settings.country)) return false;
        } else {
            if (memcmp(script, settings.localeScript, sizeof(settings.localeScript)) != 0) return false;
        }
    }
    if (screenConfig != 0) {
        const int layoutDir = screenLayout & MASK_LAYOUTDIR;
        const int setLayoutDir = settings.screenLayout & MASK_LAYOUTDIR;
        if (layoutDir != 0 && layoutDir != setLayoutDir) return false;

        const int screenSizeCls = screenLayout & MASK_SCREENSIZE;
        const int setScreenSize = settings.screenLayout & MASK_SCREENSIZE;
        if (screenSizeCls != 0 && screenSizeCls > setScreenSize) return false;

        const int screenLong = screenLayout & MASK_SCREENLONG;
        const int setScreenLong = settings.screenLayout & MASK_SCREENLONG;
        if (screenLong != 0 && screenLong != setScreenLong) return false;

        const int uiModeType = uiMode & MASK_UI_MODE_TYPE;
        const int setUiModeType = settings.uiMode & MASK_UI_MODE_TYPE;
        if (uiModeType != 0 && uiModeType != setUiModeType) return false;

        const int uiModeNight = uiMode & MASK_UI_MODE_NIGHT;
        const int setUiModeNight = settings.uiMode & MASK_UI_MODE_NIGHT;
        if (uiModeNight != 0 && uiModeNight != setUiModeNight) return false;

        if (smallestScreenWidthDp != 0 && smallestScreenWidthDp > settings.smallestScreenWidthDp) return false;
    }
    if (screenConfig2 != 0) {
        const int screenRound = screenLayout2 & MASK_SCREENROUND;
        const int setScreenRound = settings.screenLayout2 & MASK_SCREENROUND;
        if (screenRound != 0 && screenRound != setScreenRound) return false;
        const int hdr = colorMode & MASK_HDR;
        const int setHdr = settings.colorMode & MASK_HDR;
        if (hdr != 0 && hdr != setHdr) return false;
        const int wideColorGamut = colorMode & MASK_WIDE_COLOR_GAMUT;
        const int setWideColorGamut = settings.colorMode & MASK_WIDE_COLOR_GAMUT;
        if (wideColorGamut != 0 && wideColorGamut != setWideColorGamut) return false;
    }
    if (screenSizeDp != 0) {
        if (screenWidthDp != 0 && screenWidthDp > settings.screenWidthDp) return false;
        if (screenHeightDp != 0 && screenHeightDp > settings.screenHeightDp) return false;
    }
    if (screenType != 0) {
        if (orientation != 0 && orientation != settings.orientation) return false;
        if (touchscreen != 0 && touchscreen != settings.touchscreen) return false;
    }
    if (input != 0) {
        const int keysHidden = inputFlags & MASK_KEYSHIDDEN;
        const int setKeysHidden = settings.inputFlags & MASK_KEYSHIDDEN;
        if (keysHidden != 0 && keysHidden != setKeysHidden) {
            if (keysHidden != KEYSHIDDEN_NO || setKeysHidden != KEYSHIDDEN_SOFT) return false;
        }
        const int navHidden = inputFlags & MASK_NAVHIDDEN;
        const int setNavHidden = settings.inputFlags & MASK_NAVHIDDEN;
        if (navHidden != 0 && navHidden != setNavHidden) return false;
        if (keyboard != 0 && keyboard != settings.keyboard) return false;
        if (navigation != 0 && navigation != settings.navigation) return false;
    }
    if (screenSize != 0) {
        if (screenWidth != 0 && screenWidth > settings.screenWidth) return false;
        if (screenHeight != 0 && screenHeight > settings.screenHeight) return false;
    }
    if (version != 0) {
        if (sdkVersion != 0 && sdkVersion > settings.sdkVersion) return false;
        if (minorVersion != 0 && minorVersion != settings.minorVersion) return false;
    }
    return true;
}

void ResTable_config::getBcp47Locale(char out[40], bool /*canonicalize*/) const {
    // Minimal BCP-47 rendering: language[-script][-region][-variant]. Sufficient
    // for diagnostics; AOSP's full version (with computed scripts / numbering)
    // is not needed for matching.
    char lang[4] = {0,0,0,0};
    char region[4] = {0,0,0,0};
    unpackLanguage(lang);
    unpackRegion(region);
    char* p = out;
    if (lang[0]) {
        memcpy(p, lang, 2); p += 2;
        if (localeScript[0]) { *p++ = '-'; memcpy(p, localeScript, 4); p += 4; }
        if (region[0]) { *p++ = '-'; memcpy(p, region, 2); p += 2; }
        if (localeVariant[0]) { *p++ = '-'; memcpy(p, localeVariant, 8); p += 8; }
    }
    *p = 0;
}

// Resolve a (possibly absent) string-pool entry by id. AOSP folds this into the
// UnpackOptionalString template over base::expected; here our ResStringPool
// already returns nullptr on a bad id, so we only need the id>=0 gate.
static inline const char16_t* optString16(const ResStringPool& pool, int32_t id,
                                          size_t* outLen) {
    return id >= 0 ? pool.stringAt((size_t)id, outLen) : nullptr;
}
static inline const char* optString8(const ResStringPool& pool, int32_t id,
                                     size_t* outLen) {
    return id >= 0 ? pool.string8At((size_t)id, outLen) : nullptr;
}

// ===========================================================================
// ResXMLParser / ResXMLTree
// ===========================================================================

ResXMLParser::ResXMLParser(const ResXMLTree& tree)
    : mTree(tree), mEventCode(BAD_DOCUMENT), mCurNode(nullptr),
      mCurExt(nullptr), mSourceResourceId(0) {
}

void ResXMLParser::restart() {
    mCurNode = nullptr;
    mEventCode = mTree.mError == NO_ERROR ? START_DOCUMENT : BAD_DOCUMENT;
}

const ResStringPool& ResXMLParser::getStrings() const {
    return mTree.mStrings;
}

ResXMLParser::event_code_t ResXMLParser::getEventType() const {
    return mEventCode;
}

ResXMLParser::event_code_t ResXMLParser::next() {
    if (mEventCode == START_DOCUMENT) {
        mCurNode = mTree.mRootNode;
        mCurExt = mTree.mRootExt;
        return (mEventCode = mTree.mRootCode);
    } else if (mEventCode >= FIRST_CHUNK_CODE) {
        return nextNode();
    }
    return mEventCode;
}

int32_t ResXMLParser::getCommentID() const {
    return mCurNode != nullptr ? (int32_t)dtohl(mCurNode->comment.index) : -1;
}

const char16_t* ResXMLParser::getComment(size_t* outLen) const {
    return optString16(mTree.mStrings, getCommentID(), outLen);
}

uint32_t ResXMLParser::getLineNumber() const {
    return mCurNode != nullptr ? dtohl(mCurNode->lineNumber) : (uint32_t)-1;
}

int32_t ResXMLParser::getTextID() const {
    if (mEventCode == TEXT) {
        return (int32_t)dtohl(((const ResXMLTree_cdataExt*)mCurExt)->data.index);
    }
    return -1;
}

const char16_t* ResXMLParser::getText(size_t* outLen) const {
    return optString16(mTree.mStrings, getTextID(), outLen);
}

ssize_t ResXMLParser::getTextValue(Res_value* outValue) const {
    if (mEventCode == TEXT) {
        outValue->copyFrom_dtoh(((const ResXMLTree_cdataExt*)mCurExt)->typedData);
        return sizeof(Res_value);
    }
    return BAD_TYPE;
}

int32_t ResXMLParser::getNamespacePrefixID() const {
    if (mEventCode == START_NAMESPACE || mEventCode == END_NAMESPACE) {
        return (int32_t)dtohl(((const ResXMLTree_namespaceExt*)mCurExt)->prefix.index);
    }
    return -1;
}

const char16_t* ResXMLParser::getNamespacePrefix(size_t* outLen) const {
    return optString16(mTree.mStrings, getNamespacePrefixID(), outLen);
}

int32_t ResXMLParser::getNamespaceUriID() const {
    if (mEventCode == START_NAMESPACE || mEventCode == END_NAMESPACE) {
        return (int32_t)dtohl(((const ResXMLTree_namespaceExt*)mCurExt)->uri.index);
    }
    return -1;
}

const char16_t* ResXMLParser::getNamespaceUri(size_t* outLen) const {
    return optString16(mTree.mStrings, getNamespaceUriID(), outLen);
}

int32_t ResXMLParser::getElementNamespaceID() const {
    if (mEventCode == START_TAG) {
        return (int32_t)dtohl(((const ResXMLTree_attrExt*)mCurExt)->ns.index);
    }
    if (mEventCode == END_TAG) {
        return (int32_t)dtohl(((const ResXMLTree_endElementExt*)mCurExt)->ns.index);
    }
    return -1;
}

const char16_t* ResXMLParser::getElementNamespace(size_t* outLen) const {
    return optString16(mTree.mStrings, getElementNamespaceID(), outLen);
}

int32_t ResXMLParser::getElementNameID() const {
    if (mEventCode == START_TAG) {
        return (int32_t)dtohl(((const ResXMLTree_attrExt*)mCurExt)->name.index);
    }
    if (mEventCode == END_TAG) {
        return (int32_t)dtohl(((const ResXMLTree_endElementExt*)mCurExt)->name.index);
    }
    return -1;
}

const char16_t* ResXMLParser::getElementName(size_t* outLen) const {
    return optString16(mTree.mStrings, getElementNameID(), outLen);
}

size_t ResXMLParser::getAttributeCount() const {
    if (mEventCode == START_TAG) {
        return dtohs(((const ResXMLTree_attrExt*)mCurExt)->attributeCount);
    }
    return 0;
}

// Locate attribute[idx] relative to the START_TAG extension. Returns nullptr if
// the index is out of range or the current event is not START_TAG.
static inline const ResXMLTree_attribute* attrAt(const ResXMLTree_attrExt* tag,
                                                 size_t idx) {
    return (const ResXMLTree_attribute*)
        (((const uint8_t*)tag)
         + dtohs(tag->attributeStart)
         + (dtohs(tag->attributeSize) * idx));
}

int32_t ResXMLParser::getAttributeNamespaceID(size_t idx) const {
    if (mEventCode == START_TAG) {
        const ResXMLTree_attrExt* tag = (const ResXMLTree_attrExt*)mCurExt;
        if (idx < dtohs(tag->attributeCount)) {
            return (int32_t)dtohl(attrAt(tag, idx)->ns.index);
        }
    }
    return -2;
}

const char16_t* ResXMLParser::getAttributeNamespace(size_t idx, size_t* outLen) const {
    return optString16(mTree.mStrings, getAttributeNamespaceID(idx), outLen);
}

const char* ResXMLParser::getAttributeNamespace8(size_t idx, size_t* outLen) const {
    return optString8(mTree.mStrings, getAttributeNamespaceID(idx), outLen);
}

int32_t ResXMLParser::getAttributeNameID(size_t idx) const {
    if (mEventCode == START_TAG) {
        const ResXMLTree_attrExt* tag = (const ResXMLTree_attrExt*)mCurExt;
        if (idx < dtohs(tag->attributeCount)) {
            return (int32_t)dtohl(attrAt(tag, idx)->name.index);
        }
    }
    return -1;
}

const char16_t* ResXMLParser::getAttributeName(size_t idx, size_t* outLen) const {
    return optString16(mTree.mStrings, getAttributeNameID(idx), outLen);
}

const char* ResXMLParser::getAttributeName8(size_t idx, size_t* outLen) const {
    return optString8(mTree.mStrings, getAttributeNameID(idx), outLen);
}

uint32_t ResXMLParser::getAttributeNameResID(size_t idx) const {
    int32_t id = getAttributeNameID(idx);
    if (id >= 0 && (size_t)id < mTree.mNumResIds) {
        return dtohl(mTree.mResIds[id]);
    }
    return 0;
}

int32_t ResXMLParser::getAttributeValueStringID(size_t idx) const {
    if (mEventCode == START_TAG) {
        const ResXMLTree_attrExt* tag = (const ResXMLTree_attrExt*)mCurExt;
        if (idx < dtohs(tag->attributeCount)) {
            return (int32_t)dtohl(attrAt(tag, idx)->rawValue.index);
        }
    }
    return -1;
}

const char16_t* ResXMLParser::getAttributeStringValue(size_t idx, size_t* outLen) const {
    return optString16(mTree.mStrings, getAttributeValueStringID(idx), outLen);
}

int32_t ResXMLParser::getAttributeDataType(size_t idx) const {
    if (mEventCode == START_TAG) {
        const ResXMLTree_attrExt* tag = (const ResXMLTree_attrExt*)mCurExt;
        if (idx < dtohs(tag->attributeCount)) {
            uint8_t type = attrAt(tag, idx)->typedValue.dataType;
            // Dynamic references are adjusted to plain references at this level.
            if (type != Res_value::TYPE_DYNAMIC_REFERENCE) {
                return type;
            }
            return Res_value::TYPE_REFERENCE;
        }
    }
    return Res_value::TYPE_NULL;
}

int32_t ResXMLParser::getAttributeData(size_t idx) const {
    if (mEventCode == START_TAG) {
        const ResXMLTree_attrExt* tag = (const ResXMLTree_attrExt*)mCurExt;
        if (idx < dtohs(tag->attributeCount)) {
            return (int32_t)dtohl(attrAt(tag, idx)->typedValue.data);
        }
    }
    return 0;
}

ssize_t ResXMLParser::getAttributeValue(size_t idx, Res_value* outValue) const {
    if (mEventCode == START_TAG) {
        const ResXMLTree_attrExt* tag = (const ResXMLTree_attrExt*)mCurExt;
        if (idx < dtohs(tag->attributeCount)) {
            outValue->copyFrom_dtoh(attrAt(tag, idx)->typedValue);
            return sizeof(Res_value);
        }
    }
    return BAD_TYPE;
}

ssize_t ResXMLParser::indexOfAttribute(const char16_t* ns, size_t nsLen,
                                       const char16_t* attr, size_t attrLen) const {
    if (mEventCode == START_TAG) {
        if (attr == nullptr) {
            return NAME_NOT_FOUND;
        }
        const size_t N = getAttributeCount();
        // Our string pool always surfaces char16_t (UTF-8 pools transcode on
        // read), so compare in UTF-16 regardless of the pool's encoding.
        for (size_t i = 0; i < N; i++) {
            size_t curNsLen = 0, curAttrLen = 0;
            const char16_t* curNs = getAttributeNamespace(i, &curNsLen);
            const char16_t* curAttr = getAttributeName(i, &curAttrLen);
            if (curAttr != nullptr && curNsLen == nsLen && curAttrLen == attrLen
                    && memcmp(attr, curAttr, attrLen * sizeof(char16_t)) == 0) {
                if (ns == nullptr) {
                    if (curNs == nullptr) return (ssize_t)i;
                } else if (curNs != nullptr) {
                    if (memcmp(ns, curNs, nsLen * sizeof(char16_t)) == 0) {
                        return (ssize_t)i;
                    }
                }
            }
        }
    }
    return NAME_NOT_FOUND;
}

ssize_t ResXMLParser::indexOfAttribute(const char* ns, const char* attr) const {
    if (attr == nullptr) {
        return NAME_NOT_FOUND;
    }
    // Widen the UTF-8 ns/attr (ASCII XML names) to char16_t and delegate to the
    // char16_t overload. AOSP uses String8/String16 for the same conversion.
    std::u16string attr16;
    for (const char* p = attr; *p; ++p) attr16.push_back((char16_t)(unsigned char)*p);
    if (ns == nullptr) {
        return indexOfAttribute(nullptr, 0, attr16.data(), attr16.size());
    }
    std::u16string ns16;
    for (const char* p = ns; *p; ++p) ns16.push_back((char16_t)(unsigned char)*p);
    return indexOfAttribute(ns16.data(), ns16.size(), attr16.data(), attr16.size());
}

ssize_t ResXMLParser::indexOfID() const {
    if (mEventCode == START_TAG) {
        const ssize_t idx = dtohs(((const ResXMLTree_attrExt*)mCurExt)->idIndex);
        if (idx > 0) return (idx - 1);
    }
    return NAME_NOT_FOUND;
}

ssize_t ResXMLParser::indexOfClass() const {
    if (mEventCode == START_TAG) {
        const ssize_t idx = dtohs(((const ResXMLTree_attrExt*)mCurExt)->classIndex);
        if (idx > 0) return (idx - 1);
    }
    return NAME_NOT_FOUND;
}

ssize_t ResXMLParser::indexOfStyle() const {
    if (mEventCode == START_TAG) {
        const ssize_t idx = dtohs(((const ResXMLTree_attrExt*)mCurExt)->styleIndex);
        if (idx > 0) return (idx - 1);
    }
    return NAME_NOT_FOUND;
}

ResXMLParser::event_code_t ResXMLParser::nextNode() {
    if (mEventCode < 0) {
        return mEventCode;
    }

    do {
        const ResXMLTree_node* nextNode = (const ResXMLTree_node*)
            (((const uint8_t*)mCurNode) + dtohl(mCurNode->header.size));

        if (((const uint8_t*)nextNode) >= mTree.mDataEnd) {
            mCurNode = nullptr;
            return (mEventCode = END_DOCUMENT);
        }

        if (mTree.validateNode(nextNode) != NO_ERROR) {
            mCurNode = nullptr;
            return (mEventCode = BAD_DOCUMENT);
        }

        mCurNode = nextNode;
        const uint16_t headerSize = dtohs(nextNode->header.headerSize);
        const uint32_t totalSize = dtohl(nextNode->header.size);
        mCurExt = ((const uint8_t*)nextNode) + headerSize;
        size_t minExtSize = 0;
        event_code_t eventCode = (event_code_t)dtohs(nextNode->header.type);
        switch ((mEventCode = eventCode)) {
            case RES_XML_START_NAMESPACE_TYPE:
            case RES_XML_END_NAMESPACE_TYPE:
                minExtSize = sizeof(ResXMLTree_namespaceExt);
                break;
            case RES_XML_START_ELEMENT_TYPE:
                minExtSize = sizeof(ResXMLTree_attrExt);
                break;
            case RES_XML_END_ELEMENT_TYPE:
                minExtSize = sizeof(ResXMLTree_endElementExt);
                break;
            case RES_XML_CDATA_TYPE:
                minExtSize = sizeof(ResXMLTree_cdataExt);
                break;
            default:
                LOGW("Unknown XML block: header type %d in node at %d",
                     (int)dtohs(nextNode->header.type),
                     (int)(((const uint8_t*)nextNode) - ((const uint8_t*)mTree.mHeader)));
                continue;
        }

        if ((totalSize - headerSize) < minExtSize) {
            LOGW("Bad XML block: header type 0x%x in node at 0x%x has size %d, need %d",
                 (int)dtohs(nextNode->header.type),
                 (int)(((const uint8_t*)nextNode) - ((const uint8_t*)mTree.mHeader)),
                 (int)(totalSize - headerSize), (int)minExtSize);
            return (mEventCode = BAD_DOCUMENT);
        }

        return eventCode;
    } while (true);
}

void ResXMLParser::getPosition(ResXMLPosition* pos) const {
    pos->eventCode = mEventCode;
    pos->curNode = mCurNode;
    pos->curExt = mCurExt;
}

void ResXMLParser::setPosition(const ResXMLPosition& pos) {
    mEventCode = pos.eventCode;
    mCurNode = pos.curNode;
    mCurExt = pos.curExt;
}

void ResXMLParser::setSourceResourceId(const uint32_t resId) {
    mSourceResourceId = resId;
}

uint32_t ResXMLParser::getSourceResourceId() const {
    return mSourceResourceId;
}

// ---------------------------------------------------------------------------

ResXMLTree::ResXMLTree()
    : ResXMLParser(*this), mError(NO_INIT), mOwnedData(nullptr), mHeader(nullptr),
      mSize(0), mDataEnd(nullptr), mResIds(nullptr), mNumResIds(0),
      mRootNode(nullptr), mRootExt(nullptr), mRootCode(BAD_DOCUMENT) {
    restart();
}

ResXMLTree::~ResXMLTree() {
    uninit();
}

status_t ResXMLTree::setTo(const void* data, size_t size, bool copyData) {
    uninit();
    mEventCode = START_DOCUMENT;

    if (!data || !size) {
        return (mError = BAD_TYPE);
    }

    if (copyData) {
        mOwnedData = malloc(size);
        if (mOwnedData == nullptr) {
            return (mError = NO_MEMORY);
        }
        memcpy(mOwnedData, data, size);
        data = mOwnedData;
    }

    mHeader = (const ResXMLTree_header*)data;
    mSize = dtohl(mHeader->header.size);
    if (dtohs(mHeader->header.headerSize) > mSize || mSize > size) {
        LOGW("Bad XML block: header size %d or total size %d is larger than data size %zu",
             (int)dtohs(mHeader->header.headerSize), (int)mSize, size);
        mError = BAD_TYPE;
        restart();
        return mError;
    }
    mDataEnd = ((const uint8_t*)mHeader) + mSize;

    mStrings.uninit();
    mRootNode = nullptr;
    mResIds = nullptr;
    mNumResIds = 0;

    // Scan the top-level chunks for the string pool, the resource-id map, and
    // the first XML node (which becomes the root).
    const ResChunk_header* chunk =
        (const ResChunk_header*)(((const uint8_t*)mHeader) + dtohs(mHeader->header.headerSize));
    const ResChunk_header* lastChunk = chunk;
    while (((const uint8_t*)chunk) < (mDataEnd - sizeof(ResChunk_header)) &&
           ((const uint8_t*)chunk) < (mDataEnd - dtohl(chunk->size))) {
        status_t err = validate_chunk(chunk, sizeof(ResChunk_header), mDataEnd, "XML");
        if (err != NO_ERROR) {
            mError = err;
            goto done;
        }
        const uint16_t type = dtohs(chunk->type);
        const size_t chunkSize = dtohl(chunk->size);
        if (type == RES_STRING_POOL_TYPE) {
            mStrings.setTo(chunk, chunkSize);
        } else if (type == RES_XML_RESOURCE_MAP_TYPE) {
            mResIds = (const uint32_t*)
                (((const uint8_t*)chunk) + dtohs(chunk->headerSize));
            mNumResIds = (dtohl(chunk->size) - dtohs(chunk->headerSize)) / sizeof(uint32_t);
        } else if (type >= RES_XML_FIRST_CHUNK_TYPE && type <= RES_XML_LAST_CHUNK_TYPE) {
            if (validateNode((const ResXMLTree_node*)chunk) != NO_ERROR) {
                mError = BAD_TYPE;
                goto done;
            }
            mCurNode = (const ResXMLTree_node*)lastChunk;
            if (nextNode() == BAD_DOCUMENT) {
                mError = BAD_TYPE;
                goto done;
            }
            mRootNode = mCurNode;
            mRootExt = mCurExt;
            mRootCode = mEventCode;
            break;
        }
        lastChunk = chunk;
        chunk = (const ResChunk_header*)(((const uint8_t*)chunk) + chunkSize);
    }

    if (mRootNode == nullptr) {
        LOGW("Bad XML block: no root element node found");
        mError = BAD_TYPE;
        goto done;
    }

    mError = mStrings.getError();

done:
    restart();
    return mError;
}

status_t ResXMLTree::getError() const {
    return mError;
}

void ResXMLTree::uninit() {
    mError = NO_INIT;
    mStrings.uninit();
    if (mOwnedData) {
        free(mOwnedData);
        mOwnedData = nullptr;
    }
    restart();
}

status_t ResXMLTree::validateNode(const ResXMLTree_node* node) const {
    const uint16_t eventCode = dtohs(node->header.type);

    status_t err = validate_chunk(&node->header, sizeof(ResXMLTree_node), mDataEnd,
                                  "ResXMLTree_node");

    if (err >= NO_ERROR) {
        // Only perform additional validation on START nodes.
        if (eventCode != RES_XML_START_ELEMENT_TYPE) {
            return NO_ERROR;
        }

        const uint16_t headerSize = dtohs(node->header.headerSize);
        const uint32_t size = dtohl(node->header.size);
        const ResXMLTree_attrExt* attrExt =
            (const ResXMLTree_attrExt*)(((const uint8_t*)node) + headerSize);
        if ((size >= headerSize + sizeof(ResXMLTree_attrExt))
                && ((void*)attrExt > (void*)node)) {
            const size_t attrSize = ((size_t)dtohs(attrExt->attributeSize))
                * dtohs(attrExt->attributeCount);
            if ((dtohs(attrExt->attributeStart) + attrSize) <= (size - headerSize)) {
                return NO_ERROR;
            }
            LOGW("Bad XML block: node attributes use 0x%x bytes, only have 0x%x bytes",
                 (unsigned int)(dtohs(attrExt->attributeStart) + attrSize),
                 (unsigned int)(size - headerSize));
        } else {
            LOGW("Bad XML start block: node header size 0x%x, size 0x%x",
                 (unsigned int)headerSize, (unsigned int)size);
        }
        return BAD_TYPE;
    }

    return err;
}

// ===========================================================================
// ResTable — load resources.arsc + resolve resource id to a value (stage 3b).
// Internal representation is our own; the resolution/selection semantics mirror
// AOSP ResTable::getEntry / parsePackage (config match() -> isBetterThan() over
// the type's config variants, then read the ResTable_entry).
// ===========================================================================

ResTable::ResTable()
    : mError(NO_INIT), mCookie(-1), mNextPackageId(2) {
    memset(&mParams, 0, sizeof(mParams));
    mParams.size = sizeof(ResTable_config);
    mPackageMap.assign(256, 0);
}

ResTable::~ResTable() {
    uninit();
}

void ResTable::uninit() {
    mError = NO_INIT;
    for (Header& h : mHeaders) h.values.uninit();
    mHeaders.clear();
    mPackages.clear();
    mPackageMap.assign(256, 0);
    mNextPackageId = 2;
}

status_t ResTable::add(const void* data, size_t size, int32_t cookie, bool copyData) {
    return addInternal(data, size, /*appAsLib*/ false, cookie, copyData);
}

status_t ResTable::add(const void* data, size_t size, bool appAsLib,
                       int32_t cookie, bool copyData) {
    return addInternal(data, size, appAsLib, cookie, copyData);
}

status_t ResTable::getError() const {
    return mError;
}

status_t ResTable::addInternal(const void* data, size_t size, bool appAsLib,
                               int32_t cookie, bool copyData) {
    mCookie = cookie;
    if (!data || !size) return (mError = BAD_TYPE);

    // Each add() is one loaded arsc = one Header (owns its global value pool).
    size_t hdrIdx = mHeaders.size();
    mHeaders.push_back(Header());
    Header* hdr = &mHeaders[hdrIdx];
    hdr->index = (int32_t)hdrIdx;
    hdr->cookie = cookie;
    hdr->size = size;
    if (copyData) {
        hdr->owned.assign((const uint8_t*)data, (const uint8_t*)data + size);
        hdr->data = hdr->owned.data();
    } else {
        hdr->data = (const uint8_t*)data;
    }
    const uint8_t* base = hdr->data;

    const ResTable_header* rth = (const ResTable_header*)base;
    if (validate_chunk(&rth->header, sizeof(ResTable_header), base + size, "ResTable_header") != NO_ERROR) {
        mHeaders.pop_back();
        return (mError = BAD_TYPE);
    }
    hdr->dataEnd = base + dtohl(rth->header.size);
    const uint8_t* dataEnd = hdr->dataEnd;

    // Walk the top-level chunks: a global value string pool + package(s).
    const ResChunk_header* chunk =
        (const ResChunk_header*)(base + dtohs(rth->header.headerSize));
    while (((const uint8_t*)chunk) <= (dataEnd - sizeof(ResChunk_header)) &&
           ((const uint8_t*)chunk) <= (dataEnd - dtohl(chunk->size))) {
        if (validate_chunk(chunk, sizeof(ResChunk_header), dataEnd, "ResTable") != NO_ERROR) {
            return (mError = BAD_TYPE);
        }
        const uint16_t type = dtohs(chunk->type);
        const uint32_t csize = dtohl(chunk->size);
        if (type == RES_STRING_POOL_TYPE) {
            if (hdr->values.setTo(chunk, csize) != NO_ERROR) return (mError = BAD_TYPE);
        } else if (type == RES_TABLE_PACKAGE_TYPE) {
            uint8_t runtimeId = 0;
            status_t err = parsePackage((const ResTable_package*)chunk, hdr, appAsLib, &runtimeId);
            if (err != NO_ERROR) return (mError = err);
        }
        chunk = (const ResChunk_header*)(((const uint8_t*)chunk) + csize);
    }

    mError = mHeaders.empty() ? NO_INIT : mHeaders[0].values.getError();
    return mError;
}

status_t ResTable::parsePackage(const ResTable_package* pkg, Header* header,
                                bool appAsLib, uint8_t* outRuntimeId) {
    const uint8_t* base = (const uint8_t*)pkg;
    const uint8_t* dataEnd = header->dataEnd;
    if (validate_chunk(&pkg->header, sizeof(ResTable_package) - sizeof(pkg->typeIdOffset),
                       dataEnd, "ResTable_package") != NO_ERROR) {
        return BAD_TYPE;
    }
    const uint32_t pkgSize = dtohl(pkg->header.size);
    const uint32_t typeStringsOff = dtohl(pkg->typeStrings);
    const uint32_t keyStringsOff = dtohl(pkg->keyStrings);
    if (typeStringsOff >= pkgSize || (typeStringsOff & 0x3) ||
        keyStringsOff >= pkgSize || (keyStringsOff & 0x3)) {
        return BAD_TYPE;
    }
    const uint32_t rawId = dtohl(pkg->id);
    if (rawId == 0 || rawId >= 256) return BAD_TYPE;  // dynamic/staged ids unsupported
    const uint8_t buildId = (uint8_t)rawId;

    // Runtime id assignment (AOSP addInternal): an app-id (0x7f) package loaded as
    // a shared library is reassigned a fresh runtime id; its self-references are
    // translated via its DynamicRefTable. Other packages keep their build-time id.
    const bool isDynamic = (buildId == APP_PACKAGE_ID && appAsLib);
    const uint8_t runtimeId = isDynamic ? mNextPackageId++ : buildId;
    if (mNextPackageId >= APP_PACKAGE_ID) mNextPackageId = 2;  // never hand out the app id
    *outRuntimeId = runtimeId;

    Package package;
    package.headerIndex = header->index;  // store index, not pointer (mHeaders reallocs)
    package.id = runtimeId;
    package.buildId = buildId;
    package.isDynamic = isDynamic;
    package.dynamicRefTable = DynamicRefTable(runtimeId, appAsLib);
    // Package name (UTF-16 -> UTF-8; ASCII/BMP names only in practice).
    for (int i = 0; i < 128 && pkg->name[i] != 0; i++) {
        uint32_t c = pkg->name[i];
        if (c < 0x80) package.name += (char)c;
        else if (c < 0x800) { package.name += (char)(0xC0|(c>>6)); package.name += (char)(0x80|(c&0x3F)); }
        else { package.name += (char)(0xE0|(c>>12)); package.name += (char)(0x80|((c>>6)&0x3F)); package.name += (char)(0x80|(c&0x3F)); }
    }
    if (package.typeStrings.setTo(base + typeStringsOff, dataEnd - (base + typeStringsOff)) != NO_ERROR) {
        return BAD_TYPE;
    }
    if (package.keyStrings.setTo(base + keyStringsOff, dataEnd - (base + keyStringsOff)) != NO_ERROR) {
        return BAD_TYPE;
    }

    // Iterate the package's chunks: TypeSpec (creates a TypeGroup) + Type
    // (appends a config variant to the most recent TypeGroup for that id).
    const ResChunk_header* chunk =
        (const ResChunk_header*)(base + dtohs(pkg->header.headerSize));
    const uint8_t* endPos = base + pkgSize;
    while (((const uint8_t*)chunk) <= (endPos - sizeof(ResChunk_header)) &&
           ((const uint8_t*)chunk) <= (endPos - dtohl(chunk->size))) {
        if (validate_chunk(chunk, sizeof(ResChunk_header), endPos, "ResTable_package chunk") != NO_ERROR) {
            return BAD_TYPE;
        }
        const uint16_t ctype = dtohs(chunk->type);
        const uint32_t csize = dtohl(chunk->size);

        if (ctype == RES_TABLE_TYPE_SPEC_TYPE) {
            const ResTable_typeSpec* ts = (const ResTable_typeSpec*)chunk;
            if (validate_chunk(&ts->header, sizeof(ResTable_typeSpec), endPos, "ResTable_typeSpec") != NO_ERROR) {
                return BAD_TYPE;
            }
            const uint32_t entryCount = dtohl(ts->entryCount);
            if (dtohs(ts->header.headerSize) + sizeof(uint32_t) * entryCount > csize) {
                return BAD_TYPE;
            }
            if (entryCount > 0) {
                uint8_t typeIndex = ts->id - 1;
                if (typeIndex >= package.types.size()) package.types.resize(typeIndex + 1);
                package.types[typeIndex].entryCount = entryCount;
                package.types[typeIndex].specFlags = (const uint32_t*)(
                    ((const uint8_t*)ts) + dtohs(ts->header.headerSize));
            }
        } else if (ctype == RES_TABLE_TYPE_TYPE) {
            const ResTable_type* tp = (const ResTable_type*)chunk;
            if (validate_chunk(&tp->header, sizeof(ResTable_type) - sizeof(ResTable_config) + 4,
                               endPos, "ResTable_type") != NO_ERROR) {
                return BAD_TYPE;
            }
            const uint32_t entryCount = dtohl(tp->entryCount);
            if (dtohs(tp->header.headerSize) + sizeof(uint32_t) * entryCount > csize) {
                return BAD_TYPE;
            }
            if (entryCount > 0) {
                uint8_t typeIndex = tp->id - 1;
                if (typeIndex >= package.types.size()) {
                    return BAD_TYPE;  // No TypeSpec seen first
                }
                package.types[typeIndex].configs.push_back(tp);
            }
        }
        chunk = (const ResChunk_header*)(((const uint8_t*)chunk) + csize);
    }

    // Register the package under its RUNTIME id; merge if an entry already exists.
    if (mPackageMap[runtimeId] == 0) {
        mPackageMap[runtimeId] = (int)mPackages.size() + 1;
        mPackages.push_back(std::move(package));
    } else {
        Package& dst = mPackages[mPackageMap[runtimeId] - 1];
        for (size_t t = 0; t < package.types.size(); t++) {
            if (t >= dst.types.size()) dst.types.resize(t + 1);
            if (package.types[t].entryCount) dst.types[t].entryCount = package.types[t].entryCount;
            if (package.types[t].specFlags) dst.types[t].specFlags = package.types[t].specFlags;
            for (const ResTable_type* c : package.types[t].configs) dst.types[t].configs.push_back(c);
        }
    }
    return NO_ERROR;
}

const ResStringPool& ResTable::getStringPool() const {
    static const ResStringPool kEmpty;
    return mHeaders.empty() ? kEmpty : mHeaders[0].values;
}

status_t ResTable::setParameters(const ResTable_config* params) {
    if (params) {
        mParams = *params;
        mParams.size = sizeof(ResTable_config);
    } else {
        memset(&mParams, 0, sizeof(mParams));
        mParams.size = sizeof(ResTable_config);
    }
    return NO_ERROR;
}

void ResTable::getParameters(ResTable_config* params) const {
    if (params) *params = mParams;
}

const ResTable::Package* ResTable::packageForId(uint32_t pkgId) const {
    int idx = (pkgId < mPackageMap.size()) ? mPackageMap[pkgId] : 0;
    return idx ? &mPackages[idx - 1] : nullptr;
}
ResTable::Package* ResTable::packageForId(uint32_t pkgId) {
    int idx = (pkgId < mPackageMap.size()) ? mPackageMap[pkgId] : 0;
    return idx ? &mPackages[idx - 1] : nullptr;
}

// Core selection (mirrors AOSP ResTable::getEntry). Iterate the config variants
// of typeId, drop those that don't match() `desired` or whose entry is absent
// (NO_ENTRY), keep the best (isBetterThan vs `desired`). Aggregate the TypeSpec
// flags for the entry into *outSpecFlags. Returns the entry pointer or nullptr.
const ResTable_entry* ResTable::getBestEntry(const Package& pkg, uint8_t typeId,
                                             uint32_t entryId,
                                             const ResTable_config& desired,
                                             ResTable_config* outConfig,
                                             uint32_t* outSpecFlags) const {
    if (typeId == 0 || (size_t)(typeId - 1) >= pkg.types.size()) return nullptr;
    const TypeGroup& group = pkg.types[typeId - 1];
    if (group.configs.empty()) return nullptr;
    if (entryId >= group.entryCount) return nullptr;

    if (outSpecFlags && group.specFlags) *outSpecFlags |= dtohl(group.specFlags[entryId]);

    const ResTable_type* bestType = nullptr;
    uint32_t bestOffset = ResTable_type::NO_ENTRY;
    ResTable_config bestConfig;
    memset(&bestConfig, 0, sizeof(bestConfig));

    for (const ResTable_type* tp : group.configs) {
        ResTable_config thisConfig;
        thisConfig.copyFromDtoH(tp->config);
        if (!thisConfig.match(desired)) continue;

        const uint32_t* eindex = (const uint32_t*)(((const uint8_t*)tp) + dtohs(tp->header.headerSize));
        uint32_t thisOffset;
        if (tp->flags & ResTable_type::FLAG_SPARSE) {
            const ResTable_sparseTypeEntry* sparse =
                (const ResTable_sparseTypeEntry*)eindex;
            const ResTable_sparseTypeEntry* end = sparse + dtohl(tp->entryCount);
            const ResTable_sparseTypeEntry* lo = std::lower_bound(
                sparse, end, (uint16_t)entryId,
                [](const ResTable_sparseTypeEntry& a, uint16_t b) { return dtohs(a.idx) < b; });
            if (lo == end || dtohs(lo->idx) != entryId) continue;
            thisOffset = dtohs(lo->offset) * 4u;
        } else {
            if (entryId >= dtohl(tp->entryCount)) continue;
            thisOffset = dtohl(eindex[entryId]);
        }
        if (thisOffset == ResTable_type::NO_ENTRY) continue;

        if (bestType != nullptr && !thisConfig.isBetterThan(bestConfig, &desired)) continue;

        bestType = tp;
        bestOffset = thisOffset;
        bestConfig = thisConfig;
    }

    if (bestType == nullptr) return nullptr;

    bestOffset += dtohl(bestType->entriesStart);
    if (bestOffset > dtohl(bestType->header.size) - sizeof(ResTable_entry)) return nullptr;
    if ((bestOffset & 0x3) != 0) return nullptr;

    const ResTable_entry* entry = (const ResTable_entry*)(((const uint8_t*)bestType) + bestOffset);
    if (dtohs(entry->size) < sizeof(ResTable_entry)) return nullptr;
    if (outConfig) *outConfig = bestConfig;
    return entry;
}

// Faithful to AOSP ResTable::getResource. `density`>0 overrides mParams.density;
// a complex (bag) entry returns BAD_VALUE (mayBeBag only silences the log);
// outSpecFlags gets the aggregated TypeSpec flags; outConfig the matched variant.
// Returns the owning asset blockIndex (>= 0, always 0 in the single-asset model)
// or a negative error.
ssize_t ResTable::getResource(uint32_t resID, Res_value* outValue, bool mayBeBag,
                              uint16_t density, uint32_t* outSpecFlags,
                              ResTable_config* outConfig) const {
    if (mError != NO_ERROR) return mError;
    uint8_t pkgId  = (uint8_t)((resID >> 24) & 0xff);
    uint8_t typeId = (uint8_t)((resID >> 16) & 0xff);
    uint32_t entryId = resID & 0xffff;
    const Package* pkg = packageForId(pkgId);
    if (!pkg) return BAD_INDEX;
    if (typeId == 0) return BAD_INDEX;

    ResTable_config desired = mParams;
    if (density > 0) desired.density = density;

    uint32_t specFlags = 0;
    const ResTable_entry* entry = getBestEntry(*pkg, typeId, entryId, desired, outConfig,
                                               outSpecFlags ? &specFlags : nullptr);
    if (!entry) return NAME_NOT_FOUND;

    if (dtohs(entry->flags) & ResTable_entry::FLAG_COMPLEX) {
        if (!mayBeBag) {
            LOGW("Requesting resource 0x%08x failed because it is complex", resID);
        }
        return BAD_VALUE;
    }

    const Res_value* inValue = (const Res_value*)(((const uint8_t*)entry) + dtohs(entry->size));
    if (outValue) {
        outValue->copyFrom_dtoh(*inValue);
        // Translate build-time package ids in references to runtime ids (shared
        // library / add-on package case). For fixed-id packages this is identity.
        if (pkg->dynamicRefTable.lookupResourceValue(outValue) != NO_ERROR) {
            LOGW("Failed to resolve referenced package: 0x%08x", outValue->data);
            return BAD_VALUE;
        }
    }
    if (outSpecFlags) *outSpecFlags = specFlags;
    return (ssize_t)pkg->headerIndex;  // owning-asset blockIndex
}

// Faithful to AOSP ResTable::resolveReference. Follows TYPE_REFERENCE chains
// (count < 20), passing mayBeBag=true so a reference into a style/bag leaves the
// last good value. Returns the final blockIndex (>= 0) or BAD_INDEX.
ssize_t ResTable::resolveReference(Res_value* value, ssize_t blockIndex,
                                   uint32_t* outLastRef, uint32_t* inoutTypeSpecFlags,
                                   ResTable_config* outConfig) const {
    int count = 0;
    while (blockIndex >= 0 && value->dataType == Res_value::TYPE_REFERENCE
            && value->data != 0 && count < 20) {
        if (outLastRef) *outLastRef = value->data;
        uint32_t newFlags = 0;
        const ssize_t newIndex = getResource(value->data, value, true, 0, &newFlags, outConfig);
        if (newIndex == BAD_INDEX) {
            return BAD_INDEX;
        }
        if (inoutTypeSpecFlags) *inoutTypeSpecFlags |= newFlags;
        if (newIndex < 0) {
            // Referenced resource couldn't be read as a value (e.g. a style):
            // leave *value as the reference and return the last good index.
            return blockIndex;
        }
        blockIndex = newIndex;
        count++;
    }
    return blockIndex;
}

const char16_t* ResTable::getResourceString(uint32_t resId, size_t* outLen) const {
    Res_value v;
    ssize_t block = getResource(resId, &v);
    if (block < 0) return nullptr;
    block = resolveReference(&v, block);
    if (block < 0 || v.dataType != Res_value::TYPE_STRING) return nullptr;
    // String values index into their owning arsc's global pool.
    const ResStringPool& pool = (block < (ssize_t)mHeaders.size()) ? mHeaders[block].values : getStringPool();
    size_t len = 0;
    const char16_t* s = pool.stringAt(v.data, &len);
    if (outLen) *outLen = len;
    return s;
}

const ResTable_map* ResTable::getBag(uint32_t resId, size_t* outCount,
                                     ResTable_config* outConfig,
                                     ssize_t* outBlock,
                                     uint32_t* outSpecFlags) const {
    if (outCount) *outCount = 0;
    if (outBlock) *outBlock = -1;
    if (outSpecFlags) *outSpecFlags = 0;
    if (mError != NO_ERROR) return nullptr;
    uint8_t pkgId  = (uint8_t)((resId >> 24) & 0xff);
    uint8_t typeId = (uint8_t)((resId >> 16) & 0xff);
    uint32_t entryId = resId & 0xffff;
    const Package* pkg = packageForId(pkgId);
    if (!pkg) return nullptr;
    const ResTable_entry* entry = getBestEntry(*pkg, typeId, entryId, mParams, outConfig, outSpecFlags);
    if (!entry || !(entry->flags & ResTable_entry::FLAG_COMPLEX)) return nullptr;
    const ResTable_map_entry* me = (const ResTable_map_entry*)entry;
    if (outCount) *outCount = dtohl(me->count);
    if (outBlock) *outBlock = (ssize_t)pkg->headerIndex;  // owning header (string pool)
    return (const ResTable_map*)(((const uint8_t*)entry) + dtohs(entry->size));
}

bool ResTable::getResourceName(uint32_t resId, std::string* outPackage,
                               std::string* outType, std::string* outKey) const {
    uint8_t pkgId  = (uint8_t)((resId >> 24) & 0xff);
    uint8_t typeId = (uint8_t)((resId >> 16) & 0xff);
    uint32_t entryId = resId & 0xffff;
    const Package* pkg = packageForId(pkgId);
    if (!pkg) return false;
    if (outPackage) *outPackage = pkg->name;
    if (outType && typeId >= 1 && (size_t)(typeId - 1) < pkg->typeStrings.size()) {
        *outType = poolString(pkg->typeStrings, typeId - 1);
    }
    if (outKey) {
        const ResTable_entry* entry = getBestEntry(*pkg, typeId, entryId, mParams, nullptr, nullptr);
        if (entry) *outKey = poolString(pkg->keyStrings, dtohl(entry->key.index));
    }
    return true;
}

// Find a string index in a pool by content (linear; pools are unsorted).
/*static*/ int32_t ResTable::poolIndexOf(const ResStringPool& pool, const std::string& needle) {
    size_t n = pool.size();
    for (uint32_t i = 0; i < n; i++) {
        if (poolString(pool, i) == needle) return (int32_t)i;
    }
    return -1;
}

uint32_t ResTable::getIdentifier(const std::string& name, const std::string& type,
                                 const std::string& package) const {
    if (name.empty() || type.empty()) return 0;
    for (const Package& pkg : mPackages) {
        if (!package.empty() && pkg.name != package) continue;
        // Find the type id by type name.
        int32_t keyType = -1;
        for (uint32_t t = 0; t < pkg.typeStrings.size(); t++) {
            if (poolString(pkg.typeStrings, t) == type) { keyType = (int32_t)t; break; }
        }
        if (keyType < 0 || (size_t)keyType >= pkg.types.size()) continue;
        // Find the entry whose key matches the name.
        int32_t keyIdx = poolIndexOf(pkg.keyStrings, name);
        if (keyIdx < 0) continue;
        const TypeGroup& tg = pkg.types[keyType];
        for (uint32_t e = 0; e < tg.entryCount; e++) {
            const ResTable_entry* entry = anyEntry(pkg, (uint8_t)(keyType + 1), e);
            if (entry && (int32_t)dtohl(entry->key.index) == keyIdx) {
                return ((uint32_t)pkg.id << 24) | ((uint32_t)(keyType + 1) << 16) | e;
            }
        }
    }
    return 0;
}

void ResTable::getConfigurations(std::vector<ResTable_config>* out) const {
    if (!out) return;
    out->clear();
    for (const Package& pkg : mPackages) {
        for (const TypeGroup& tg : pkg.types) {
            for (const ResTable_type* tp : tg.configs) {
                ResTable_config c;
                c.copyFromDtoH(tp->config);
                // Dedupe.
                bool seen = false;
                for (const auto& e : *out) {
                    if (memcmp(&e, &c, sizeof(ResTable_config)) == 0) { seen = true; break; }
                }
                if (!seen) out->push_back(c);
            }
        }
    }
}

std::vector<std::string> ResTable::listPackageNames() const {
    std::vector<std::string> names;
    for (const Package& pkg : mPackages) names.push_back(pkg.name);
    return names;
}

// Decode string-pool entry idx to UTF-8 regardless of the pool's encoding
// (UTF-8 pools return raw bytes; UTF-16 pools transcode ASCII/BMP).
/*static*/ std::string ResTable::poolString(const ResStringPool& pool, uint32_t idx) {
    std::string out;
    if (pool.isUTF8()) {
        size_t len = 0;
        const char* s = pool.string8At(idx, &len);
        if (s) out.assign(s, len);
        return out;
    }
    size_t len = 0;
    const char16_t* s = pool.stringAt(idx, &len);
    for (size_t i = 0; s && i < len; ++i) {
        uint32_t c = s[i];
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < len && s[i + 1] >= 0xDC00 && s[i + 1] <= 0xDFFF)
            c = 0x10000 + ((c - 0xD800) << 10) + (s[++i] - 0xDC00);
        if (c < 0x80) out += (char)c;
        else if (c < 0x800) { out += (char)(0xC0 | (c >> 6)); out += (char)(0x80 | (c & 0x3F)); }
        else if (c < 0x10000) { out += (char)(0xE0 | (c >> 12)); out += (char)(0x80 | ((c >> 6) & 0x3F)); out += (char)(0x80 | (c & 0x3F)); }
        else { out += (char)(0xF0 | (c >> 18)); out += (char)(0x80 | ((c >> 12) & 0x3F)); out += (char)(0x80 | ((c >> 6) & 0x3F)); out += (char)(0x80 | (c & 0x3F)); }
    }
    return out;
}

// First present entry for (typeId, entryId) across any config (no config
// preference) — used to enumerate resources without resolving a value.
const ResTable_entry* ResTable::anyEntry(const Package& pkg, uint8_t typeId,
                                         uint32_t entryId) const {
    if (typeId == 0 || (size_t)(typeId - 1) >= pkg.types.size()) return nullptr;
    const TypeGroup& group = pkg.types[typeId - 1];
    if (entryId >= group.entryCount) return nullptr;
    for (const ResTable_type* tp : group.configs) {
        const uint32_t* eindex = (const uint32_t*)(((const uint8_t*)tp) + dtohs(tp->header.headerSize));
        uint32_t off;
        if (tp->flags & ResTable_type::FLAG_SPARSE) {
            const ResTable_sparseTypeEntry* sparse = (const ResTable_sparseTypeEntry*)eindex;
            const ResTable_sparseTypeEntry* end = sparse + dtohl(tp->entryCount);
            const ResTable_sparseTypeEntry* lo = std::lower_bound(
                sparse, end, (uint16_t)entryId,
                [](const ResTable_sparseTypeEntry& a, uint16_t b) { return dtohs(a.idx) < b; });
            if (lo == end || dtohs(lo->idx) != entryId) continue;
            off = dtohs(lo->offset) * 4u;
        } else {
            if (entryId >= dtohl(tp->entryCount)) continue;
            off = dtohl(eindex[entryId]);
        }
        if (off == ResTable_type::NO_ENTRY) continue;
        off += dtohl(tp->entriesStart);
        if (off > dtohl(tp->header.size) - sizeof(ResTable_entry)) continue;
        const ResTable_entry* e = (const ResTable_entry*)(((const uint8_t*)tp) + off);
        if (dtohs(e->size) >= sizeof(ResTable_entry)) return e;
    }
    return nullptr;
}

size_t ResTable::listAllResources(std::vector<ResourceRef>* out) const {
    if (out) out->clear();
    size_t count = 0;
    for (const Package& pkg : mPackages) {
        for (size_t t = 0; t < pkg.types.size(); t++) {
            uint8_t typeId = (uint8_t)(t + 1);
            const TypeGroup& group = pkg.types[t];
            if (group.entryCount == 0) continue;
            std::string typeName = poolString(pkg.typeStrings, typeId - 1);
            for (uint32_t entryId = 0; entryId < group.entryCount; entryId++) {
                const ResTable_entry* e = anyEntry(pkg, typeId, entryId);
                if (!e) continue;  // entry not present in any config
                ResourceRef r;
                r.resId = ((uint32_t)pkg.id << 24) | ((uint32_t)typeId << 16) | entryId;
                r.packageId = pkg.id;
                r.type = typeName;
                r.key = poolString(pkg.keyStrings, dtohl(e->key.index));
                if (out) out->push_back(std::move(r));
                count++;
            }
        }
    }
    return count;
}

uint32_t ResTable::getBagParent(uint32_t resId, ResTable_config* outConfig) const {
    if (mError != NO_ERROR) return 0;
    uint8_t pkgId  = (uint8_t)((resId >> 24) & 0xff);
    uint8_t typeId = (uint8_t)((resId >> 16) & 0xff);
    uint32_t entryId = resId & 0xffff;
    const Package* pkg = packageForId(pkgId);
    if (!pkg) return 0;
    const ResTable_entry* e = getBestEntry(*pkg, typeId, entryId, mParams, outConfig, nullptr);
    if (!e || !(dtohs(e->flags) & ResTable_entry::FLAG_COMPLEX)) return 0;
    return dtohl(((const ResTable_map_entry*)e)->parent.ident);
}

// ===========================================================================
// ResTable::Theme — applied-style set with parent inheritance + force/NULL
// override semantics (port of AOSP ResTable::Theme, minus the per-type bag_set
// locking cache; a plain map is semantically equivalent).
// ===========================================================================

ResTable::Theme::Theme(const ResTable& table) : mTable(table) {}
ResTable::Theme::~Theme() {}

status_t ResTable::Theme::clear() {
    mEntries.clear();
    mTypeSpecFlags = 0;
    return NO_ERROR;
}

status_t ResTable::Theme::setTo(const Theme& other) {
    if (this == &other) return NO_ERROR;
    mEntries = other.mEntries;
    mTypeSpecFlags = other.mTypeSpecFlags;
    return NO_ERROR;
}

uint32_t ResTable::Theme::getChangingConfigurations() const {
    return mTypeSpecFlags;
}

status_t ResTable::Theme::applyStyle(uint32_t resID, bool force) {
    return applyStyleChain(resID, force, 0);
}

// Parent inheritance: apply THIS (most-derived) style's items first, then walk
// up the parent chain. Combined with the "overwrite only if slot is null" rule
// below, a child's already-set value is sticky and the parent only fills gaps.
status_t ResTable::Theme::applyStyleChain(uint32_t resID, bool force, int depth) {
    if (depth > 16) return BAD_VALUE;

    size_t count = 0;
    ResTable_config cfg;
    ssize_t block = -1;
    uint32_t specFlags = 0;
    const ResTable_map* map = mTable.getBag(resID, &count, &cfg, &block, &specFlags);
    if (!map) return NAME_NOT_FOUND;  // not a (complex) style
    mTypeSpecFlags |= specFlags;  // aggregate config axes (for getChangingConfigurations)

    for (size_t i = 0; i < count; i++) {
        const uint32_t attrRes = dtohl(map[i].name.ident);
        Res_value v;
        v.copyFrom_dtoh(map[i].value);
        ThemedItem& it = mEntries[attrRes];
        // AOSP override rule: force wins; otherwise an unset slot, or a slot
        // holding TYPE_NULL undefined, is overwritten (sticky otherwise).
        const bool overwrite = force || !it.set ||
            (it.value.dataType == Res_value::TYPE_NULL &&
             it.value.data == Res_value::DATA_NULL_UNDEFINED);
        if (overwrite) {
            it.value = v;
            it.stringBlock = block;
            it.typeSpecFlags = mTypeSpecFlags;
            it.set = true;
        }
    }

    // Now the parent fills any attributes this style didn't set.
    uint32_t parent = mTable.getBagParent(resID);
    if (parent != 0 && parent != resID) {
        status_t err = applyStyleChain(parent, force, depth + 1);
        if (err != NO_ERROR && err != NAME_NOT_FOUND) return err;
    }
    return NO_ERROR;
}

ssize_t ResTable::Theme::getAttribute(uint32_t resID, Res_value* outValue,
                                      uint32_t* outTypeSpecFlags) const {
    auto it = mEntries.find(resID);
    if (it == mEntries.end() || !it->second.set) return NAME_NOT_FOUND;
    if (outValue) *outValue = it->second.value;
    if (outTypeSpecFlags) *outTypeSpecFlags = it->second.typeSpecFlags | mTypeSpecFlags;
    return it->second.stringBlock;  // owning header index (for string values)
}

// Like ResTable::resolveReference, but TYPE_ATTRIBUTE resolves against this
// theme (getAttribute) rather than the table. Returns the final blockIndex or a
// negative error; bounded to < 20 iterations.
ssize_t ResTable::Theme::resolveAttributeReference(Res_value* inOutValue, ssize_t blockIndex,
                                                   uint32_t* outLastRef,
                                                   uint32_t* inoutTypeSpecFlags,
                                                   ResTable_config* inoutConfig) const {
    int count = 0;
    while (blockIndex >= 0 && count < 20) {
        if (inOutValue->dataType == Res_value::TYPE_ATTRIBUTE) {
            uint32_t newFlags = 0;
            ssize_t newBlock = getAttribute(inOutValue->data, inOutValue, &newFlags);
            if (inoutTypeSpecFlags) *inoutTypeSpecFlags |= newFlags;
            if (newBlock < 0) return newBlock;
            blockIndex = newBlock;
        } else if (inOutValue->dataType == Res_value::TYPE_REFERENCE) {
            if (outLastRef) *outLastRef = inOutValue->data;
            if (inOutValue->data == 0) break;
            uint32_t newFlags = 0;
            ssize_t newBlock = mTable.getResource(inOutValue->data, inOutValue, true, 0,
                                                  &newFlags, inoutConfig);
            if (newBlock == BAD_INDEX) return BAD_INDEX;
            if (inoutTypeSpecFlags) *inoutTypeSpecFlags |= newFlags;
            if (newBlock < 0) return blockIndex;  // e.g. target is a style/bag
            blockIndex = newBlock;
        } else {
            break;
        }
        count++;
    }
    return blockIndex;
}

// ===========================================================================
// obtainStyledAttributes (applyStyle merge) + TypedArray — full attribute
// resolution: element > style= > defStyleAttr(theme) > defStyleRes > theme.
// ===========================================================================

static float complexToFloat(uint32_t data) {
    const uint32_t radix = (data >> Res_value::COMPLEX_RADIX_SHIFT) & Res_value::COMPLEX_RADIX_MASK;
    const uint32_t mantissa = (data >> Res_value::COMPLEX_MANTISSA_SHIFT) & Res_value::COMPLEX_MANTISSA_MASK;
    switch (radix) {
        case Res_value::COMPLEX_RADIX_23p0: return (float)(int32_t)mantissa;
        case Res_value::COMPLEX_RADIX_16p7: return mantissa * (1.0f / (1 << 7));
        case Res_value::COMPLEX_RADIX_8p15: return mantissa * (1.0f / (1 << 15));
        default:                            return mantissa * (1.0f / (1 << 23));
    }
}

void obtainStyledAttributes(const ResXMLTree& xml, const ResTable& table,
                            const ResTable::Theme* theme,
                            const uint32_t* attrs, size_t attrCount,
                            uint32_t defStyleAttr, uint32_t defStyleRes,
                            StyledAttr* out) {
    for (size_t i = 0; i < attrCount; i++) { out[i].set = false; out[i].stringBlock = -1; }
    if (xml.getEventType() != ResXMLParser::START_TAG) return;

    // The element's style= attribute (no namespace, name "style") -> style resId.
    uint32_t styleRes = 0;
    ssize_t styleIdx = xml.indexOfAttribute(nullptr, "style");
    if (styleIdx >= 0) {
        Res_value sv;
        if (xml.getAttributeValue((size_t)styleIdx, &sv) == sizeof(Res_value) &&
            (sv.dataType == Res_value::TYPE_REFERENCE || sv.dataType == Res_value::TYPE_ATTRIBUTE)) {
            styleRes = sv.data;
        }
    }

    // Style/theme fallback chain. Lowest priority is applied first so the
    // sticky "first-set wins" rule yields the right precedence.
    ResTable::Theme chain(table);
    if (defStyleRes) chain.applyStyle(defStyleRes);
    if (defStyleAttr && theme) {
        Res_value dv;
        if (theme->getAttribute(defStyleAttr, &dv) >= 0 &&
            (dv.dataType == Res_value::TYPE_REFERENCE || dv.dataType == Res_value::TYPE_ATTRIBUTE)) {
            chain.applyStyle(dv.data);
        }
    }
    if (styleRes) chain.applyStyle(styleRes);

    const size_t elemCount = xml.getAttributeCount();
    for (size_t i = 0; i < attrCount; i++) {
        const uint32_t a = attrs[i];
        // 1. Element's own attribute (matched by resource id).
        bool found = false;
        for (size_t j = 0; j < elemCount; j++) {
            if (xml.getAttributeNameResID(j) == a) {
                Res_value v;
                if (xml.getAttributeValue(j, &v) == sizeof(Res_value)) {
                    out[i].value = v; out[i].stringBlock = -2; out[i].set = true; found = true;
                }
                break;
            }
        }
        if (found) continue;
        // 2. style / defStyleAttr / defStyleRes chain.
        Res_value v;
        ssize_t blk = chain.getAttribute(a, &v);
        if (blk >= 0) { out[i].value = v; out[i].stringBlock = blk; out[i].set = true; continue; }
        // 3. Theme direct value.
        if (theme && theme->getAttribute(a, &v) >= 0) {
            out[i].value = v; out[i].stringBlock = 0; out[i].set = true; continue;
        }
    }
}

int32_t TypedArray::getInt(size_t idx, int32_t def) const {
    Res_value v; if (!get(idx, &v)) return def;
    return (v.dataType == Res_value::TYPE_INT_DEC || v.dataType == Res_value::TYPE_INT_HEX) ? (int32_t)v.data : def;
}
bool TypedArray::getBoolean(size_t idx, bool def) const {
    Res_value v; if (!get(idx, &v)) return def;
    return v.dataType == Res_value::TYPE_INT_BOOLEAN ? (v.data != 0) : def;
}
uint32_t TypedArray::getColor(size_t idx, uint32_t def) const {
    Res_value v; if (!get(idx, &v)) return def;
    return (v.dataType >= Res_value::TYPE_FIRST_COLOR_INT && v.dataType <= Res_value::TYPE_LAST_COLOR_INT) ? v.data : def;
}
float TypedArray::getDimension(size_t idx, float def) const {
    Res_value v; if (!get(idx, &v)) return def;
    return v.dataType == Res_value::TYPE_DIMENSION ? complexToFloat(v.data) : def;
}
int32_t TypedArray::getDimensionPixelSize(size_t idx, int32_t def) const {
    Res_value v; if (!get(idx, &v)) return def;
    if (v.dataType != Res_value::TYPE_DIMENSION) return def;
    float mag = complexToFloat(v.data);
    int unit = (v.data >> Res_value::COMPLEX_UNIT_SHIFT) & Res_value::COMPLEX_UNIT_MASK;
    float px = (unit == Res_value::COMPLEX_UNIT_PX) ? mag : mag * mDensity;
    return (int32_t)(px + 0.5f);
}
uint32_t TypedArray::getResourceId(size_t idx, uint32_t def) const {
    Res_value v; if (!get(idx, &v)) return def;
    return (v.dataType == Res_value::TYPE_REFERENCE || v.dataType == Res_value::TYPE_ATTRIBUTE) ? v.data : def;
}
std::string TypedArray::getString(size_t idx) const {
    Res_value v; if (!get(idx, &v) || v.dataType != Res_value::TYPE_STRING) return "";
    const ResStringPool* pool = nullptr;
    if (mVals[idx].stringBlock == -2 && mXml) {
        pool = &mXml->getStrings();  // element-sourced: AXML's own pool
    } else if (mVals[idx].stringBlock >= 0 && mVals[idx].stringBlock < (ssize_t)0 /*placeholder*/) {
        // style-sourced: owning arsc header pool (resolved via table on demand)
        // (kept simple: fall back to table's first pool)
        pool = &mTable.getStringPool();
    } else {
        pool = &mTable.getStringPool();
    }
    size_t len = 0;
    const char16_t* s = pool->stringAt(v.data, &len);
    std::string out;
    for (size_t i = 0; s && i < len; i++) {
        uint32_t c = s[i];
        if (c >= 0xD800 && c <= 0xDBFF && i + 1 < len && s[i + 1] >= 0xDC00) c = 0x10000 + ((c - 0xD800) << 10) + (s[++i] - 0xDC00);
        if (c < 0x80) out += (char)c;
        else if (c < 0x800) { out += (char)(0xC0 | (c >> 6)); out += (char)(0x80 | (c & 0x3F)); }
        else { out += (char)(0xE0 | (c >> 12)); out += (char)(0x80 | ((c >> 6) & 0x3F)); out += (char)(0x80 | (c & 0x3F)); }
    }
    return out;
}

} // namespace cdroid
