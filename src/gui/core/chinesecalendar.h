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
#ifndef __CHINESE_CALENDAR_H__
#define __CHINESE_CALENDAR_H__

#include <core/gregoriancalendar.h>

namespace cdroid {

class ChineseCalendar : public GregorianCalendar {
public:
    ChineseCalendar();
    ChineseCalendar(int lunarYear, int lunarMonth, int lunarDay, bool leapMonth = false);

    // ChineseCalendar uses Calendar fields directly, like YEAR, MONTH, DATE and IS_LEAP_MONTH.

protected:
    int handleGetLimit(int field, int limitType) const override;
    int handleGetMonthLength(int extendedYear, int month) const override;
    int handleGetYearLength(int extendedYear) const override;
    int getActualMaximum(int field) const override;
    void add(int field, int amount) override;
    void roll(int field, int amount) override;
    void computeTime() override;
    void computeFields() override;

private:
    bool mLeapMonth = false;

    int64_t chineseToSolarMillis(int year, int lunarMonth, int lunarDay, bool leapMonth) const;
    void solarToChinese(int64_t utcMillis, int& year, int& month, int& day, bool& leapMonth) const;
    static int getLunarLeapMonth(int year);
    static int getLunarMonthDays(int year, int lunarMonth, bool leap);
    static int getLunarYearDays(int year);

    static int getChineseMonthCount(int year);
    static int getLinearMonthIndex(int year, int month, bool leapMonth);
    static void decodeLinearMonthIndex(int year, int index, int& month, bool& leapMonth);
};

} // namespace cdroid

#endif // __CHINESE_CALENDAR_H__
