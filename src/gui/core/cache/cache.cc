#include<cache.h>
namespace cdroid {

RefPtr<ImageSurface> BaseMemoryCache::get(const std::string& key) {
    Reference<ImageSurface> ref = imagemaps.get(key);
    return ref;
}

bool BaseMemoryCache::put(const std::string& key,RefPtr<ImageSurface> value) {
    imagemaps[key]= value;
    return true;
}

RefPtr<ImageSurface> BaseMemoryCache::remove(const std::string& key) {
    RefPtr<ImageSurface>ref=imagemaps.get(key);
    imagemaps.erase(key);
    return ref;
}

size_t BaseMemoryCache::getkeys(std::vector<std::string>&) {
    return new HashSet<String>(softMap.keySet());
}

void BaseMemoryCache::clear() {
    imagemaps.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////
LimitedMemoryCache::LimitedMemoryCache(int sizeLimit):BaseMemoryCache() {
    sizeLimit = sizeLimit;
    cacheSize=0;
}

bool LimitedMemoryCache::put(const std::string&key, RefPtr<ImageSurface>value) {
    bool putSuccessfully = false;
    // Try to add value to hard cache
    int valueSize = getSize(value);
    int sizeLimit = getSizeLimit();
    int curCacheSize = cacheSize;
    if (valueSize < sizeLimit) {
        while (curCacheSize + valueSize > sizeLimit) {
            RefPtr<ImageSurface>removedValue = removeNext();
            if (hardCache.remove(removedValue)) {
                curCacheSize-=getSize(removedValue);
            }
        }
        hardCache.add(value);
        cacheSize+=valueSize;

        putSuccessfully = true;
    }
    // Add value to soft cache
    BaseMemoryCache::put(key, value);
    return putSuccessfully;
}

RefPtr<ImageSurface>LimitedMemoryCache::remove(const std::string& key) {
    RefPtr<ImageSurface> value = BaseMemoryCache::get(key);
    if (value != null) {
        if (hardCache.remove(value)) {
            cacheSize.addAndGet(-getSize(value));
        }
    }
    return BaseMemoryCache::remove(key);
}

void LimitedMemoryCache::clear() {
    hardCache.clear();
    cacheSize=0;
    BaseMemoryCache::clear();
}

}
