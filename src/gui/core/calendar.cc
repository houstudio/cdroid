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
#include <calendar.h>
#include <stdarg.h>
#include <climits>
#include <algorithm>
#include <assert.h>
#include <cdtypes.h>
#include <cdlog.h>


#define INTERNALLY_SET -1
#define ONE_SECOND 1000
#define ONE_MINUTE (60*ONE_SECOND)
#define ONE_HOUR   (60*ONE_MINUTE)
#define ONE_DAY    (24*ONE_HOUR)
#define ONE_WEEK   (7*ONE_DAY)
#define JAN_1_1_JULIAN_DAY 1721426
#define EPOCH_JULIAN_DAY   2440588
#define STAMP_MAX  10000
namespace cdroid{
int Calendar::nextStamp=0;

Calendar::Calendar(){
    zone  = 0;
    mTime = 0;
    isTimeSet = false;
    areFieldsSet = false;
    areAllFieldsSet =false;
    clear();
    firstDayOfWeek=SUNDAY;
    setTime(time(nullptr));
}

void  Calendar::internalSet(int field, int value){
    
    fields[field] = value;
}

int Calendar::internalGet(int field){
    return fields[field];
}

void Calendar::updateTime() {
    computeTime();
    // The areFieldsSet and areAllFieldsSet values are no longer
    // controlled here (as of 1.5).
    isTimeSet = true;
}

Calendar& Calendar::set(int field, int value){
    if (areFieldsSet && !areAllFieldsSet) {
         computeFields();
    }
    internalSet(field,value);
    isTimeSet = false;
    areFieldsSet =false;
    bisSet[field] = true;
    stamp[field] = nextStamp++;
    if(nextStamp==INT_MAX)
        adjustStamp();
    return *this;
}

void Calendar::setTime(int64_t millis){
    if(mTime==millis && isTimeSet && areFieldsSet && areAllFieldsSet)
       return; 
    mTime = millis;
    isTimeSet = true;
    areFieldsSet = false;
    computeFields();
    areAllFieldsSet = areFieldsSet = true;
}

void Calendar::add(int field, int amount){
}

void Calendar::roll(int field, bool up){
}

void Calendar::roll(int field, int amount){
     while (amount > 0) {
        roll(field, true);
        amount--;
     }
     while (amount < 0) {
        roll(field, false);
        amount++;
    }
}

int64_t Calendar::getTime(){
    if (!isTimeSet) {
        updateTime();
    }
    return mTime;
}

void Calendar::setTimeZone(int zone){
    this->zone=zone;
    /* Recompute the fields from the time using the new zone.  This also
     * works if isTimeSet is false (after a call to set()).  In that case
     * the time will be computed from the fields using the new zone, then
     * the fields will get recomputed from that.  Consider the sequence of
     * calls: cal.setTimeZone(EST); cal.set(HOUR, 1); cal.setTimeZone(PST).
     * Is cal set to 1 o'clock EST or 1 o'clock PST?  Answer: PST.  More
     * generally, a call to setTimeZone() affects calls to set() BEFORE AND
     * AFTER it up to the next call to complete().*/
    areAllFieldsSet = areFieldsSet = false;
}

int Calendar::getTimeZone()const{
    return zone;
}

void Calendar::set(int year, int month, int date){
    set(YEAR, year);
    set(MONTH, month);
    set(DATE, date);    
}

void Calendar::set(int year, int month, int date, int hourOfDay, int minute,int second,int millis){
    set(YEAR, year);
    set(MONTH, month);
    set(DATE, date);
    set(HOUR_OF_DAY, hourOfDay);
    set(MINUTE, minute);
    set(SECOND, second);
    set(MILLISECOND,millis);
}

void Calendar::clear(int field){
    fields[field] = 0;
    stamp[field] = UNSET;
    bisSet[field] = false;
    areAllFieldsSet = areFieldsSet = false;
    isTimeSet = false;
}

void Calendar::clear(){
    for (int i = 0; i < FIELD_COUNT/*fields.length*/; ) {
        stamp[i] = fields[i] = 0; // UNSET == 0
        bisSet[i++] = false;
    }
    areAllFieldsSet = areFieldsSet = false;
    isTimeSet = false;
}

bool Calendar::isSet(int field){
    return stamp[field] != UNSET;
}

void Calendar::complete(){
    if (!isTimeSet) {
        updateTime();
    }
    if (!areFieldsSet || !areAllFieldsSet) {
        computeFields(); // fills in unset fields
        areAllFieldsSet = areFieldsSet = true;
    }
}


void Calendar::adjustStamp(){
    int max = MINIMUM_USER_STAMP;
    int newStamp = MINIMUM_USER_STAMP;

    for (;;) {
        int min = INT_MAX;
        for (int i = 0; i <FIELD_COUNT/*stamp.length*/; i++) {
            int v = stamp[i];
            if (v >= newStamp && min > v) min = v;
            if (max < v)  max = v;
        }
        if (max != min && min == INT_MAX) {
            break;
        }
        for (int i = 0; i < FIELD_COUNT/*stamp.length*/; i++) {
            if (stamp[i] == min) {
                stamp[i] = newStamp;
            }
        }
        newStamp++;
        if (min == max) {
            break;
        }
    }
    nextStamp = newStamp;
}

int Calendar::get(int field){
    complete();
    return fields[field];
}

Calendar& Calendar::setFields(int count,...){
    va_list ap;
    va_start(ap,count);
    for(int i=0;i<count;i+=2){
        int field = va_arg(ap,int);
        int value = va_arg(ap,int);
        set(field,value);
    }
    va_end(ap);
    return *this;
}

#if defined(_WIN32)||defined(_WIN64)||defined(_MSVC_VER)
static void gmtime_r(const time_t* timer, struct tm* result) {
    //std::lock_guard<std::mutex> lock(gmtime_mutex);
    std::tm* tmp = std::gmtime(timer);
    if (tmp) {
        *result = *tmp;
    }
}
static time_t timegm(struct tm* tm) {
    return _mkgmtime(tm);
}
#endif
void Calendar::computeFields(){
#if 1 
    struct tm tn;
    time_t tmLocal= mTime/1000+zone;
    gmtime_r(&tmLocal,&tn);
    set(tn.tm_year+1900,tn.tm_mon,tn.tm_mday,tn.tm_hour,tn.tm_min,tn.tm_sec);
    set(DAY_OF_WEEK,tn.tm_wday);
    set(DAY_OF_YEAR,tn.tm_yday);
    set(MILLISECOND,mTime%1000);
    LOGV("%d/%d/%d wday=%d",tn.tm_year+1900,tn.tm_mon+1,tn.tm_mday,tn.tm_wday); 
#else
    int offsets[2]={0,0};
    //getTimeZone().getOffset(time, false, offsets);
    long localMillis = time + offsets[0] + offsets[1];

    // Mark fields as set.  Do this before calling handleComputeFields().
    int mask = internalSetMask;
    for (int i=0; i<2/*fields.length*/; ++i) {
        if ((mask & 1) == 0) {
            stamp[i] = INTERNALLY_SET;
        } else {
            stamp[i] = UNSET;
        }
        mask >>= 1;
    }

    // We used to check for and correct extreme millis values (near
    // Long.MIN_VALUE or Long.MAX_VALUE) here.  Such values would cause
    // overflows from positive to negative (or vice versa) and had to
    // be manually tweaked.  We no longer need to do this because we
    // have limited the range of supported dates to those that have a
    // Julian day that fits into an int.  This allows us to implement a
    // JULIAN_DAY field and also removes some inelegant code. - Liu
    // 11/6/00

    long days = localMillis/ONE_DAY;//floorDivide(localMillis, ONE_DAY);
    fields[JULIAN_DAY] = (int) days + EPOCH_JULIAN_DAY;

    computeGregorianAndDOWFields(fields[JULIAN_DAY]);

    // Call framework method to have subclass compute its fields.
    // These must include, at a minimum, MONTH, DAY_OF_MONTH,
    // EXTENDED_YEAR, YEAR, DAY_OF_YEAR.  This method will call internalSet(),
    // which will update stamp[].
    handleComputeFields(fields[JULIAN_DAY]);

    // Compute week-related fields, based on the subclass-computed
    // fields computed by handleComputeFields().
    computeWeekFields();

    // Compute time-related fields.  These are indepent of the date and
    // of the subclass algorithm.  They depend only on the local zone
    // wall milliseconds in day.
    int millisInDay = (int) (localMillis - (days * ONE_DAY));
    fields[MILLISECONDS_IN_DAY] = millisInDay;
    fields[MILLISECOND] = millisInDay % 1000;
    millisInDay /= 1000;
    fields[SECOND] = millisInDay % 60;
    millisInDay /= 60;
    fields[MINUTE] = millisInDay % 60;
    millisInDay /= 60;
    fields[HOUR_OF_DAY] = millisInDay;
    fields[AM_PM] = millisInDay / 12; // Assume AM == 0
    fields[HOUR] = millisInDay % 12;
    fields[ZONE_OFFSET] = offsets[0];
    fields[DST_OFFSET] = offsets[1];
#endif
}

void Calendar::setFieldsComputed(int fieldMask){
    if (fieldMask == ALL_FIELDS) {
        for (int i = 0; i < FIELD_COUNT/*fields.length*/; i++) {
            stamp[i] = COMPUTED;
            bisSet[i] = true;
        }
        areFieldsSet = areAllFieldsSet = true;
    } else {
        for (int i = 0; i < FIELD_COUNT/*fields.length*/; i++) {
            if ((fieldMask & 1) == 1) {
                stamp[i] = COMPUTED;
                bisSet[i] = true;
            } else {
                if (areAllFieldsSet && !bisSet[i]) {
                    areAllFieldsSet = false;
                }
            }
            fieldMask >>= 1;
        }
    }
}

void Calendar::setFieldsNormalized(int fieldMask){
    if (fieldMask != ALL_FIELDS) {
        for (int i = 0; i < FIELD_COUNT/*fields.length*/; i++) {
            if ((fieldMask & 1) == 0) {
                stamp[i] = fields[i] = 0; // UNSET == 0
                bisSet[i] = false;
            }
            fieldMask >>= 1;
        }
    }

    // Some or all of the fields are in sync with the
    // milliseconds, but the stamp values are not normalized yet.
    areFieldsSet = true;
    areAllFieldsSet = false;
}

bool Calendar::isPartiallyNormalized()const{
    return areFieldsSet && !areAllFieldsSet;
}

bool Calendar::isFullyNormalized()const{
    return areFieldsSet && areAllFieldsSet;
}

void Calendar::setUnnormalized() {
    areFieldsSet = areAllFieldsSet = false;
}

bool Calendar::isFieldSet(int fieldMask, int field){
    return (fieldMask & (1 << field)) != 0;
}

int Calendar::selectFields(){
    int fieldMask = YEAR_MASK;

    if (stamp[ERA] != UNSET) {
        fieldMask |= ERA_MASK;
    }
    // Find the most recent group of fields specifying the day within
    // the year.  These may be any of the following combinations:
    //   MONTH + DAY_OF_MONTH
    //   MONTH + WEEK_OF_MONTH + DAY_OF_WEEK
    //   MONTH + DAY_OF_WEEK_IN_MONTH + DAY_OF_WEEK
    //   DAY_OF_YEAR
    //   WEEK_OF_YEAR + DAY_OF_WEEK
    // We look for the most recent of the fields in each group to determine
    // the age of the group.  For groups involving a week-related field such
    // as WEEK_OF_MONTH, DAY_OF_WEEK_IN_MONTH, or WEEK_OF_YEAR, both the
    // week-related field and the DAY_OF_WEEK must be set for the group as a
    // whole to be considered.  (See bug 4153860 - liu 7/24/98.)
    int dowStamp = stamp[DAY_OF_WEEK];
    int monthStamp = stamp[MONTH];
    int domStamp = stamp[DAY_OF_MONTH];
    int womStamp = aggregateStamp(stamp[WEEK_OF_MONTH], dowStamp);
    int dowimStamp = aggregateStamp(stamp[DAY_OF_WEEK_IN_MONTH], dowStamp);
    int doyStamp = stamp[DAY_OF_YEAR];
    int woyStamp = aggregateStamp(stamp[WEEK_OF_YEAR], dowStamp);

    int bestStamp = domStamp;
    if ( womStamp > bestStamp ) bestStamp = womStamp;
    if (dowimStamp > bestStamp) bestStamp = dowimStamp;
    if ( doyStamp > bestStamp ) bestStamp = doyStamp;
    if ( woyStamp > bestStamp ) bestStamp = woyStamp;

    /* No complete combination exists.  Look for WEEK_OF_MONTH,
     * DAY_OF_WEEK_IN_MONTH, or WEEK_OF_YEAR alone.  Treat DAY_OF_WEEK alone
     * as DAY_OF_WEEK_IN_MONTH.*/
    if (bestStamp == UNSET) {
        womStamp = stamp[WEEK_OF_MONTH];
        dowimStamp = std::max(stamp[DAY_OF_WEEK_IN_MONTH], dowStamp);
        woyStamp = stamp[WEEK_OF_YEAR];
        bestStamp = std::max(std::max(womStamp, dowimStamp), woyStamp);

        /* Treat MONTH alone or no fields at all as DAY_OF_MONTH.  This may
         * result in bestStamp = domStamp = UNSET if no fields are set,
         * which indicates DAY_OF_MONTH. */
        if (bestStamp == UNSET) {
            bestStamp = domStamp = monthStamp;
        }
    }

    if (bestStamp == domStamp ||
       (bestStamp == womStamp && stamp[WEEK_OF_MONTH] >= stamp[WEEK_OF_YEAR]) ||
       (bestStamp == dowimStamp && stamp[DAY_OF_WEEK_IN_MONTH] >= stamp[WEEK_OF_YEAR])) {
        fieldMask |= MONTH_MASK;
        if (bestStamp == domStamp) {
            fieldMask |= DAY_OF_MONTH_MASK;
        } else {
            assert (bestStamp == womStamp || bestStamp == dowimStamp);
            if (dowStamp != UNSET) {
                fieldMask |= DAY_OF_WEEK_MASK;
            }
            if (womStamp == dowimStamp) {
                // When they are equal, give the priority to
                // WEEK_OF_MONTH for compatibility.
                if (stamp[WEEK_OF_MONTH] >= stamp[DAY_OF_WEEK_IN_MONTH]) {
                    fieldMask |= WEEK_OF_MONTH_MASK;
                } else {
                    fieldMask |= DAY_OF_WEEK_IN_MONTH_MASK;
                }
            } else {
                if (bestStamp == womStamp) {
                    fieldMask |= WEEK_OF_MONTH_MASK;
                } else {
                    assert (bestStamp == dowimStamp);
                    if (stamp[DAY_OF_WEEK_IN_MONTH] != UNSET) {
                        fieldMask |= DAY_OF_WEEK_IN_MONTH_MASK;
                    }
                }
            }
        }
    } else {
        assert (bestStamp == doyStamp || bestStamp == woyStamp ||
                bestStamp == UNSET);
        if (bestStamp == doyStamp) {
            fieldMask |= DAY_OF_YEAR_MASK;
        } else {
            assert (bestStamp == woyStamp);
            if (dowStamp != UNSET) {
                fieldMask |= DAY_OF_WEEK_MASK;
            }
            fieldMask |= WEEK_OF_YEAR_MASK;
        }
    }

    // Find the best set of fields specifying the time of day.  There
    // are only two possibilities here; the HOUR_OF_DAY or the
    // AM_PM and the HOUR.
    int hourOfDayStamp = stamp[HOUR_OF_DAY];
    int hourStamp = aggregateStamp(stamp[HOUR], stamp[AM_PM]);
    bestStamp = (hourStamp > hourOfDayStamp) ? hourStamp : hourOfDayStamp;
    // if bestStamp is still UNSET, then take HOUR or AM_PM. (See 4846659)
    if (bestStamp == UNSET) {
        bestStamp = std::max(stamp[HOUR], stamp[AM_PM]);
    }
    // Hours
    if (bestStamp != UNSET) {
        if (bestStamp == hourOfDayStamp) {
            fieldMask |= HOUR_OF_DAY_MASK;
        } else {
            fieldMask |= HOUR_MASK;
            if (stamp[AM_PM] != UNSET) {
                fieldMask |= AM_PM_MASK;
            }
        }
    }
    if (stamp[MINUTE] != UNSET) fieldMask |= MINUTE_MASK;
    if (stamp[SECOND] != UNSET) fieldMask |= SECOND_MASK;
    if (stamp[MILLISECOND] != UNSET) fieldMask |= MILLISECOND_MASK;
    if (stamp[ZONE_OFFSET] >= MINIMUM_USER_STAMP)fieldMask |= ZONE_OFFSET_MASK;
    if (stamp[DST_OFFSET] >= MINIMUM_USER_STAMP) fieldMask |= DST_OFFSET_MASK;

    return fieldMask;
}

int Calendar::aggregateStamp(int stamp_a, int stamp_b){
    if (stamp_a == UNSET || stamp_b == UNSET) {
        return UNSET;
    }
    return (stamp_a > stamp_b) ? stamp_a : stamp_b;
}
/*Converts the current calendar field values in {@link #fields fields[]} to the millisecond time value*/
void Calendar::computeTime(){
    struct tm tn;
    tn.tm_year=internalGet(YEAR)-1900;
    tn.tm_mon =internalGet(MONTH);
    tn.tm_mday=internalGet(DATE);
    tn.tm_hour=internalGet(HOUR_OF_DAY);
    tn.tm_min =internalGet(MINUTE);
    tn.tm_sec =internalGet(SECOND); 
    mTime=timegm(&tn)*1000+internalGet(MILLISECOND);
    LOGV("%d/%d/%d=%ld zone=%d",tn.tm_year+1900,tn.tm_mon+1,tn.tm_mday,mTime,zone);
}

Calendar& Calendar::setDate(int year, int month, int dayOfMonth){
    return setFields(6,YEAR, year, MONTH, month, DAY_OF_MONTH, dayOfMonth);
}

Calendar& Calendar::setTimeOfDay(int hourOfDay, int minute, int second,int millis){
    return setFields(8,HOUR_OF_DAY, hourOfDay, MINUTE, minute,
                    SECOND, second, MILLISECOND, millis);
}

Calendar& Calendar::setWeekDate(int weekYear, int weekOfYear, int dayOfWeek){
    return setFields(6,WEEK_YEAR, weekYear,WEEK_OF_YEAR, weekOfYear,
                     DAY_OF_WEEK, dayOfWeek);   
}

Calendar& Calendar::setWeekDefinition(int firstDayOfWeek, int minimalDaysInFirstWeek){
    this->firstDayOfWeek = firstDayOfWeek;
    this->minimalDaysInFirstWeek = minimalDaysInFirstWeek;
    return *this;
}

bool Calendar::isValidWeekParameter(int value){
    return value > 0 && value <= 7;
}

Calendar& Calendar::setInstant(long instant){
    //this->instant = instant;
    //nextStamp = COMPUTED;
    return *this;
}

Calendar& Calendar::setLenient(bool lenient){
    this->lenient = lenient;
    return *this;
}

void Calendar::invalidateWeekFields(){
    if (stamp[WEEK_OF_MONTH] != COMPUTED &&
        stamp[WEEK_OF_YEAR] != COMPUTED) {
        return;
    }

    // We have to check the new values of these fields after changing
    // firstDayOfWeek and/or minimalDaysInFirstWeek. If the field values
    // have been changed, then set the new values. (4822110)
    Calendar cal ;//= (Calendar) clone();
    cal.setLenient(true);
    cal.clear(WEEK_OF_MONTH);
    cal.clear(WEEK_OF_YEAR);

    if (stamp[WEEK_OF_MONTH] == COMPUTED) {
        int weekOfMonth = cal.get(WEEK_OF_MONTH);
        if (fields[WEEK_OF_MONTH] != weekOfMonth) {
            fields[WEEK_OF_MONTH] = weekOfMonth;
        }
    }

    if (stamp[WEEK_OF_YEAR] == COMPUTED) {
        int weekOfYear = cal.get(WEEK_OF_YEAR);
        if (fields[WEEK_OF_YEAR] != weekOfYear) {
            fields[WEEK_OF_YEAR] = weekOfYear;
        }
    }
}

void Calendar::setFirstDayOfWeek(int value){
    if (firstDayOfWeek == value) return;
    firstDayOfWeek = value;
    invalidateWeekFields();
}

int Calendar::getFirstDayOfWeek(){
    return firstDayOfWeek;
}

int Calendar::getMinimalDaysInFirstWeek(){
    return minimalDaysInFirstWeek;
}

void Calendar::setMinimalDaysInFirstWeek(int value){
    if (minimalDaysInFirstWeek == value) {
        return;
    }
    minimalDaysInFirstWeek = value;
    invalidateWeekFields();
}

int64_t Calendar::getTimeInMillis(){
    if (!isTimeSet) {
        updateTime();
    }
    return mTime;
}

void Calendar::setTimeInMillis(int64_t millis){
    if (mTime == millis && isTimeSet && areFieldsSet && areAllFieldsSet) {
        // END Android-changed: Android doesn't have sun.util.calendar.ZoneInfo.

        return;
    }
    mTime = millis;
    isTimeSet = true;
    areFieldsSet = false;
    computeFields();
    areAllFieldsSet = areFieldsSet = true;
}

bool Calendar::after(const Calendar&other)const{
    return mTime>other.mTime;
}

bool Calendar::before(const Calendar&other)const{
    return mTime<other.mTime;
}
}//namespace
