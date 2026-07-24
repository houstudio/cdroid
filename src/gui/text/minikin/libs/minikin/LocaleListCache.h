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

#ifndef MINIKIN_LOCALE_LIST_CACHE_H
#define MINIKIN_LOCALE_LIST_CACHE_H

#include <mutex>
#include <unordered_map>

#include "minikin/Buffer.h"
#include "minikin/Macros.h"

#include "Locale.h"

namespace minikin {

class LocaleListCache {
public:
    // A special ID for the invalid locale list.
    const static uint32_t kInvalidListId = (uint32_t)(-1);

    // Returns the locale list ID for the given string representation of LocaleList.
    static inline uint32_t getId(const std::string& locales) {
        return getInstance().getIdInternal(locales);
    }

    // Returns the locale list ID for the LocaleList serialized in the buffer.
    static inline uint32_t readFrom(BufferReader* reader) {
        return getInstance().readFromInternal(reader);
    }

    static inline void writeTo(BufferWriter* writer, uint32_t id) {
        return getInstance().writeToInternal(writer, id);
    }

    static inline const LocaleList& getById(uint32_t id) {
        return getInstance().getByIdInternal(id);
    }

private:
    struct LocaleVectorHash {
        size_t operator()(const std::vector<Locale>& locales) const;
    };

    LocaleListCache();  // Singleton
    ~LocaleListCache() {}

    uint32_t getIdInternal(const std::string& locales);
    uint32_t getIdInternal(std::vector<Locale>&& locales) EXCLUSIVE_LOCKS_REQUIRED(mMutex);
    uint32_t readFromInternal(BufferReader* reader);
    void writeToInternal(BufferWriter* writer, uint32_t id);
    const LocaleList& getByIdInternal(uint32_t id);

    // Caller should acquire a lock before calling the method.
    static LocaleListCache& getInstance() {
        static LocaleListCache instance;
        return instance;
    }

    std::vector<LocaleList> mLocaleLists GUARDED_BY(mMutex);

    // A map from the list of locale identifier to the ID.
    //
    // Locale's operator==() doesn't have reflexivity for unsupported locales,
    // but it won't cause problems because we never store unsupported locales in
    // LocaleListCache. See parseLocaleList() in LocaleListCache.cpp.
    std::unordered_map<std::vector<Locale>, uint32_t, LocaleVectorHash> mLocaleListLookupTable
            GUARDED_BY(mMutex);

    // A cache map from the string representation of the font locale list to the ID.
    // This is a mere cache over mLocaleListLookupTable. Some LocaleList objects may be in
    // mLocaleListLookupTable even if they are not in mLocaleListStringCache.
    std::unordered_map<std::string, uint32_t> mLocaleListStringCache GUARDED_BY(mMutex);

    std::mutex mMutex;
};

}  // namespace minikin

#endif  // MINIKIN_LOCALE_LIST_CACHE_H
