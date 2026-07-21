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

#include <buddhistcalendar.h>

namespace cdroid {

BuddhistCalendar::BuddhistCalendar() : GregorianCalendar() {
}

BuddhistCalendar::BuddhistCalendar(int year, int month, int date)
        : GregorianCalendar(year - 543, month, date) {
}

BuddhistCalendar::BuddhistCalendar(int year, int month, int date, int hourOfDay, int minute, int second)
        : GregorianCalendar(year - 543, month, date, hourOfDay, minute, second) {
}

// Buddhist era: BE year = Gregorian extended year - BUDDHIST_ERA_START
// (= gregorian + 543). ICU android.icu.util.BuddhistCalendar.
static const int BUDDHIST_ERA_START = -543;
static const int BUDDHIST_ERA_BE = 0; // the only allowable era value

void BuddhistCalendar::computeTime() {
    // Translate the BE year to a Gregorian extended year for the base math,
    // run the base computation, then restore. internalSet touches only fields[],
    // not stamp[], so the user-visible field is not polluted.
    int bYear = isSet(YEAR) ? internalGet(YEAR) : (1970 - BUDDHIST_ERA_START); // 2513 BE
    internalSet(YEAR, bYear + BUDDHIST_ERA_START); // -> Gregorian extended year
    GregorianCalendar::computeTime();
    internalSet(YEAR, bYear); // restore BE year
}

void BuddhistCalendar::computeFields() {
    GregorianCalendar::computeFields();
    internalSet(YEAR, internalGet(YEAR) - BUDDHIST_ERA_START); // Gregorian -> BE
    internalSet(ERA, BUDDHIST_ERA_BE);
}

} // namespace cdroid
