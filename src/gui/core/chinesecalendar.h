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

#include <gregoriancalendar.h>

namespace cdroid {

class ChineseCalendar : public GregorianCalendar {
public:
    ChineseCalendar();
    ChineseCalendar(int lunarYear, int lunarMonth, int lunarDay, bool leapMonth = false);

    void setLeapMonth(bool leapMonth);
    bool isLeapMonth() const;

    int getChineseYear() const;
    int getChineseMonth() const;
    int getChineseDayOfMonth() const;

protected:
    int handleGetLimit(int field, int limitType) const override;
    void computeTime() override;
    void computeFields() override;

private:
    bool mLeapMonth = false;

    int64_t chineseToSolarMillis(int year, int lunarMonth, int lunarDay, bool leapMonth) const;
    void solarToChinese(int64_t utcMillis, int& year, int& month, int& day, bool& leapMonth) const;
    static int getLunarLeapMonth(int year);
    static int getLunarMonthDays(int year, int lunarMonth, bool leap);
    static int getLunarYearDays(int year);
};

} // namespace cdroid

#endif // __CHINESE_CALENDAR_H__
