/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "data_resource.h"
#include <cstring>
#ifdef I18N_PRODUCT
#include <cerrno>
#include <log.h>
#endif
#include "i18n_memory_adapter.h"
#include "str_util.h"

using namespace cdroid::i18n;
using namespace std;

#ifdef I18N_PRODUCT
static const char *DATA_RESOURCE_PATH = "system/i18n/i18n.dat";
#else
static const char *DATA_RESOURCE_PATH = "./i18n.dat";//"/storage/data/i18n.dat";
#endif

// Process-wide in-memory copy of i18n.dat. Loaded ONCE (first Init or
// SetData); all subsequent Init() calls do per-instance locale extraction
// from this buffer with zero syscalls (no open/dup/lseek/read/close).
const char *DataResource::s_data = nullptr;
size_t DataResource::s_dataSize = 0;
bool DataResource::s_dataLoaded = false;

void DataResource::SetData(const char *data, size_t size)
{
    s_data = data;
    s_dataSize = size;
    s_dataLoaded = (data != nullptr && size > 0);
}

bool DataResource::EnsureDataLoaded()
{
    if (s_dataLoaded) return true;
    // If App set a buffer via SetData (pak-loaded), use it.
    if (s_data != nullptr && s_dataSize > 0) {
        s_dataLoaded = true;
        return true;
    }
    // Fallback: read the entire sidecar file into a buffer (never freed —
    // process lifetime, same as the pak buffer would be).
    int32_t infile = open(DATA_RESOURCE_PATH, O_RDONLY);
    if (infile < 0) {
#ifdef I18N_PRODUCT
        HILOG_ERROR(HILOG_MODULE_GLOBAL, "DataResource::EnsureDataLoaded: open failed, errno(%d)!", errno);
#endif
        return false;
    }
    off_t end = lseek(infile, 0, SEEK_END);
    if (end <= 0) { close(infile); return false; }
    s_dataSize = (size_t)end;
    s_data = reinterpret_cast<const char *>(I18nMalloc(s_dataSize));
    if (!s_data) { close(infile); return false; }
    lseek(infile, 0, SEEK_SET);
    ssize_t n = read(infile, const_cast<char *>(s_data), s_dataSize);
    close(infile);
    if ((size_t)n != s_dataSize) {
        I18nFree(const_cast<char *>(s_data));
        s_data = nullptr;
        s_dataSize = 0;
        return false;
    }
    s_dataLoaded = true;
    return true;
}

DataResource::DataResource(const LocaleInfo *localeInfo)
{
    uint32_t enMask = LocaleInfo("en", "US").GetMask();
    if (localeInfo == nullptr) {
        localeMask = enMask;
    } else {
        localeMask = localeInfo->GetMask();
        if (localeInfo->IsDefaultLocale()) {
            fallbackMask = 0;
        } else {
            fallbackMask = GetFallbackMask(*localeInfo);
        }
        if ((fallbackMask != 0) && (fallbackMask != enMask)) {
            defaultMask = enMask;
        }
    }
    for (int i = 0; i < DataResourceType::RESOURCE_TYPE_END; ++i) {
        loaded[i] = DataResourceType::RESOURCE_TYPE_END;
    }
}

DataResource::~DataResource()
{
    if (resourceIndex != nullptr) {
        I18nFree(static_cast<void *>(resourceIndex));
    }
    if (fallbackResourceIndex) {
        I18nFree(static_cast<void *>(fallbackResourceIndex));
    }
    if (defaultResourceIndex) {
        I18nFree(static_cast<void *>(defaultResourceIndex));
    }
    FreeResource();
}

void DataResource::FreeResource()
{
    if (resource != nullptr) {
        while (resourceCount > 0) {
            I18nFree(static_cast<void *>(resource[resourceCount - 1]));
            --resourceCount;
        }
    }
    I18nFree(static_cast<void *>(resource));
    if (fallbackResource != nullptr) {
        while (fallbackResourceCount > 0) {
            I18nFree(static_cast<void *>(fallbackResource[fallbackResourceCount - 1]));
            --fallbackResourceCount;
        }
    }
    I18nFree(static_cast<void *>(fallbackResource));
    if (defaultResource != nullptr) {
        while (defaultResourceCount > 0) {
            I18nFree(static_cast<void *>(defaultResource[defaultResourceCount - 1]));
            --defaultResourceCount;
        }
    }
    I18nFree(static_cast<void *>(defaultResource));
}

char *DataResource::GetString(DataResourceType type) const
{
    uint32_t index = static_cast<uint32_t>(type);
    return GetString(index);
}

char *DataResource::GetString(uint32_t index) const
{
    if (index >= DataResourceType::RESOURCE_TYPE_END) {
        return nullptr;
    }
    uint32_t targetType = loaded[index];
    if (targetType == DataResourceType::RESOURCE_TYPE_END) {
        return nullptr;
    }
    switch (targetType) {
        case LocaleDataType::RESOURCE: {
            return BinarySearchString(resourceIndex, resourceCount, index, resource, resourceCount);
        }
        case LocaleDataType::FALLBACK_RESOURCE: {
            return BinarySearchString(fallbackResourceIndex, fallbackResourceCount, index,
                fallbackResource, fallbackResourceCount);
        }
        default: {
            return BinarySearchString(defaultResourceIndex, defaultResourceCount, index,
                defaultResource, defaultResourceCount);
        }
    }
}

char *DataResource::BinarySearchString(uint32_t *indexArray, uint32_t length, uint32_t target,
    char **stringArray, uint32_t stringLength) const
{
    if ((indexArray == nullptr) || (stringArray == nullptr) || (stringLength == 0) || (length == 0)) {
        return nullptr;
    }
    int32_t low = 0;
    int32_t high = static_cast<int32_t>(length - 1);
    while (low <= high) {
        int32_t mid = low + (high - low) / 2;
        if (mid > static_cast<int32_t>(stringLength)) {
            return nullptr;
        }
        uint32_t temp = indexArray[mid];
        if (temp == target) {
            return stringArray[mid];
        } else if (temp < target) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return nullptr;
}

bool DataResource::Init()
{
    // Load the static buffer once (first call). Subsequent calls skip this
    // and go straight to per-instance locale extraction from the buffer.
    if (!EnsureDataLoaded()) return false;
    if (!ReadHeader()) return false;
    if ((localesCount < 1) || (localesCount > MAX_LOCALE_ITEM_SIZE)) return false;
    return PrepareData();
}

bool DataResource::ReadHeader()
{
    if (!s_data || s_dataSize < GLOBAL_RESOURCE_HEADER_SKIP + GLOBAL_RESOURCE_HEADER_LEFT) {
        return false;
    }
    const char *cache = s_data + GLOBAL_RESOURCE_HEADER_SKIP;
    localesCount = ((static_cast<unsigned char>(cache[0]) << SHIFT_ONE_BYTE) | (static_cast<unsigned char>(cache[1])));
    stringPoolOffset = ((static_cast<unsigned char>(cache[GLOBAL_RESOURCE_INDEX_OFFSET]) << SHIFT_ONE_BYTE) |
        (static_cast<unsigned char>(cache[GLOBAL_RESOURCE_INDEX_OFFSET + 1])));
    return true;
}

bool DataResource::PrepareData()
{
    uint32_t localeSize = localesCount * GLOBAL_LOCALE_MASK_ITEM_SIZE;
    // Locale table follows the header in the buffer — direct pointer, no malloc.
    size_t localeBase = GLOBAL_RESOURCE_HEADER_SKIP + GLOBAL_RESOURCE_HEADER_LEFT;
    if (s_dataSize < localeBase + localeSize) return false;
    char *locales = const_cast<char *>(s_data + localeBase);

    int32_t localeIndex = BinarySearchLocale(localeMask, reinterpret_cast<unsigned char*>(locales));
    int32_t fallbackLocaleIndex = -1;
    int32_t defaultLocaleIndex = -1;
    GetFallbackAndDefaultLocaleIndex(fallbackLocaleIndex, defaultLocaleIndex, locales);
    uint32_t configOffset = 0;
    if (localeIndex >= 0) {
        configOffset = ConvertUChar(reinterpret_cast<unsigned char*>(locales + GLOBAL_LOCALE_MASK_ITEM_SIZE *
            localeIndex + GLOBAL_RESOURCE_MASK_OFFSET));
        resourceCount = ConvertUChar(reinterpret_cast<unsigned char*>(locales + GLOBAL_LOCALE_MASK_ITEM_SIZE *
            localeIndex + GLOBAL_RESOURCE_MASK_OFFSET + GLOBAL_RESOURCE_INDEX_OFFSET));
    }
    uint32_t fallbackConfigOffset = 0;
    uint32_t defaultConfigOffset = 0;
    GetFallbackAndDefaultInfo(fallbackLocaleIndex, defaultLocaleIndex, fallbackConfigOffset, defaultConfigOffset,
        locales);
    // No I18nFree(locales) — it's a pointer into the static buffer.
    bool ret = true;
    if (IsTypeNeeded(localeIndex, resourceCount)) {
        ret = PrepareLocaleData(configOffset, resourceCount, LocaleDataType::RESOURCE);
    }
    if (IsTypeNeeded(fallbackLocaleIndex, fallbackResourceCount)) {
        ret = PrepareLocaleData(fallbackConfigOffset, fallbackResourceCount,
            LocaleDataType::FALLBACK_RESOURCE);
    }
    if (IsTypeNeeded(defaultLocaleIndex, defaultResourceCount)) {
        ret = PrepareLocaleData(defaultConfigOffset, defaultResourceCount, LocaleDataType::DEFAULT_RESOURCE);
    }
    return ret;
}

bool DataResource::IsTypeNeeded(int32_t index, uint32_t count)
{
    if ((index < 0) || FullLoaded()) {
        return false;
    }
    return (count > 0) && (count <= DataResourceType::RESOURCE_TYPE_END);
}

void DataResource::GetFallbackAndDefaultLocaleIndex(int32_t &fallbackLocaleIndex, int32_t &defaultLocaleIndex,
    char *locales)
{
    if (fallbackMask != 0) {
        fallbackLocaleIndex = BinarySearchLocale(fallbackMask, reinterpret_cast<unsigned char*>(locales));
    }
    if (defaultMask != 0) {
        defaultLocaleIndex = BinarySearchLocale(defaultMask, reinterpret_cast<unsigned char*>(locales));
    }
}

void DataResource::GetFallbackAndDefaultInfo(const int32_t &fallbackLocaleIndex, const int32_t &defaultLocaleIndex,
    uint32_t &fallbackConfigOffset, uint32_t &defaultConfigOffset, char* locales)
{
    if (fallbackLocaleIndex != -1) {
        fallbackConfigOffset = ConvertUChar(reinterpret_cast<unsigned char*>(locales + GLOBAL_LOCALE_MASK_ITEM_SIZE *
            fallbackLocaleIndex + GLOBAL_RESOURCE_MASK_OFFSET));
        fallbackResourceCount = ConvertUChar(reinterpret_cast<unsigned char*>(locales + GLOBAL_LOCALE_MASK_ITEM_SIZE *
            fallbackLocaleIndex + GLOBAL_RESOURCE_MASK_OFFSET + GLOBAL_RESOURCE_INDEX_OFFSET));
    }
    if (defaultLocaleIndex != -1) {
        defaultConfigOffset = ConvertUChar(reinterpret_cast<unsigned char*>(locales + GLOBAL_LOCALE_MASK_ITEM_SIZE *
            defaultLocaleIndex + GLOBAL_RESOURCE_MASK_OFFSET));
        defaultResourceCount = ConvertUChar(reinterpret_cast<unsigned char*>(locales + GLOBAL_LOCALE_MASK_ITEM_SIZE *
            defaultLocaleIndex + GLOBAL_RESOURCE_MASK_OFFSET + GLOBAL_RESOURCE_INDEX_OFFSET));
    }
}

bool DataResource::PrepareLocaleData(uint32_t configOffset, uint32_t count, LocaleDataType type)
{
    currentType = type;
    if (count < 1 || count > DataResourceType::RESOURCE_TYPE_END) {
        return false;
    }
    uint32_t resourceSize = count * GLOBAL_RESOURCE_CONFIG_SIZE;
    if (s_dataSize < configOffset + resourceSize) {
        return false;
    }
    // Configs are a direct pointer into the static buffer — no malloc, no read.
    char *configs = const_cast<char *>(s_data + configOffset);
    bool ret = GetStringFromStringPool(configs, resourceSize, type);
    // No I18nFree(configs) — it's part of the static buffer.
    return ret;
}

uint32_t DataResource::GetFinalCount(char *configs, uint32_t configSize, LocaleDataType type)
{
    uint32_t count = 0;
    switch (type) {
        case LocaleDataType::RESOURCE: {
            count = resourceCount;
            break;
        }
        case LocaleDataType::FALLBACK_RESOURCE: {
            count = fallbackResourceCount;
            break;
        }
        default: {
            count = defaultResourceCount;
        }
    }
    uint32_t finalCount = 0;
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t index = ConvertUChar(reinterpret_cast<unsigned char*>(configs + i * GLOBAL_RESOURCE_CONFIG_SIZE));
        if (index >= DataResourceType::RESOURCE_TYPE_END) {
            return 0;
        }
        if (loaded[index] != DataResourceType::RESOURCE_TYPE_END) {
            continue;
        } else {
            loaded[index] = type;
        }
        ++finalCount;
    }
    return finalCount;
}

bool DataResource::GetStringFromStringPool(char *configs, const uint32_t configsSize, LocaleDataType type)
{
    uint32_t finalCount = GetFinalCount(configs, configsSize, type);
    if (finalCount == 0) {
        return true;
    }
    uint32_t **index = nullptr;
    char ***wanted = nullptr;
    uint32_t originalCount = 0;
    switch (type) {
        case LocaleDataType::RESOURCE: {
            originalCount = resourceCount;
            resourceCount = finalCount;
            index = &resourceIndex;
            wanted = &resource;
            break;
        }
        case LocaleDataType::FALLBACK_RESOURCE: {
            originalCount = fallbackResourceCount;
            fallbackResourceCount = finalCount;
            index = &fallbackResourceIndex;
            wanted = &fallbackResource;
            break;
        }
        default: {
            originalCount = defaultResourceCount;
            defaultResourceCount = finalCount;
            index = &defaultResourceIndex;
            wanted = &defaultResource;
            break;
        }
    }
    if (!ApplyForResource(index, wanted, finalCount)) {
        return false;
    }
    return Retrieve(configs, configsSize, originalCount, type);
}

void DataResource::GetType(char** &adjustResource, uint32_t* &adjustResourceIndex, uint32_t &count,
    LocaleDataType type)
{
    switch (type) {
        case LocaleDataType::RESOURCE: {
            adjustResource = resource;
            adjustResourceIndex = resourceIndex;
            count = resourceCount;
            break;
        }
        case LocaleDataType::FALLBACK_RESOURCE: {
            adjustResource = fallbackResource;
            adjustResourceIndex = fallbackResourceIndex;
            count = fallbackResourceCount;
            break;
        }
        default: {
            adjustResource = defaultResource;
            adjustResourceIndex = defaultResourceIndex;
            count = defaultResourceCount;
            break;
        }
    }
}

bool DataResource::Retrieve(char *configs, uint32_t configsSize, const uint32_t orginalCount,
    LocaleDataType type)
{
    uint32_t count = 0;
    char **adjustResource = nullptr;
    uint32_t *adjustResourceIndex = nullptr;
    GetType(adjustResource, adjustResourceIndex, count, type);
    uint32_t currentIndex = 0;
    for (uint32_t i = 0; i < orginalCount; ++i) {
        uint32_t index = ConvertUChar(reinterpret_cast<unsigned char*>(configs + i * GLOBAL_RESOURCE_CONFIG_SIZE));
        if (loaded[index] != type) {
            continue;
        }
        uint32_t offset = ConvertUChar(reinterpret_cast<unsigned char*>(configs + i *
            GLOBAL_RESOURCE_CONFIG_SIZE + GLOBAL_RESOURCE_INDEX_OFFSET));
        uint32_t length = ConvertUChar(reinterpret_cast<unsigned char*>(configs + i *
            GLOBAL_RESOURCE_CONFIG_SIZE + GLOBAL_RESOURCE_MASK_OFFSET));
        size_t strPos = stringPoolOffset + offset;
        if ((length == 0) || (s_dataSize < strPos + length)) {
            adjustResource[currentIndex] = nullptr;
            adjustResourceIndex[currentIndex] = index;
        } else {
            // Copy from the static buffer into a owned, null-terminated string.
            char *temp = reinterpret_cast<char *>(I18nMalloc(length + 1));
            if (temp == nullptr) {
                loaded[index] = DataResourceType::RESOURCE_TYPE_END;
                return false;
            }
            memcpy(temp, s_data + strPos, length);
            temp[length] = 0;
            adjustResource[currentIndex] = temp;
            adjustResourceIndex[currentIndex] = index;
        }
        ++currentIndex;
    }
    return true;
}

bool DataResource::ApplyForResource(uint32_t **index, char ***wanted, uint32_t totalCount)
{
    if ((index == nullptr) || (wanted == nullptr) || (totalCount == 0)) {
        return false;
    }
    *index = reinterpret_cast<uint32_t *>(I18nMalloc(sizeof(uint32_t) * totalCount));
    if (*index == nullptr) {
        return false;
    }
    *wanted = reinterpret_cast<char **>(I18nMalloc(sizeof(char *) * totalCount));
    if (*wanted == nullptr) {
        return false; // free *index in FreeResource
    }
    for (uint32_t i = 0; i < totalCount; ++i) {
        (*wanted)[i] = nullptr;
    }
    return true;
}

int32_t DataResource::BinarySearchLocale(const uint32_t mask, unsigned char *locales)
{
    if ((locales == nullptr) || (localesCount == 0)) {
        return -1;
    }
    int32_t low = 0;
    int32_t high = static_cast<int32_t>(localesCount - 1);
    while (low <= high) {
        int32_t mid = low + (high - low) / 2;
        if (mid > 1024) { // locales count < 1024
            return -1;
        }
        uint32_t midMask = ConvertUint(locales + mid * GLOBAL_LOCALE_MASK_ITEM_SIZE);
        if (midMask == mask) {
            return mid;
        } else if (midMask < mask) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1;
}

uint32_t DataResource::ConvertUint(unsigned char *src)
{
    if (src == nullptr) {
        return 0;
    }
    uint32_t ret = 0;
    ret |= (src[0] << SHIFT_THREE_BYTE); // 0 indicates first byte
    ret |= (src[1] << SHIFT_TWO_BYTE); // 1 indicates second byte
    ret |= (src[2] << SHIFT_ONE_BYTE); // 2 indicates third byte
    ret |= src[3]; // 3 indicates forth byte
    return ret;
}

uint32_t DataResource::ConvertUChar(unsigned char *src)
{
    if (src == nullptr) {
        return 0;
    }
    uint32_t ret = 0;
    ret |= (src[0] << SHIFT_ONE_BYTE);
    ret |= src[1];
    return ret;
}

bool DataResource::FullLoaded()
{
    for (uint32_t i = 0; i < DataResourceType::RESOURCE_TYPE_END; ++i) {
        if (loaded[i] == DataResourceType::RESOURCE_TYPE_END) {
            return false;
        }
    }
    return true;
}

uint32_t DataResource::GetFallbackMask(const LocaleInfo &src)
{
    const char *language = src.GetLanguage();
    const char *script = src.GetScript();
    const char *region = src.GetRegion();
    if ((language != nullptr) && (strcmp("en", language) == 0) && (script == nullptr)) {
        return LocaleInfo("en", "", "US").GetMask();
    }
    if (region == nullptr) {
        return LocaleInfo("en", "US").GetMask();
    }
    if (script == nullptr) {
        return LocaleInfo(language, "", "").GetMask();
    }
    return LocaleInfo(language, script, "").GetMask();
}

void DataResource::GetString(DataResourceType type, std::string &ret) const
{
    uint32_t index = static_cast<uint32_t>(type);
    char *temp = GetString(index);
    if (temp == nullptr) {
        ret = "";
    } else {
        ret = temp;
    }
}
