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

#ifndef DATE_TIME_FORMAT_IMPL_H
#define DATE_TIME_FORMAT_IMPL_H

#include <ctype.h>
#include <string>
#include "types.h"
#include "time.h"
#include "locale_info.h"
#include "number_format_impl.h"
#include "data_resource.h"
#include "date_time_data.h"
#include "str_util.h"

#define YEAR_START 1900
#define LENGTH_HOUR 12
#define MAX_COUNT 10
#define SECONDS_IN_HOUR 3600
#define SECONDS_IN_MINUTE 60
#define DECIMAL_COUNT 10
#define ABB_COUNT 3
#define WIDE_COUNT 4
#define SHORT_YEAR_FORMAT_COUNT 2
#define ONE_HUNDRED_YEAR 100
#define QUOTE '\''
#define AM_PM_MAX_LENGTH 2
#define CONSTANT_TIME_NUMBER 10
#define SECOND_IN_MILLIS 1000
#define MINUTE_IN_MILLIS 60000
#define HOUR_IN_MILLIS 3600000

namespace OHOS {
namespace I18N {
struct ElapsedTime {
    int32_t hours;
    int32_t minutes;
    int32_t seconds;
    int32_t milliseconds;
};
class DateTimeFormatImpl {
public:
    DateTimeFormatImpl(AvailableDateTimeFormatPattern requestpattern, const LocaleInfo &locale);
    virtual ~DateTimeFormatImpl();
    bool Init(const DataResource &resource);
    void Format(const time_t &cal, const std::string &zoneInfo, std::string &apependTo, I18nStatus &status) const;
    void ApplyPattern(const AvailableDateTimeFormatPattern &pattern);
    LocaleInfo GetLocale();
    std::string GetWeekName(const int32_t &index, DateTimeDataType type) const;
    std::string GetMonthName(const int32_t &index, DateTimeDataType type) const;
    std::string GetAmPmMarker(const int32_t &index, DateTimeDataType type = DateTimeDataType::STANDALONE_ABBR) const;
    std::string GetFPattern()
    {
        return fPattern;
    }
    void SetNumberFormatter(NumberFormatImpl *numberFormat)
    {
        this->numberFormat = numberFormat;
    }
    int8_t Get12HourTimeWithoutAmpm(const time_t &cal, const std::string &zoneInfo,
        std::string &appendTo, I18nStatus &status) const;
    std::string FormatElapsedDuration(int32_t milliseconds, ElapsedPatternType type, I18nStatus &status) const;
    std::string GetTimeSeparator();
private:
    void FreeResource();
    void Format(const struct tm &time, const std::string &pattern, std::string &appendTo, I18nStatus &status) const;
    void ZeroPadding(std::string &appendTo, uint32_t minValue, uint32_t maxValue, int32_t value) const;
    void Process(const tm &time, std::string &append, char pre, uint32_t count,  I18nStatus &status) const;
    void ProcessTime(const tm &time, std::string &append, char pre, uint32_t count, I18nStatus &status) const;
    void ProcessWeekDayYear(const tm &time, std::string &appendTo, char pre,
        uint32_t count,  I18nStatus &status) const;
    bool IsTimeChar(char ch) const;
    int32_t ParseZoneInfo(const std::string &zoneInfo) const;
    char *GetNoAmPmPattern(const std::string &patternString, int8_t &ret) const;
    void FormatElapsed(const struct ElapsedTime &time, char pre, uint32_t count, std::string &appendTo,
        I18nStatus &status) const;
    std::string FormatNumber(int32_t value) const;
    std::string FormatYear(int32_t value) const;
    std::string GetZero() const;
    uint32_t GetLength(int32_t value) const;
    std::string AddSeconds(const std::string &hmPattern) const;
    AvailableDateTimeFormatPattern requestPattern;
    DateTimeData *data = nullptr;
    NumberFormatImpl *numberFormat = nullptr;
    std::string fPattern = "";
    LocaleInfo fLocale;
};
} // namespace I18N
} // namespace OHOS
#endif
