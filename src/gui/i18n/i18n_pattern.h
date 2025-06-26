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

#ifndef GLOBAL_I18N_PATTERN_H
#define GLOBAL_I18N_PATTERN_H

#include <string>
#include "date_time_data.h"
#include "str_util.h"

// this file should only be included by date_time_format_impl.cpp
#define HOUR12_MINUTE_INDEX                     0x00000000
#define HOUR24_MINUTE_INDEX                     0x00000001
#define HOUR_MINUTE_INDEX                       0x00000002
#define ABBR_MONTH_WEEKDAY_DAY_INDEX            0x00010000
#define ABBR_MONTH_DAY_INDEX                    0x00010001
#define YEAR_ABBR_MONTH_ABBR_WEEKDAY_DAY_INDEX  0x00010002
#define YEAR_WIDE_MONTH_ABBR_WEEKDAY_DAY_INDEX  0x00010003
#define YEAR_SHORT_MONTH_WIDE_WEEKDAY_DAY_INDEX 0x00010004
#define YEAR_SHORT_MONTH_ABBR_WEEKDAY_DAY_INDEX 0x00010005
#define YEAR_ABBR_MONTH_WIDE_WEEKDAY_DAY_INDEX  0x00010006
#define YEAR_WIDE_MONTH_DAY_INDEX               0x00010007
#define WEEK_DAY_INDEX                          0x00010008
#define NUMBER_MONTH_ABBR_WEEK_DAY_INDEX        0x00010009
#define NUMBER_MONTH_DAY_INDEX                  0x0001000A
#define HOUR12_MINUTE_SECOND_INDEX              0x00020000
#define HOUR24_MINUTE_SECOND_INDEX              0x00020001
#define HOUR_MINUTE_SECOND_INDEX                0x00020002
#define FULL_INDEX                              0x00030000
#define MEDIUM_INDEX                            0x00030001
#define SHORT_INDEX                             0x00030002
#define ELAPSED_MINUTE_SECOND_INDEX             0x00040000
#define ELAPSED_MINUTE_SECOND_MILLISECOND_INDEX 0x00040001

#define PATTERN_TYPE_SHIFT 16
#define PATTERN_INDEX_MASK 0x0000ffff

namespace OHOS {
namespace I18N {
enum PatternType {
    PATTERN_TYPE_BEGIN = 0,
    TIME_PATTERN = PATTERN_TYPE_BEGIN,
    DATE_PATTERN,
    HOUR_MINUTE_SECOND_PATTERN,
    FULL_MEDIUM_SHORT_PATTERN,
    ELAPSED_PATTERN,
    PATTERN_TYPE_END,
};

std::string GetPatternFromIndex(uint32_t index, const DateTimeData * const data)
{
    uint32_t type = index >> PATTERN_TYPE_SHIFT;
    if (type > PatternType::PATTERN_TYPE_END) {
        return "";
    }
    uint32_t ind = index & PATTERN_INDEX_MASK;
    PatternType patternType = static_cast<PatternType>(type);
    switch (patternType) {
        case TIME_PATTERN: {
            return Parse(data->timePatterns, ind);
        }
        case DATE_PATTERN: {
            return Parse(data->datePatterns, ind);
        }
        case HOUR_MINUTE_SECOND_PATTERN: {
            return Parse(data->hourMinuteSecondPatterns, ind);
        }
        case FULL_MEDIUM_SHORT_PATTERN: {
            return Parse(data->fullMediumShortPatterns, ind);
        }
        default: {
            return Parse(data->elapsedPatterns, ind);
        }
    }
}

std::string GetStringFromPattern2(const AvailableDateTimeFormatPattern &requestPattern, const DateTimeData * const data)
{
    switch (requestPattern) {
        case ABBR_MONTH_WEEKDAY_DAY: {
            return GetPatternFromIndex(ABBR_MONTH_WEEKDAY_DAY_INDEX, data);
        }
        case FULL: {
            return GetPatternFromIndex(FULL_INDEX, data);
        }
        case MEDIUM: {
            return GetPatternFromIndex(MEDIUM_INDEX, data);
        }
        case SHORT: {
            return GetPatternFromIndex(SHORT_INDEX, data);
        }
        case YEAR_ABBR_MONTH_ABBR_WEEKDAY_DAY: {
            return GetPatternFromIndex(YEAR_ABBR_MONTH_ABBR_WEEKDAY_DAY_INDEX, data);
        }
        case YEAR_WIDE_MONTH_ABBR_WEEKDAY_DAY: {
            return GetPatternFromIndex(YEAR_WIDE_MONTH_ABBR_WEEKDAY_DAY_INDEX, data);
        }
        case YEAR_SHORT_MONTH_WIDE_WEEKDAY_DAY: {
            return GetPatternFromIndex(YEAR_SHORT_MONTH_WIDE_WEEKDAY_DAY_INDEX, data);
        }
        case YEAR_SHORT_MONTH_ABBR_WEEKDAY_DAY: {
            return GetPatternFromIndex(YEAR_SHORT_MONTH_ABBR_WEEKDAY_DAY_INDEX, data);
        }
        case YEAR_ABBR_MONTH_WIDE_WEEKDAY_DAY: {
            return GetPatternFromIndex(YEAR_ABBR_MONTH_WIDE_WEEKDAY_DAY_INDEX, data);
        }
        case YEAR_WIDE_MONTH_DAY: {
            return GetPatternFromIndex(YEAR_WIDE_MONTH_DAY_INDEX, data);
        }
        case WEEK_DAY: {
            return GetPatternFromIndex(WEEK_DAY_INDEX, data);
        }
        case NUMBER_MONTH_ABBR_WEEK_DAY: {
            return GetPatternFromIndex(NUMBER_MONTH_ABBR_WEEK_DAY_INDEX, data);
        }
        case NUMBER_MONTH_DAY: {
            return GetPatternFromIndex(NUMBER_MONTH_DAY_INDEX, data);
        }
        default: {
            return "";
        }
    }
}

std::string GetStringFromPattern(const AvailableDateTimeFormatPattern &requestPattern, const DateTimeData * const data)
{
    if (data == nullptr) {
        return "";
    }
    switch (requestPattern) {
        case HOUR12_MINUTE_SECOND: {
            return GetPatternFromIndex(HOUR12_MINUTE_SECOND_INDEX, data);
        }
        case HOUR24_MINUTE_SECOND: {
            return GetPatternFromIndex(HOUR24_MINUTE_SECOND_INDEX, data);
        }
        case HOUR_MINUTE_SECOND: {
            return GetPatternFromIndex(HOUR_MINUTE_SECOND_INDEX, data);
        }
        case ABBR_MONTH_DAY: {
            return GetPatternFromIndex(ABBR_MONTH_DAY_INDEX, data);
        }
        case HOUR12_MINUTE: {
            return GetPatternFromIndex(HOUR12_MINUTE_INDEX, data);
        }
        case HOUR24_MINUTE: {
            return GetPatternFromIndex(HOUR24_MINUTE_INDEX, data);
        }
        case HOUR_MINUTE: {
            return GetPatternFromIndex(HOUR_MINUTE_INDEX, data);
        }
        default: {
            return GetStringFromPattern2(requestPattern, data);
        }
    }
}

std::string GetStringFromElapsedPattern(const ElapsedPatternType &type, const DateTimeData * const data)
{
    if (data == nullptr) {
        return "";
    }
    switch (type) {
        case ELAPSED_MINUTE_SECOND: {
            return GetPatternFromIndex(ELAPSED_MINUTE_SECOND_INDEX, data);
        }
        case ELAPSED_MINUTE_SECOND_MILLISECOND: {
            return GetPatternFromIndex(ELAPSED_MINUTE_SECOND_MILLISECOND_INDEX, data);
        }
        case ELAPSED_HOUR_MINUTE: {
            return GetPatternFromIndex(HOUR24_MINUTE_INDEX, data);
        }
        case ELAPSED_HOUR_MINUTE_SECOND: {
            return GetPatternFromIndex(HOUR24_MINUTE_SECOND_INDEX, data);
        }
        default: {
            return "";
        }
    }
}
} // namespace I18N
} // namespace OHOS
#endif