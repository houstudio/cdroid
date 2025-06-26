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

#ifndef DATA_RESOURCE_IMPL_H
#define DATA_RESOURCE_IMPL_H

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "locale_info.h"
#include "fcntl.h"
#include "types.h"

#define GLOBAL_RESOURCE_HEADER_SIZE 16
#define GLOBAL_RESOURCE_HEADER_LEFT 4
#define GLOBAL_RESOURCE_HEADER_SKIP 12
#define GLOBAL_RESOURCE_INDEX_OFFSET 2
#define GLOBAL_RESOURCE_MASK_OFFSET 4
#define GLOBAL_LOCALE_MASK_ITEM_SIZE 8
#define GLOBAL_RESOURCE_CONFIG_SIZE 6
#define MONTH_SEP '_'
#define DAY_SEP '_'
#define PATTERN_SEP '_'
#define AMPM_SEP '_'
#define NUM_PATTERN_SIZE 5
#define NUM_PAT_INDEX 0
#define NUM_PERCENT_PAT_INDEX 1
#define NUM_DEC_SIGN_INDEX 2
#define NUM_GROUP_SIGN_INDEX 3
#define NUM_PERCENT_SIGN_INDEX 4
#define NUM_PATTERN_SEP '_'
#define NUM_DIGIT_SEP ';'
#define PLURAL_SEP '_'
#define NUM_DIGIT_SIZE 10
#define RULES_NUM 6
#define RESOURCE_INDEX 0
#define FALLBACK_RESOURCE_INDEX 1
#define DEFAULT_RESOURCE_INDEX 2
#define SHIFT_ONE_BYTE 8
#define SHIFT_TWO_BYTE 16
#define SHIFT_THREE_BYTE 24
#define MAX_LOCALE_ITEM_SIZE 500
#define MEASURE_BASE_ITEM_COUNT 4
#define MEASURE_SINGLE_UNIT_COUNT 24
#define MEASURE_FORMAT_TYPE_NUM 4
#define MEASURE_PLURAL_NUM 6
#define MAX_MEASURE_FORMAT_LENGTH 128
#define MEASURE_UNIT_SEP '|'

namespace OHOS {
namespace I18N {
enum LocaleDataType {
    RESOURCE = 0,
    FALLBACK_RESOURCE,
    DEFAULT_RESOURCE
};

enum DataResourceType {
    RESOURCE_TYPE_BEGIN = 0,
    GREGORIAN_FORMAT_ABBR_MONTH = RESOURCE_TYPE_BEGIN, // 0
    GREGORIAN_FORMAT_ABBR_DAY, // 1
    GREGORIAN_TIME_PATTERNS, // 2
    GREGORIAN_DATE_PATTERNS, // 3
    GREGORIAN_AM_PMS, // 4
    PLURAL, // 5
    NUMBER_FORMAT, // 6
    NUMBER_DIGIT, // 7
    TIME_SEPARATOR, // 8
    DEFAULT_HOUR, // 9
    GREGORIAN_STANDALONE_ABBR_MONTH, // 10
    GREGORIAN_STANDALONE_ABBR_DAY, // 11
    GREGORIAN_FORMAT_WIDE_MONTH, // 12
    GREGORIAN_HOUR_MINUTE_SECOND_PATTERN, // 13
    GREGORIAN_FULL_MEDIUM_SHORT_PATTERN, // 14
    GREGORIAN_FORMAT_WIDE_DAY, // 15
    GREGORIAN_STANDALONE_WIDE_DAY, // 16
    GREGORIAN_STANDALONE_WIDE_MONTH, // 17
    ELAPSED_PATERNS, // 18
    WEEK_DATA, // 19
    DECIMAL_PLURAL, // 20
    MINUS_SIGN, // 21
    MEASURE_FORMAT_PATTERNS, // 22
    RESOURCE_TYPE_END // 23
};

class DataResource {
public:
    explicit DataResource(const LocaleInfo *localeInfo);
    bool Init();
    char *GetString(DataResourceType type) const;
    virtual ~DataResource();
    void GetString(DataResourceType type, std::string &ret) const;

private:
    static uint32_t GetFallbackMask(const LocaleInfo &src);
    bool ReadHeader(int32_t infile);
    bool PrepareData(int32_t infile);
    int32_t BinarySearchLocale(const uint32_t mask, unsigned char *locales);
    bool GetStringFromStringPool(char *configs, const uint32_t configsSize, int32_t infile, LocaleDataType type);
    uint32_t ConvertUint(unsigned char *src);
    uint32_t ConvertUChar(unsigned char *src);
    char *GetString2(DataResourceType type) const;
    char *GetString(uint32_t index) const;
    char *BinarySearchString(uint32_t *indexArray, uint32_t length,
        uint32_t target, char **stringArray, uint32_t stringLength) const;
    LocaleInfo *GetFallbackLocaleInfo(const LocaleInfo &src);
    void GetFallbackAndDefaultLocaleIndex(int32_t &fallbackLocaleIndex, int32_t &defaultLocaleIndex,
        char *locales);
    void GetFallbackAndDefaultInfo(const int32_t &fallbackLocaleIndex, const int32_t &defaultLocaleIndex,
        uint32_t &fallbackConfigOffset, uint32_t &defaultConfigOffset, char *locales);
    bool Retrieve(char *configs, const uint32_t configsSize, int32_t infile, const uint32_t originalCount,
        LocaleDataType type);
    bool PrepareLocaleData(int32_t infile, uint32_t configOffset, uint32_t count, LocaleDataType type);
    bool FullLoaded();
    void GetType(char** &adjustResource, uint32_t* &adjustResourceIndex, uint32_t &count, LocaleDataType type);
    uint32_t GetFinalCount(char *configs, uint32_t configSize, LocaleDataType type);
    void FreeResource();
    bool IsTypeNeeded(int32_t index, uint32_t count);
    bool ApplyForResource(uint32_t **index, char ***wanted, uint32_t totalCount);
    uint32_t localeMask = 0;
    uint32_t fallbackMask = 0;
    uint32_t defaultMask = 0;
    uint32_t localesCount = 0;
    uint32_t stringPoolOffset = 0;
    uint32_t *resourceIndex = nullptr;
    uint32_t *defaultResourceIndex = nullptr;
    char **resource = nullptr;
    uint32_t resourceCount = 0;
    uint32_t fallbackResourceCount = 0;
    uint32_t defaultResourceCount = 0;
    LocaleDataType currentType = LocaleDataType::DEFAULT_RESOURCE;
    uint32_t *fallbackResourceIndex = nullptr;
    char **fallbackResource = nullptr;
    char **defaultResource = nullptr;
    uint32_t loaded[DataResourceType::RESOURCE_TYPE_END] = { 0 };
};
} // namespace I18N
} // namespace OHOS
#endif