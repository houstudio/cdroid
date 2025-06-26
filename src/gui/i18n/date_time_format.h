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

#ifndef DATE_TIME_FORMAT_H
#define DATE_TIME_FORMAT_H

/**
* @addtogroup I18N
* @{
*
* @brief Provides functions related to internationalization (i18n), with which you can format date, time and numbers.
*
* @since 2.2
* @version 1.0
*/

/**
* @file date_time_format.h
*
* @brief Declares functions for formatting time and obtaining the name for a month or a day of the week,
    and an AM/PM marker.
*
* Example code: \n
* Creating a <b>LocaleInfo</b> instance: \n
* {@code LocaleInfo locale("zh", "Hans", "CN"); // zh indicates the language , Hans indicates
    the script, and CN indicates the country.
* Creating a <b>DateTimeFormat</b> instance: \n
* {@code DateTimeFormat formatter(AvailableDateTimeFormatPattern::HOUR_MINUTE, locale);}
* Formatting data: \n
* {@code
*     time_t time = 3600 * 3;
*     std::string zoneInfo = "+1:00"; // UTC + 1;
*     std::string out; // Save the formatting result in <b>out</b>.
*     Ii8nStatus status = Ii8nStatus::ISUCCESS;
*     formatter.Format(time, zoneInfo, out, status);}
* Output: \n
*     4:00
*
* @since 2.2
* @version 1.0
*/

#include <string>
#include "date_time_format_impl.h"
#include "types.h"
#include "locale_info.h"
#include "number_format.h"
#include "time.h"

namespace OHOS {
namespace I18N {
class DateTimeFormat {
public:
    /**
    * @brief A constructor used to create a <b>DateTimeFormat</b> instance with specified pattern and locale.
    *
    * @param requestPattern Indicates the specified pattern for formatting.
    * @param locale Indicates the specified locale.
    * @since 2.2
    * @version 1.0
    */
    DateTimeFormat(AvailableDateTimeFormatPattern requestPattern, const LocaleInfo &locale);

    /**
    * @brief A destructor used to delete the <b>DateTimeFormat</b> instance.
    *
    * @since 2.2
    * @version 1.0
    */
    ~DateTimeFormat();

    /**
    * @brief Performs an initialization to load data.
    *
    * @return Returns <b>true</b> if the initialization is successful; returns <b>false</b> otherwise.
    * @since 2.2
    * @version 1.0
    */
    bool Init();

    /**
    * @brief Formats a time value, number of seconds elapsed since the Unix epoch (00:00:00 UTC on 1 January 1970),
    *   into a string based on a pattern.
    *
    * @param cal Indicates the time value to format.
    * @param zoneInfo Indicates the time zone information in the <b>+/-ab:cd</b> pattern. <b>+</b> indicates
    *   that the time zone offset is a positive value, <b>-</b> indicates that the time zone offset is a negative value,
    *   and <b>ab:cd</b> indicates <b>hour:minute</b>.
    * @param appendTo Used to save the formatting result.
    * @param status Indicates the formatting status.
    * @since 2.2
    * @version 1.0
    */
    void Format(const time_t &cal, const std::string &zoneInfo, std::string &appendTo, I18nStatus &status);

    /**
    * @brief Applies a pattern for this formatting.
    *
    * @param pattern Indicates the pattern to apply.
    * @since 2.2
    * @version 1.0
    */
    void ApplyPattern(const AvailableDateTimeFormatPattern &pattern);

    /**
    * @brief Obtains the name for a day of the week based on the specified <b>index</b> and <b>type</b>.
    *
    * @param index Indicates the index for the name of a day of the week. The values <b>0</b> to <b>6</b>
    *   indicate <b>Sunday</b> to <b>Saturday</b>, respectively.
    * @param type Indicates the type of a day of the week, as enumerated in {@link DateTimeDataType}.
    * @return Returns the name for a day of the week.
    * @since 2.2
    * @version 1.0
    */
    std::string GetWeekName(const int32_t &index, DateTimeDataType type);

    /**
    * @brief Obtains the month name based on the specified <b>index</b> and <b>type</b>.
    *
    * @param index Indicates the index for the month name. The values <b>0</b> to <b>11</b>
    *   indicate <b>January</b> to <b>December</b>, respectively.
    * @param type Indicates the month type, as enumerated in {@link DateTimeDataType}.
    * @return Returns the month name.
    * @since 2.2
    * @version 1.0
    */
    std::string GetMonthName(const int32_t &index, DateTimeDataType type);

    /**
    * @brief Obtains the AM/PM marker based on the specified <b>index</b> and <b>type</b>.
    *
    * @param index Indicates the index for the AM/PM marker. The value <b>0</b> indicates the AM marker,
    *   and <b>1</b> indicates the PM marker.
    * @param type Indicates the AM/PM marker type, as enumerated in {@link DateTimeDataType}.
    * @return Returns the AM/PM marker.
    * @since 2.2
    * @version 1.0
    */
    std::string GetAmPmMarker(const int32_t &index,
        DateTimeDataType type = DateTimeDataType::STANDALONE_ABBR);

    /**
    * @brief Formats a time value, number of seconds elapsed since the Unix epoch (00:00:00 UTC on 1 January 1970),
    *   into a string based on 12-hour:minute:second or 12-hour:minute, without am/pm.
    *
    * @param cal Indicates the time value to format.
    * @param zoneInfo Indicates the time zone information in the <b>+/-ab:cd</b> pattern. <b>+</b> indicates
    *   that the time zone offset is a positive value, <b>-</b> indicates that the time zone offset is a negative value,
    *   and <b>ab:cd</b> indicates <b>hour:minute</b>.
    * @param appendTo Used to save the formatting result.
    * @param status Indicates the formatting status.
    * @return Returns 1 or -1 if am/pm markers should be put at the beginning or end of the returned string when we call
    *   format with HOUR12_MINUTE_SECOND, returns 0 if no am/pm markers should be added.
    */
    int8_t Get12HourTimeWithoutAmpm(const time_t &cal, const std::string &zoneInfo,
        std::string &appendTo, I18nStatus &status);

    /**
    * @brief format a elapsed duration in the pattern indicated by the type.
    *
    * @param milliseconds indicates the time to be formatted.
    * @param type pattern used to format string.
    * @param status Indicates the formatting status.
    * @return the formatted string.
    */
    std::string FormatElapsedDuration(int32_t milliseconds, ElapsedPatternType type, I18nStatus &status);

    /**
    * @brief Obtains the time separator.
    *
    * @return Returns the time separator.
    * @since 2.2
    * @version 1.0
    */
    std::string GetTimeSeparator();
private:
    DateTimeFormatImpl *impl = nullptr;
    LocaleInfo locale;
    AvailableDateTimeFormatPattern requestPattern;
};
} // namespace I18N
} // namespace OHOS
/** @} */
#endif
