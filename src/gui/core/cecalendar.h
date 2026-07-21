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
#ifndef __CE_CALENDAR_H__
#define __CE_CALENDAR_H__
#include <core/gregoriancalendar.h>

namespace cdroid {

// Shared base for CopticCalendar and EthiopicCalendar (ICU android.icu.util.
// CECalendar). Both are 13-month calendars (12x30 days + intercalary 5/6);
// they differ only in the Julian-day epoch offset returned by getJDEpochOffset().
class CECalendar : public GregorianCalendar {
public:
    CECalendar();
    CECalendar(int year, int month, int date);
    CECalendar(int year, int month, int date, int hourOfDay, int minute, int second);

    // JD offset of this calendar's epoch from the Julian day epoch.
    // Coptic == 1825029, Ethiopic == 1723856.
    virtual int getJDEpochOffset() const = 0;

protected:
    void computeTime() override;
    void computeFields() override;
    int handleGetMonthLength(int extendedYear, int month) const override;
    int handleGetYearLength(int extendedYear) const override;
};

} // namespace cdroid

#endif // __CE_CALENDAR_H__
