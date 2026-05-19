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
#ifndef __CALENDAR_H__
#define __CALENDAR_H__
#include <time.h>
#include <stdint.h>
#include <memory>
#include <vector>
namespace cdroid{

class Calendar{
public:
    virtual ~Calendar() = default;
    enum{
        ERA   = 0,
        YEAR  = 1,
        MONTH = 2,  /*0-11 JANUARY...DECEMBER*/ 
        WEEK_OF_YEAR = 3,
        WEEK_OF_MONTH= 4,
        DATE  = 5,
        DAY_OF_MONTH =5 ,/*1-31*/
        DAY_OF_YEAR  =6 ,
        DAY_OF_WEEK  =7 ,/*SUNDAY...SATURDAY*/
        DAY_OF_WEEK_IN_MONTH =8,
        AM_PM  = 9,
        HOUR   = 10,
        HOUR_OF_DAY =11,
        MINUTE =12,
        SECOND =13,
        MILLISECOND =14,
        ZONE_OFFSET =15,
        DST_OFFSET =16,
        WEEK_YEAR =17,
        FIELD_COUNT =18,

        SUNDAY  =0 ,
        MONDAY  =1 ,
        TUESDAY =2 ,
        WEDNESDAY=3,
        THURSDAY =4,
        FRIDAY  =5 ,
        SATURDAY=6 ,

        JANUARY  = 0,
        FEBRUARY = 1,
        MARCH = 2,
        APRIL = 3,
        MAY   = 4,
        JUNE  = 5,
        JULY  = 6,
        AUGUST= 7,
        SEPTEMBER = 8,
        OCTOBER   = 9,
        NOVEMBER  = 10,
        DECEMBER  = 11,
        UNDECIMBER= 12,
        AM = 0,
        PM = 1,
        ALL_STYLES = 0,
        STANDALONE_MASK =0x8000,
        SHORT= 1,
        LONG = 2,
        NARROW_FORMAT =4,
        NARROW_STANDALONE = NARROW_FORMAT | STANDALONE_MASK,
        SHORT_FORMAT = 1,
        LONG_DORMAT  = 2,
        SHORT_STANDALONE = SHORT | STANDALONE_MASK,
        LONG_STANDALONE = LONG | STANDALONE_MASK ,

        UNSET    = 0,
        COMPUTED = 1,
        MINIMUM_USER_STAMP = 2,

        ALL_FIELDS = (1 << FIELD_COUNT) - 1,

        ERA_MASK           = (1 << ERA),
        YEAR_MASK          = (1 << YEAR),
        MONTH_MASK         = (1 << MONTH),
        WEEK_OF_YEAR_MASK  = (1 << WEEK_OF_YEAR),
        WEEK_OF_MONTH_MASK = (1 << WEEK_OF_MONTH),
        DAY_OF_MONTH_MASK  = (1 << DAY_OF_MONTH),
        DATE_MASK          = DAY_OF_MONTH_MASK,
        DAY_OF_YEAR_MASK   = (1 << DAY_OF_YEAR),
        DAY_OF_WEEK_MASK   = (1 << DAY_OF_WEEK),
        DAY_OF_WEEK_IN_MONTH_MASK  = (1 << DAY_OF_WEEK_IN_MONTH),
        AM_PM_MASK         = (1 << AM_PM),
        HOUR_MASK          = (1 << HOUR),
        HOUR_OF_DAY_MASK   = (1 << HOUR_OF_DAY),
        MINUTE_MASK        = (1 << MINUTE),
        SECOND_MASK        = (1 << SECOND),
        MILLISECOND_MASK   = (1 << MILLISECOND),
        ZONE_OFFSET_MASK   = (1 << ZONE_OFFSET),
        DST_OFFSET_MASK    = (1 << DST_OFFSET),
        WEEK_YEAR_MASK     = (1 << WEEK_YEAR),
        LIMIT_MINIMUM = 0,
        LIMIT_GREATEST_MINIMUM = 1,
        LIMIT_LEAST_MAXIMUM = 2,
        LIMIT_MAXIMUM = 3,
    };
private:
    /**
     * Pseudo-time-stamps which specify when each field was set. There
     * are two special values, UNSET and COMPUTED. Values from
     * MINIMUM_USER_SET to Integer.MAX_VALUE are legal user set values.
     */
    int stamp[FIELD_COUNT];
    bool lenient = true;
    int zone = 0;/*time zone in seconds*/
    bool sharedZone = false;
    int  firstDayOfWeek = SUNDAY;
    int  minimalDaysInFirstWeek = 1;
    static int nextStamp;
    int maxFieldIndex;
    int internalSetMask;
    /* Cache to hold the firstDayOfWeek and minimalDaysInFirstWeek
     * of a Locale.*/
    //static ConcurrentMap<Locale, int[]> cachedLocaleData
    bool isValidWeekParameter(int value);
    void adjustStamp();
    void updateTime();
    void invalidateWeekFields();
    static int aggregateStamp(int stamp_a, int stamp_b);
protected:
    void computeWeekFields();
    int fields[FIELD_COUNT];
    bool bisSet[FIELD_COUNT];
    int64_t mTime;
    bool isTimeSet;
    bool areFieldsSet;
    bool areAllFieldsSet;
    bool areFieldsVirtuallySet;
    void internalSet(int field, int value);
    int internalGet(int field);
    int64_t internalGetTimeInMillis() const;
    virtual int handleGetLimit(int field, int limitType) const;
    void pinField(int field);
    Calendar& setFields(const std::vector<int>&);
    void complete();
    virtual void computeTime();
    virtual void computeFields();
    void setFieldsComputed(int fieldMask);
    void setFieldsNormalized(int fieldMask);
    bool isPartiallyNormalized()const;
    bool isFullyNormalized()const;
    void setUnnormalized();
    static bool isFieldSet(int fieldMask, int field);
    int selectFields();
    virtual int handleGetMonthLength(int extendedYear, int month) const;
    virtual int handleGetYearLength(int extendedYear) const;
public:
    Calendar();
    Calendar& set(int field, int value);
    /*set UTC time in milliseconds from epoch*/
    void setTime(int64_t millisecond);
    /*return the current time as UTC milliseconds from the epoch*/
    int64_t getTime() const;
    void setTimeZone(int zone);
    int getTimeZone()const;
    void set(int year, int month, int date);
    void set(int year, int month, int date, int hourOfDay, int minute,int second=0,int millis=0);
    void clear(int field);
    void clear();
    bool isSet(int field) const;
    bool isEquivalentTo(const Calendar&other)const;
    virtual int getActualMinimum(int field) const;
    virtual int getActualMaximum(int field) const;
    virtual int getMinimum(int field) const;
    virtual int getMaximum(int field) const;
    virtual int getGreatestMinimum(int field) const;
    virtual int getLeastMaximum(int field) const;
    virtual int getLimit(int field, int limitType) const;
    static bool isLeapYear(int year);
    static int getDaysInMonth(int year, int month);
    static int getDaysInYear(int year);
    static std::unique_ptr<Calendar> getInstance();
    static std::unique_ptr<Calendar> getInstance(int zoneOffsetSeconds);
    static std::unique_ptr<Calendar> getInstance(int64_t instantMillis);
    static std::unique_ptr<Calendar> getInstance(int64_t instantMillis, int zoneOffsetSeconds);
    virtual void add(int field, int amount);
    virtual void roll(int field, bool up);
    virtual void roll(int field, int amount);
    Calendar& setDate(int year, int month, int dayOfMonth);
    Calendar& setTimeOfDay(int hourOfDay, int minute, int second,int millis=0);
    Calendar& setWeekDate(int weekYear, int weekOfYear, int dayOfWeek);
    Calendar& setLenient(bool lenient);
    Calendar& setWeekDefinition(int firstDayOfWeek, int minimalDaysInFirstWeek);
    Calendar& setInstant(int64_t instant);
    bool isLenient()const;
    int getFirstDayOfWeek()const;
    void setFirstDayOfWeek(int);
    int getMinimalDaysInFirstWeek() const;
    void setMinimalDaysInFirstWeek(int value);
    int get(int);
    int get(int) const;
    int64_t getTimeInMillis();
    void setTimeInMillis(int64_t millis);
    bool after(const Calendar&other)const;
    bool before(const Calendar&other)const;
};

}//namespace
#endif
