/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <chinesecalendar.h>
#include <climits>

namespace cdroid {

static const unsigned int LUNAR_INFO[] = {
    0x04bd8,0x04ae0,0x0a570,0x054d5,0x0d260,0x0d950,0x16554,0x056a0,0x09ad0,0x055d2,
    0x04ae0,0x0a5b6,0x0a4d0,0x0d250,0x1d255,0x0b540,0x0d6a0,0x0ada2,0x095b0,0x14977,
    0x04970,0x0a4b0,0x0b4b5,0x06a50,0x06d40,0x1ab54,0x02b60,0x09570,0x052f2,0x04970,
    0x06566,0x0d4a0,0x0ea50,0x06e95,0x05ad0,0x02b60,0x186e3,0x092e0,0x1c8d7,0x0c950,
    0x0d4a0,0x1d8a6,0x0b550,0x056a0,0x1a5b4,0x025d0,0x092d0,0x0d2b2,0x0a950,0x0b557,
    0x06ca0,0x0b550,0x15355,0x04da0,0x0a5d0,0x14573,0x052d0,0x0a9a8,0x0e950,0x06aa0,
    0x0aea6,0x0ab50,0x04b60,0x0aae4,0x0a570,0x05260,0x0f263,0x0d950,0x05b57,0x056a0,
    0x096d0,0x04dd5,0x04ad0,0x0a4d0,0x0d4d4,0x0d250,0x0d558,0x0b540,0x0b5a0,0x195a6,
    0x095b0,0x049b0,0x0a974,0x0a4b0,0x0b27a,0x06a50,0x06d40,0x0af46,0x0ab60,0x09570,
    0x04af5,0x04970,0x064b0,0x074a3,0x0ea50,0x06b58,0x05ac0,0x0ab60,0x096d5,0x092e0,
    0x0c960,0x0d954,0x0d4a0,0x0da50,0x07552,0x056a0,0x0abb7,0x025d0,0x092d0,0x0cab5,
    0x0a950,0x0b4a0,0x0baa4,0x0ad50,0x055d9,0x04ba0,0x0a5b0,0x15176,0x052b0,0x0a930,
    0x07954,0x06aa0,0x0ad50,0x05b52,0x04b60,0x0a6e6,0x0a4e0,0x0d260,0x0ea65,0x0d530,
    0x05aa0,0x076a3,0x096d0,0x04bd7,0x04ad0,0x0a4d0,0x1d0b6,0x0d250,0x0d520,0x0dd45,
    0x0b5a0,0x056d0,0x055b2,0x049b0,0x0a577,0x0a4b0,0x0aa50,0x1b255,0x06d20,0x0ada0,
    0x14b63,0x09370,0x049f8,0x04970,0x064b0,0x168a6,0x0ea50,0x06b20,0x1a6c4,0x0aae0,
    0x0a2e0,0x0d2e3,0x0c960,0x0d557,0x0d4a0,0x0da50,0x05d55,0x056a0,0x0a6d0,0x055d4,
    0x052d0,0x0a9b8,0x0a950,0x0b4a0,0x0b6a6,0x0ad50,0x055a0,0x0aba4,0x0a5b0,0x052b0,
    0x0b273,0x06930,0x07337,0x06aa0,0x0ad50,0x14b55,0x04b60,0x0a570,0x054e4,0x0d160,
    0x0e968,0x0d520,0x0daa0,0x16aa6,0x056d0,0x04ae0,0x0a9d4,0x0a2d0,0x0d150,0x0f252,
    0x0d520,0x0dd45,0x0b5a0,0x056d0,0x055b2,0x049b0,0x0a577,0x0a4b0,0x0aa50,0x1b255,
    0x06d20,0x0ada0,0x14b63
};

static const int CHINESE_BASE_YEAR = 1900;
static int64_t getChineseBaseMillis() {
    struct tm base = {};
    base.tm_year = 1900 - 1900;
    base.tm_mon = 0;
    base.tm_mday = 31;
    base.tm_hour = 0;
    base.tm_min = 0;
    base.tm_sec = 0;
    return static_cast<int64_t>(timegm(&base)) * 1000;
}

static int64_t getChineseDayMillis() {
    return 24LL * 60 * 60 * 1000;
}

static const int LIMITS[][4] = {
    { 1, 1, 83333, 83333 }, // ERA
    { 1, 1, 60, 60 }, // YEAR
    { 0, 0, 11, 11 }, // MONTH
    { 1, 1, 50, 55 }, // WEEK_OF_YEAR
    { 0, 0, 0, 0 }, // WEEK_OF_MONTH
    { 1, 1, 29, 30 }, // DAY_OF_MONTH
    { 1, 1, 353, 385 }, // DAY_OF_YEAR
    { 0, 0, 0, 0 }, // DAY_OF_WEEK
    { -1, -1, 5, 5 }, // DAY_OF_WEEK_IN_MONTH
    { 0, 0, 0, 0 }, // AM_PM
    { 0, 0, 0, 0 }, // HOUR
    { 0, 0, 0, 0 }, // HOUR_OF_DAY
    { 0, 0, 0, 0 }, // MINUTE
    { 0, 0, 0, 0 }, // SECOND
    { 0, 0, 0, 0 }, // MILLISECOND
    { 0, 0, 0, 0 }, // ZONE_OFFSET
    { 0, 0, 0, 0 }, // DST_OFFSET
    { 1, INT_MAX, 1, INT_MAX }, // WEEK_YEAR
};

int ChineseCalendar::handleGetLimit(int field, int limitType) const {
    if (field < 0 || field >= static_cast<int>(sizeof(LIMITS) / sizeof(LIMITS[0]))) {
        return 0;
    }
    if (limitType < LIMIT_MINIMUM || limitType > LIMIT_MAXIMUM) {
        return 0;
    }
    return LIMITS[field][limitType];
}

ChineseCalendar::ChineseCalendar() : GregorianCalendar() {
    mLeapMonth = false;
}

ChineseCalendar::ChineseCalendar(int lunarYear, int lunarMonth, int lunarDay, bool leapMonth)
        : GregorianCalendar() {
    set(YEAR, lunarYear);
    set(MONTH, lunarMonth);
    set(DAY_OF_MONTH, lunarDay);
    mLeapMonth = leapMonth;
}

void ChineseCalendar::setLeapMonth(bool leapMonth) {
    mLeapMonth = leapMonth;
}

bool ChineseCalendar::isLeapMonth() const {
    return mLeapMonth;
}

int ChineseCalendar::getChineseYear() const {
    return get(YEAR);
}

int ChineseCalendar::getChineseMonth() const {
    return get(MONTH);
}

int ChineseCalendar::getChineseDayOfMonth() const {
    return get(DAY_OF_MONTH);
}

int ChineseCalendar::getLunarLeapMonth(int year) {
    if (year < CHINESE_BASE_YEAR) {
        return 0;
    }
    int index = year - CHINESE_BASE_YEAR;
    if (index >= static_cast<int>(sizeof(LUNAR_INFO) / sizeof(LUNAR_INFO[0]))) {
        return 0;
    }
    return LUNAR_INFO[index] & 0xF;
}

int ChineseCalendar::getLunarMonthDays(int year, int lunarMonth, bool leap) {
    int index = year - CHINESE_BASE_YEAR;
    if (index < 0 || index >= static_cast<int>(sizeof(LUNAR_INFO) / sizeof(LUNAR_INFO[0]))) {
        return 29;
    }
    unsigned int info = LUNAR_INFO[index];
    int leapMonth = getLunarLeapMonth(year);
    if (leap) {
        if (leapMonth != lunarMonth) {
            return 0;
        }
        return 29 + ((info >> 16) & 1);
    }
    int bit = (info >> (16 - lunarMonth)) & 1;
    return 29 + bit;
}

int ChineseCalendar::getLunarYearDays(int year) {
    int index = year - CHINESE_BASE_YEAR;
    if (index < 0 || index >= static_cast<int>(sizeof(LUNAR_INFO) / sizeof(LUNAR_INFO[0]))) {
        return 365;
    }
    unsigned int info = LUNAR_INFO[index];
    int days = 348;
    for (int i = 0; i < 12; ++i) {
        if ((info >> (12 - i)) & 1) {
            days += 1;
        }
    }
    int leapMonth = getLunarLeapMonth(year);
    if (leapMonth != 0) {
        days += 29 + ((info >> 16) & 1);
    }
    return days;
}

int64_t ChineseCalendar::chineseToSolarMillis(int year, int lunarMonth, int lunarDay, bool leapMonth) const {
    if (year < CHINESE_BASE_YEAR || year >= CHINESE_BASE_YEAR + static_cast<int>(sizeof(LUNAR_INFO) / sizeof(LUNAR_INFO[0]))) {
        return getChineseBaseMillis();
    }
    int64_t offset = 0;
    for (int y = CHINESE_BASE_YEAR; y < year; ++y) {
        offset += static_cast<int64_t>(getLunarYearDays(y)) * getChineseDayMillis();
    }
    int leap = getLunarLeapMonth(year);
    for (int m = 1; m < lunarMonth; ++m) {
        offset += static_cast<int64_t>(getLunarMonthDays(year, m, false)) * getChineseDayMillis();
    }
    if (leap != 0 && lunarMonth > leap) {
        offset += static_cast<int64_t>(getLunarMonthDays(year, leap, true)) * getChineseDayMillis();
    }
    if (leapMonth && leap == lunarMonth) {
        offset += static_cast<int64_t>(getLunarMonthDays(year, lunarMonth, false)) * getChineseDayMillis();
    }
    offset += static_cast<int64_t>(lunarDay - 1) * getChineseDayMillis();
    return getChineseBaseMillis() + offset;
}

void ChineseCalendar::solarToChinese(int64_t utcMillis, int& year, int& month, int& day, bool& leapMonth) const {
    int64_t base = getChineseBaseMillis();
    int64_t dayMillis = getChineseDayMillis();
    int64_t diff = (utcMillis - base) / dayMillis;
    if (utcMillis < base) {
        diff = 0;
    }
    year = CHINESE_BASE_YEAR;
    while (year < CHINESE_BASE_YEAR + static_cast<int>(sizeof(LUNAR_INFO) / sizeof(LUNAR_INFO[0]))) {
        int yearDays = getLunarYearDays(year);
        if (diff < yearDays) {
            break;
        }
        diff -= yearDays;
        ++year;
    }
    int leap = getLunarLeapMonth(year);
    month = 1;
    leapMonth = false;
    while (month <= 12) {
        int monthDays = getLunarMonthDays(year, month, false);
        if (diff < monthDays) {
            break;
        }
        diff -= monthDays;
        if (leap != 0 && month == leap) {
            int leapDays = getLunarMonthDays(year, month, true);
            if (diff < leapDays) {
                leapMonth = true;
                break;
            }
            diff -= leapDays;
        }
        ++month;
    }
    day = static_cast<int>(diff + 1);
    month -= 1;
    if (month < 0) {
        month = 0;
    }
}

void ChineseCalendar::computeTime() {
    int year = internalGet(YEAR);
    int month = internalGet(MONTH);
    int day = internalGet(DAY_OF_MONTH);
    if (month < 0 || month > DECEMBER || day < 1) {
        GregorianCalendar::computeTime();
        return;
    }

    int hourOfDay = isSet(HOUR_OF_DAY) ? internalGet(HOUR_OF_DAY) : 0;
    if (!isSet(HOUR_OF_DAY) && isSet(HOUR)) {
        hourOfDay = (internalGet(AM_PM) == PM ? 12 : 0) + internalGet(HOUR);
    }
    int minute = isSet(MINUTE) ? internalGet(MINUTE) : 0;
    int second = isSet(SECOND) ? internalGet(SECOND) : 0;
    int millis = isSet(MILLISECOND) ? internalGet(MILLISECOND) : 0;

    int64_t solarMillis = chineseToSolarMillis(year, month + 1, day, mLeapMonth);
    mTime = solarMillis + hourOfDay * 3600000LL + minute * 60000LL + second * 1000LL + millis;
    isTimeSet = true;
}

void ChineseCalendar::computeFields() {
    int64_t utcMillis = getTime();
    int lunarYear;
    int lunarMonth;
    int lunarDay;
    bool leap = false;
    solarToChinese(utcMillis, lunarYear, lunarMonth, lunarDay, leap);

    internalSet(YEAR, lunarYear);
    internalSet(MONTH, lunarMonth);
    internalSet(DATE, lunarDay);
    internalSet(DAY_OF_MONTH, lunarDay);

    int dayOfYear = lunarDay;
    int leapMonth = getLunarLeapMonth(lunarYear);
    int monthNumber = lunarMonth + 1;
    for (int m = 1; m < monthNumber; ++m) {
        dayOfYear += getLunarMonthDays(lunarYear, m, false);
        if (leapMonth != 0 && m == leapMonth) {
            dayOfYear += getLunarMonthDays(lunarYear, m, true);
        }
    }
    if (leap && leapMonth == monthNumber) {
        dayOfYear += getLunarMonthDays(lunarYear, monthNumber, false);
    }
    internalSet(DAY_OF_YEAR, dayOfYear);

    int64_t seconds = utcMillis / 1000LL;
    int millisecond = static_cast<int>(utcMillis % 1000LL);
    if (millisecond < 0) {
        millisecond += 1000LL;
        seconds -= 1;
    }
    int64_t localSeconds = seconds + getTimeZone();
    time_t localTime = static_cast<time_t>(localSeconds);
    struct tm tn;
    gmtime_r(&localTime, &tn);

    internalSet(DAY_OF_WEEK, tn.tm_wday);
    internalSet(HOUR_OF_DAY, tn.tm_hour);
    internalSet(AM_PM, tn.tm_hour / 12);
    internalSet(HOUR, tn.tm_hour % 12);
    internalSet(MINUTE, tn.tm_min);
    internalSet(SECOND, tn.tm_sec);
    internalSet(MILLISECOND, millisecond);
    internalSet(ZONE_OFFSET, getTimeZone());
    internalSet(DST_OFFSET, 0);
    mLeapMonth = leap;
    computeWeekFields();
    setFieldsComputed(ALL_FIELDS);
}

} // namespace cdroid
