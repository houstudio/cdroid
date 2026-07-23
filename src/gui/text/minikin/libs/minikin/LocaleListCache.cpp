/*
 * Copyright (C) 2015 The Android Open Source Project
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

#include "LocaleListCache.h"

#include <unordered_set>

#include <log/log.h>
#include <minikin/Hasher.h>
#include <minikin/LocaleList.h>
#include <unicode/uloc.h>
#include <unicode/umachine.h>

#include "Locale.h"
#include "MinikinInternal.h"

namespace minikin {

// Returns the text length of output.
static size_t toLanguageTag(char* output, size_t outSize, const StringPiece& locale) {
    output[0] = '\0';
    if (locale.empty()) {
        return 0;
    }

    std::string localeString = locale.toString();  // ICU only understands C-style string.

    size_t outLength = 0;
    UErrorCode uErr = U_ZERO_ERROR;
    outLength = uloc_canonicalize(localeString.c_str(), output, outSize, &uErr);
    if (U_FAILURE(uErr) || (uErr == U_STRING_NOT_TERMINATED_WARNING)) {
        // unable to build a proper locale identifier
        ALOGD("uloc_canonicalize(\"%s\") failed: %s", localeString.c_str(), u_errorName(uErr));
        output[0] = '\0';
        return 0;
    }

    // Preserve "" and "_****" since uloc_addLikelySubtags changes "" to "en_Latn_US".
    if (outLength == 0 || (outLength == 5 && output[0] == '_')) {
        if (output[0] == '_') {
            output[0] = '-';
        }
        std::string buf(output, outLength);
        outLength = (size_t)snprintf(output, outSize, "und%s", buf.c_str());
        return outLength;
    }

    char likelyChars[ULOC_FULLNAME_CAPACITY];
    uErr = U_ZERO_ERROR;
    uloc_addLikelySubtags(output, likelyChars, ULOC_FULLNAME_CAPACITY, &uErr);
    if (U_FAILURE(uErr) || (uErr == U_STRING_NOT_TERMINATED_WARNING)) {
        // unable to build a proper locale identifier
        ALOGD("uloc_addLikelySubtags(\"%s\") failed: %s", output, u_errorName(uErr));
        output[0] = '\0';
        return 0;
    }

    uErr = U_ZERO_ERROR;
    outLength = uloc_toLanguageTag(likelyChars, output, outSize, false, &uErr);
    if (U_FAILURE(uErr) || (uErr == U_STRING_NOT_TERMINATED_WARNING)) {
        // unable to build a proper locale identifier
        ALOGD("uloc_toLanguageTag(\"%s\") failed: %s", likelyChars, u_errorName(uErr));
        output[0] = '\0';
        return 0;
    }
    return outLength;
}

static std::vector<Locale> parseLocaleList(const std::string& input) {
    std::vector<Locale> result;
    char langTag[ULOC_FULLNAME_CAPACITY];
    std::unordered_set<uint64_t> seen;

    SplitIterator it(input, ',');
    while (it.hasNext()) {
        StringPiece localeStr = it.next();
        size_t length = toLanguageTag(langTag, ULOC_FULLNAME_CAPACITY, localeStr);
        Locale locale(StringPiece(langTag, length));
        if (locale.isUnsupported()) {
            continue;
        }
        const bool isNewLocale = seen.insert(locale.getIdentifier()).second;
        if (!isNewLocale) {
            continue;
        }

        result.push_back(locale);
        if (result.size() >= FONT_LOCALE_LIMIT) {
            break;
        }
    }
    return result;
}

size_t LocaleListCache::LocaleVectorHash::operator()(const std::vector<Locale>& locales) const {
    Hasher hasher;
    for (const auto& locale : locales) {
        uint64_t id = locale.getIdentifier();
        hasher.update(static_cast<uint32_t>((id >> 32) & 0xFFFFFFFF));
        hasher.update(static_cast<uint32_t>(id & 0xFFFFFFFF));
    }
    return hasher.hash();
}

LocaleListCache::LocaleListCache() {
    // Insert an empty locale list for mapping default locale list to kEmptyLocaleListId.
    // The default locale list has only one Locale and it is the unsupported locale.
    mLocaleLists.emplace_back();
    mLocaleListLookupTable.emplace(std::vector<Locale>(), kEmptyLocaleListId);
    mLocaleListStringCache.emplace("", kEmptyLocaleListId);
}

uint32_t LocaleListCache::getIdInternal(const std::string& locales) {
    std::lock_guard<std::mutex> lock(mMutex);
    const auto& it = mLocaleListStringCache.find(locales);
    if (it != mLocaleListStringCache.end()) {
        return it->second;
    }
    uint32_t id = getIdInternal(parseLocaleList(locales));
    mLocaleListStringCache.emplace(locales, id);
    return id;
}

uint32_t LocaleListCache::getIdInternal(std::vector<Locale>&& locales) {
    if (locales.empty()) {
        return kEmptyLocaleListId;
    }
    const auto& it = mLocaleListLookupTable.find(locales);
    if (it != mLocaleListLookupTable.end()) {
        return it->second;
    }

    // Given locale list is not in cache. Insert it and return newly assigned ID.
    const uint32_t nextId = mLocaleLists.size();
    mLocaleListLookupTable.emplace(locales, nextId);
    LocaleList fontLocales(std::move(locales));
    mLocaleLists.push_back(std::move(fontLocales));
    return nextId;
}

uint32_t LocaleListCache::readFromInternal(BufferReader* reader) {
    uint32_t size = reader->read<uint32_t>();
    std::vector<Locale> locales;
    locales.reserve(size);
    for (uint32_t i = 0; i < size; i++) {
        locales.emplace_back(reader->read<uint64_t>());
    }
    std::lock_guard<std::mutex> lock(mMutex);
    return getIdInternal(std::move(locales));
}

void LocaleListCache::writeToInternal(BufferWriter* writer, uint32_t id) {
    const LocaleList& localeList = getByIdInternal(id);
    writer->write<uint32_t>(localeList.size());
    for (size_t i = 0; i < localeList.size(); i++) {
        writer->write<uint64_t>(localeList[i].getIdentifier());
    }
}

const LocaleList& LocaleListCache::getByIdInternal(uint32_t id) {
    std::lock_guard<std::mutex> lock(mMutex);
    MINIKIN_ASSERT(id < mLocaleLists.size(), "Lookup by unknown locale list ID.");
    return mLocaleLists[id];
}

}  // namespace minikin
