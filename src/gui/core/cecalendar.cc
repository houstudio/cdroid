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

#include <cecalendar.h>

namespace cdroid {

CECalendar::CECalendar() : GregorianCalendar() {
}

CECalendar::CECalendar(int year, int month, int date)
        : GregorianCalendar(year, month, date) {
}

CECalendar::CECalendar(int year, int month, int date, int hourOfDay, int minute, int second)
        : GregorianCalendar(year, month, date, hourOfDay, minute, second) {
}

void CECalendar::computeTime() {
    GregorianCalendar::computeTime();
}

void CECalendar::computeFields() {
    GregorianCalendar::computeFields();
    internalSet(ERA, 1);
}

int CECalendar::handleGetMonthLength(int extendedYear, int month) const {
    if ((month + 1) % 13 != 0) {
        return 30;
    }
    return ((extendedYear % 4) / 3) + 5;
}

int CECalendar::handleGetYearLength(int extendedYear) const {
    return 365 + ((extendedYear % 4) / 3);
}

} // namespace cdroid
