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
#include <core/taiwancalendar.h>

namespace cdroid {

// Minguo (Taiwan/ROC) era: Minguo year = Gregorian - Taiwan_ERA_START
// (= gregorian - 1911; 1912 AD == Minguo 1). ICU android.icu.util.TaiwanCalendar.
static const int Taiwan_ERA_START = 1911;
static const int MINGUO = 1;        // era for dates after 1911
static const int BEFORE_MINGUO = 0; // era for 1911 and earlier, year counts back

TaiwanCalendar::TaiwanCalendar() : GregorianCalendar() {
}

TaiwanCalendar::TaiwanCalendar(int year, int month, int date)
        : GregorianCalendar(year, month, date) {
}

TaiwanCalendar::TaiwanCalendar(int year, int month, int date, int hourOfDay, int minute, int second)
        : GregorianCalendar(year, month, date, hourOfDay, minute, second) {
}

void TaiwanCalendar::computeTime() {
    // Translate the Minguo year to a Gregorian extended year for the base
    // math, run the base computation, then restore. internalSet touches only
    // fields[], not stamp[], so the user-visible field is not polluted.
    int tYear = isSet(YEAR) ? internalGet(YEAR) : (1970 - Taiwan_ERA_START); // Minguo 59
    int gYear;
    if (isSet(ERA) && internalGet(ERA) == BEFORE_MINGUO) {
        gYear = 1 - tYear + Taiwan_ERA_START;
    } else {
        gYear = tYear + Taiwan_ERA_START;
    }
    internalSet(YEAR, gYear);
    GregorianCalendar::computeTime();
    internalSet(YEAR, tYear);
}

void TaiwanCalendar::computeFields() {
    GregorianCalendar::computeFields();
    int y = internalGet(YEAR) - Taiwan_ERA_START;
    if (y > 0) {
        internalSet(ERA, MINGUO);
        internalSet(YEAR, y);
    } else {
        internalSet(ERA, BEFORE_MINGUO);
        internalSet(YEAR, 1 - y);
    }
}

} // namespace cdroid
