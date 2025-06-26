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

#ifndef TYPE_H
#define TYPE_H

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
* @file Types.h
*
* @brief Declares enumerated types of time, date, and number formatting.
*
* @since 2.2
* @version 1.0
*/

#define CONST_CAST const_cast<char *>
#define CHAR_CAST reinterpret_cast<char const *>

#include <string>

namespace OHOS {
namespace I18N {
/**
* @brief Enumerates formatting statuses.
*
* @since 2.2
* @version 1.0
*/
enum I18nStatus {
    /* Success */
    ISUCCESS = 0,

    /* Error */
    IERROR
};

/**
* @brief Enumerates number formatting types.
*
* @since 2.2
* @version 1.0
*/
enum NumberFormatType {
    /* Formats a number into a decimal. */
    DECIMAL,

    /* Formats a number into a percentage. */
    PERCENT
};

/**
* @brief Enumerates date formatting patterns.
*
* @since 2.2
* @version 1.0
*/
enum AvailableDateTimeFormatPattern {
    /* Displays hour, minute, and second in 12-hour format. */
    HOUR12_MINUTE_SECOND,

    /* Displays hour, minute, and second in 24-hour format. */
    HOUR24_MINUTE_SECOND,

    /* Displays hour, minute, and second in the default time used in a country/region. */
    HOUR_MINUTE_SECOND,

    /* Displays hour and minute in 12-hour format. */
    HOUR12_MINUTE,

    /* Displays hour and minute in 24-hour format. */
    HOUR24_MINUTE,

    /* Displays hour and minute in the default time format used in a country/region. */
    HOUR_MINUTE,

    /* Displays month (abbreviated) and day of the week, and day. */
    ABBR_MONTH_WEEKDAY_DAY,

    /* Displays month (abbreviated) and day. */
    ABBR_MONTH_DAY,

    /* Display year, month, day, and day of the week, for example, Friday December 18, 2020. */
    FULL,

    /* Displays year, month, and day, for example, Dec 18, 2020 */
    MEDIUM,

    /* Displays year, month, and day in numeric pattern, for example, 12/18/2020. */
    SHORT,

    /* Displays year, month (abbreviated), day of the week (abbreviated), and day. */
    YEAR_ABBR_MONTH_ABBR_WEEKDAY_DAY,

    /* Displays year, month (wide), day of the week (abbreviated), and day. */
    YEAR_WIDE_MONTH_ABBR_WEEKDAY_DAY,

    /* Displays year, month (short), day of the week (wide), and day. */
    YEAR_SHORT_MONTH_WIDE_WEEKDAY_DAY,

    /* Displays year, month (short), day of the week (abbreviated), and day. */
    YEAR_SHORT_MONTH_ABBR_WEEKDAY_DAY,

    /* Displays year, month (abbreviated), day of the week (wide), and day. */
    YEAR_ABBR_MONTH_WIDE_WEEKDAY_DAY,

    /* Displays year, month (wide), and day. */
    YEAR_WIDE_MONTH_DAY,

    /* Display week day */
    WEEK_DAY,

    /* Display numeric month-day, and week day */
    NUMBER_MONTH_ABBR_WEEK_DAY,

    /* Display numeric month-day */
    NUMBER_MONTH_DAY
};

enum ElapsedPatternType {
    /* Minute:Second */
    ELAPSED_MINUTE_SECOND,

    /* Minute:Second:Millisecond */
    ELAPSED_MINUTE_SECOND_MILLISECOND,

    /* HOUR:MINUTE:SECOND */
    ELAPSED_HOUR_MINUTE_SECOND,

    /* HOUR:MINUTE */
    ELAPSED_HOUR_MINUTE
};

enum DateTimeDataType {
    /* Abbreviated (format style) */
    FORMAT_ABBR,

    /* Wide (format style) */
    FORMAT_WIDE,

    /* Abbreviated (stand-alone style) */
    STANDALONE_ABBR,

    /* Wide (stand-alone style) */
    STANDALONE_WIDE
};

/**
* @brief Enumerates plural rule types.
*
* @since 2.2
* @version 1.0
*/
enum PluralRuleType {
    /* Zero */
    ZERO,
    /* One */
    ONE,
    /* Two */
    TWO,
    /* Few */
    FEW,
    /* Many */
    MANY,
    /* Other */
    OTHER
};

/**
* @brief Measure Format types.
*
* @since 2.2
* @version 1.0
*/
enum MeasureFormatType {
    /* Short */
    MEASURE_SHORT = 0,
    /* Medium */
    MEASURE_MEDIUM,
    /* Long */
    MEASURE_LONG,
    /* Full */
    MEASURE_FULL,
};
} // namespace I18N
} // namespace OHOS
/** @} */
#endif
