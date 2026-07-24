/*
 * Copyright (C) 2020 The Android Open Source Project
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

#ifndef MINIKIN_BOUNDS_CACHE_H
#define MINIKIN_BOUNDS_CACHE_H

#include "minikin/LayoutCache.h"

#include <mutex>

#include <utils/LruCache.h>

#include "minikin/BoundsCache.h"
#include "minikin/FontCollection.h"
#include "minikin/Hasher.h"
#include "minikin/MinikinPaint.h"

namespace minikin {

// Cache entry
struct BoundsValue {
    MinikinRect rect;
    float advance;
};

// Used for callback for LayoutCache.
struct ValueExtractor {
    void operator()(const LayoutPiece& layoutPiece, const MinikinPaint& paint);
    std::unique_ptr<BoundsValue> value;
};

class BoundsCache : private android::OnEntryRemoved<LayoutCacheKey, BoundsValue*> {
public:
    void clear() {
        std::lock_guard<std::mutex> lock(mMutex);
        mCache.clear();
    }

    // Do not use BoundsCache inside the callback function, otherwise dead-lock may happen.
    template <typename F>
    void getOrCreate(const U16StringPiece& text, const Range& range, const MinikinPaint& paint,
                     bool dir, StartHyphenEdit startHyphen, EndHyphenEdit endHyphen, F& f) {
        LayoutCacheKey key(text, range, paint, dir, startHyphen, endHyphen);
        if (paint.skipCache() || range.getLength() >= LENGTH_LIMIT_CACHE) {
            LayoutPiece piece = LayoutPiece(text, range, dir, paint, startHyphen, endHyphen);
            f(getBounds(piece, paint), piece.advance());
            return;
        }
        {
            std::lock_guard<std::mutex> lock(mMutex);
            BoundsValue* value = mCache.get(key);
            if (value != nullptr) {
                f(value->rect, value->advance);
                return;
            }
        }
        // Doing text layout takes long time, so releases the mutex during doing layout.
        // Don't care even if we do the same layout in other thread.
        key.copyText();
        ValueExtractor ve;
        LayoutCache::getInstance().getOrCreate(text, range, paint, dir, startHyphen, endHyphen, ve);
        f(ve.value->rect, ve.value->advance);
        {
            std::lock_guard<std::mutex> lock(mMutex);
            mCache.put(key, ve.value.release());
        }
    }

    static BoundsCache& getInstance() {
        static BoundsCache cache(kMaxEntries);
        return cache;
    }

    // Compute new bounding box for the layout piece.
    static MinikinRect getBounds(const LayoutPiece& layoutPiece, const MinikinPaint& paint);

protected:
    BoundsCache(uint32_t maxEntries) : mCache(maxEntries) {
        mCache.setOnEntryRemovedListener(this);
    }

private:
    // callback for OnEntryRemoved
    void operator()(LayoutCacheKey& key, BoundsValue*& value) {
        key.freeText();
        delete value;
    }

    std::mutex mMutex;
    android::LruCache<LayoutCacheKey, BoundsValue*> mCache GUARDED_BY(mMutex) GUARDED_BY(mMutex);
    // LRU cache capacity. Should be fine to be less than LayoutCache#kMaxEntries since bbox
    // calculation happens less than layout calculation.
    static const size_t kMaxEntries = 500;
};

}  // namespace minikin
#endif  // MINIKIN_BOUNDS_CACHE_H
