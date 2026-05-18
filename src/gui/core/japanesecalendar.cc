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
#include <japanesecalendar.h>

namespace cdroid {

JapaneseCalendar::JapaneseCalendar() : GregorianCalendar() {
}

JapaneseCalendar::JapaneseCalendar(int year, int month, int date)
        : GregorianCalendar(year, month, date) {
}

JapaneseCalendar::JapaneseCalendar(int year, int month, int date, int hourOfDay, int minute, int second)
        : GregorianCalendar(year, month, date, hourOfDay, minute, second) {
}

void JapaneseCalendar::computeTime() {
    GregorianCalendar::computeTime();
}

void JapaneseCalendar::computeFields() {
    GregorianCalendar::computeFields();
    internalSet(ERA, 1);
}

} // namespace cdroid
