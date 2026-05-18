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

#include <dangicalendar.h>

namespace cdroid {

DangiCalendar::DangiCalendar() : GregorianCalendar() {
}

DangiCalendar::DangiCalendar(int year, int month, int date)
        : GregorianCalendar(year - 2333, month, date) {
}

DangiCalendar::DangiCalendar(int year, int month, int date, int hourOfDay, int minute, int second)
        : GregorianCalendar(year - 2333, month, date, hourOfDay, minute, second) {
}

void DangiCalendar::computeTime() {
    int year = internalGet(YEAR) - 2333;
    internalSet(YEAR, year);
    GregorianCalendar::computeTime();
    internalSet(YEAR, year + 2333);
}

void DangiCalendar::computeFields() {
    GregorianCalendar::computeFields();
    internalSet(YEAR, internalGet(YEAR) + 2333);
    internalSet(ERA, 1);
}

} // namespace cdroid
