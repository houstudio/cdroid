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
#ifndef __CDROID_DATEFORMAT_H__
#define __CDROID_DATEFORMAT_H__

#include <string>
#include <memory>
#include <core/Locale.h>
#include <core/calendar.h>
#include <content/parseposition.h>

namespace cdroid{

/* java.text.DateFormat (android-36 port): the abstract date/time formatting
   superclass. Use the getDateInstance/getTimeInstance/getDateTimeInstance
   factories to obtain a locale-default formatter (a SimpleDateFormat), or
   construct SimpleDateFormat directly with an explicit pattern.

   AOSP inherits Format (format(Object)/parseObject reflection plumbing);
   CDROID has no Format counterpart, so this is the hierarchy root. Dates
   travel as epoch milliseconds (java.util.Date is not ported); the time
   zone is a raw UTC offset in seconds on the calendar (TimeZone not
   ported). NumberFormat injection is not supported. */
class DateFormat{
public:
    /* AOSP style constants (Calendar-independent). */
    enum Style {
        FULL = 0,
        LONG = 1,
        MEDIUM = 2,
        SHORT = 3,
        DEFAULT = MEDIUM,
    };

    /* AOSP DateFormat field constants (FieldPosition selectors; CDROID has
       no FieldPosition, they are kept for API parity). */
    enum {
        ERA_FIELD = 0,
        YEAR_FIELD = 1,
        MONTH_FIELD = 2,
        DATE_FIELD = 3,
        HOUR_OF_DAY1_FIELD = 4,
        HOUR_OF_DAY0_FIELD = 5,
        MINUTE_FIELD = 6,
        SECOND_FIELD = 7,
        MILLISECOND_FIELD = 8,
        DAY_OF_WEEK_FIELD = 9,
        DAY_OF_YEAR_FIELD = 10,
        DAY_OF_WEEK_IN_MONTH_FIELD = 11,
        WEEK_OF_YEAR_FIELD = 12,
        WEEK_OF_MONTH_FIELD = 13,
        AM_PM_FIELD = 14,
        HOUR1_FIELD = 15,
        HOUR0_FIELD = 16,
        TIMEZONE_FIELD = 17,
    };

    virtual ~DateFormat() = default;

    /* AOSP abstract format(Date, StringBuffer, FieldPosition) — FieldPosition
       is not ported; implementations append the formatted text. */
    virtual void format(int64_t dateMillis, std::string& toAppendTo) = 0;
    /* AOSP abstract parse(String, ParsePosition). */
    virtual int64_t parse(const std::string& text, ParsePosition& pos) = 0;

    /* AOSP final format(Date) -> String. */
    std::string format(int64_t dateMillis);
    /* AOSP final parse(String) — implementations report failures through
       their own exception type (ParseException for SimpleDateFormat). */
    virtual int64_t parse(const std::string& text) = 0;

    /* AOSP factories: a SimpleDateFormat carrying the locale's pattern for
       the requested styles. */
    static DateFormat* getDateInstance();
    static DateFormat* getDateInstance(int style);
    static DateFormat* getDateInstance(int style, const Locale& aLocale);
    static DateFormat* getTimeInstance();
    static DateFormat* getTimeInstance(int style);
    static DateFormat* getTimeInstance(int style, const Locale& aLocale);
    static DateFormat* getDateTimeInstance();
    static DateFormat* getDateTimeInstance(int dateStyle, int timeStyle);
    static DateFormat* getDateTimeInstance(int dateStyle, int timeStyle, const Locale& aLocale);

    /* AOSP calendar/lenient/time-zone surface. setCalendar takes ownership. */
    virtual Calendar& getCalendar() const;
    virtual void setCalendar(std::unique_ptr<Calendar> newCalendar);
    virtual void setLenient(bool lenient);
    virtual bool isLenient() const;
    virtual void setTimeZone(int offsetSeconds);
    virtual int getTimeZone() const;

    /* AOSP equals compares calendar + numberFormat; CDROID has no injected
       NumberFormat, so implementations compare the pattern too. */
    virtual bool operator==(const DateFormat& that) const;
    bool operator!=(const DateFormat& that) const { return !(*this == that); }

    DateFormat(const DateFormat&) = delete;
    DateFormat& operator=(const DateFormat&) = delete;
protected:
    DateFormat();
    /* The shared date-time state (AOSP keeps these private with accessors;
       leniency lives on the calendar, like AOSP). */
    std::unique_ptr<Calendar> calendar;
};

}//endof namespace
#endif//__CDROID_DATEFORMAT_H__
