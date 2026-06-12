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

#ifndef MINIKIN_SYSTEM_FONTS_H
#define MINIKIN_SYSTEM_FONTS_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <functional>
#include "minikin/FontCollection.h"
#include "minikin/U16StringPiece.h"

namespace minikin {

// Provides a system font mapping.
class SystemFonts {
public:
    static std::shared_ptr<FontCollection> findFontCollection(const std::string& familyName) {
        return getInstance().findFontCollectionInternal(familyName);
    }

    static void registerFallback(const std::string& familyName,
                                 const std::shared_ptr<FontCollection>& fc) {
        return getInstance().registerFallbackInternal(familyName, fc);
    }

    static void registerDefault(const std::shared_ptr<FontCollection>& fc) {
        return getInstance().registerDefaultInternal(fc);
    }

    using FontMapDeleter = std::function<void()>;

    static void addFontMap(std::shared_ptr<FontCollection>&& collections) {
        return getInstance().addFontMapInternal(std::move(collections));
    }

    // This obtains a mutex inside, so do not call this method inside callback.
    static void getFontSet(std::function<void(const std::vector<std::shared_ptr<Font>>&)> func) {
        return getInstance().getFontSetInternal(func);
    }

protected:
    // Visible for testing purposes.
    SystemFonts() {}
    virtual ~SystemFonts() {}

    std::shared_ptr<FontCollection> findFontCollectionInternal(const std::string& familyName);
    void registerFallbackInternal(const std::string& familyName,
                                  const std::shared_ptr<FontCollection>& fc) {
        std::lock_guard<std::mutex> lock(mMutex);
        mSystemFallbacks[familyName] = fc;
    }

    void registerDefaultInternal(const std::shared_ptr<FontCollection>& fc) {
        std::lock_guard<std::mutex> lock(mMutex);
        mDefaultFallback = fc;
    }

    void addFontMapInternal(std::shared_ptr<FontCollection>&& collections) {
        std::lock_guard<std::mutex> lock(mMutex);
        mCollections.emplace_back(std::move(collections));
    }

    void getFontSetInternal(std::function<void(const std::vector<std::shared_ptr<Font>>&)> func) {
        std::lock_guard<std::mutex> lock(mMutex);
        if (mFonts.empty()){//!mFonts) {
            buildFontSetLocked();
        }
        func(mFonts);//.value());
    }

private:
    static SystemFonts& getInstance();

    void buildFontSetLocked() EXCLUSIVE_LOCKS_REQUIRED(mMutex);

    std::map<std::string, std::shared_ptr<FontCollection>> mSystemFallbacks GUARDED_BY(mMutex);
    std::shared_ptr<FontCollection> mDefaultFallback GUARDED_BY(mMutex);
    std::vector<std::shared_ptr<FontCollection>> mCollections GUARDED_BY(mMutex);
    std::vector<std::shared_ptr<Font>> mFonts GUARDED_BY(mMutex);
    //std::optional<std::vector<std::shared_ptr<Font>>> mFonts GUARDED_BY(mMutex);

    std::mutex mMutex;
};

}  // namespace minikin

#endif  // MINIKIN_SYSTEM_FONTS_H
