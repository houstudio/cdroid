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

#ifndef DATE_TIME_DATA_H
#define DATE_TIME_DATA_H

#include <string>
#include "i18n_memory_adapter.h"
#include "locale_info.h"
#include "securec.h"
#include "str_util.h"
#include "types.h"

#define MONTH_SIZE 12
#define DAY_SIZE 7
#define AM_SIZE 2
#define SEP_HOUR_SIZE 2

namespace OHOS {
namespace I18N {
class DateTimeData {
public:
    DateTimeData(const char *amPmMarkers, const char *configs, const int size);
    ~DateTimeData();
    std::string GetMonthName(int32_t index, DateTimeDataType type = DateTimeDataType::FORMAT_ABBR);
    std::string GetDayName(int32_t index, DateTimeDataType type = DateTimeDataType::FORMAT_ABBR);
    std::string GetAmPmMarker(int32_t index, DateTimeDataType type);
    char *timePatterns = nullptr;
    char *datePatterns = nullptr;
    char *hourMinuteSecondPatterns = nullptr;
    char *fullMediumShortPatterns = nullptr;
    char *elapsedPatterns = nullptr;
    char GetTimeSeparator(void) const;
    char GetDefaultHour(void) const;
    void SetMonthNamesData(const char *formatAbbreviatedMonthNames, const char *formatWideMonthNames,
        const char *standaloneAbbreviatedMonthNames, const char *standaloneWideMonthNames);
    void SetDayNamesData(const char *formatAbbreviatedDayNames, const char *formatWideDayNames,
        const char *standaloneAbbreviatedDayNames, const char *standaloneWideDayNames);
    void SetPatternsData(const char *datePatterns, const char *timePatterns, const char *hourMinuteSecondPatterns,
        const char *fullMediumShortPatterns, const char *elapsedPatterns);
private:
    char *formatAbbreviatedMonthNames = nullptr;
    char *formatWideMonthNames = nullptr;
    char *standaloneAbbreviatedMonthNames = nullptr;
    char *standaloneWideMonthNames = nullptr;
    char *formatAbbreviatedDayNames = nullptr;
    char *formatWideDayNames = nullptr;
    char *standaloneAbbreviatedDayNames = nullptr;
    char *standaloneWideDayNames = nullptr;
    char *amPmMarkers = nullptr;
    char timeSeparator = ':';
    char defaultHour = 'H';
    const int CONFIG_MIN_SIZE = 2;
};
} // namespace I18N
} // namespace OHOS
#endif
