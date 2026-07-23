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
#include <core/japanesecalendar.h>

namespace {
// Modern Japanese imperial eras. Era numbers match ICU/android.icu.util.
// JapaneseCalendar (MEIJI=232..REIWA=236). Listed latest-first for lookup.
struct JapaneseEra {
    int eraNumber;   // ICU value
    int startYear;   // Gregorian year the era began
    int startMonth;  // 0-based month
    int startDay;    // 1-based day
};
static const JapaneseEra JAPANESE_ERAS[] = {
    { 236 /*REIWA*/,  2019, 4,  1 }, // 2019-05-01
    { 235 /*HEISEI*/, 1989, 0,  8 }, // 1989-01-08
    { 234 /*SHOWA*/,  1926, 11, 25 },// 1926-12-25
    { 233 /*TAISHO*/, 1912, 6,  30 },// 1912-07-30
    { 232 /*MEIJI*/,  1868, 8,  8 }, // 1868-09-08
};
static const int N_ERAS = sizeof(JAPANESE_ERAS) / sizeof(JAPANESE_ERAS[0]);
static const int CURRENT_ERA = 236; // REIWA

// Latest era whose start date is <= (year, month0, day), or null if pre-Meiji.
const JapaneseEra* eraFor(int year, int month0, int day) {
    for (int i = 0; i < N_ERAS; ++i) {
        const JapaneseEra& e = JAPANESE_ERAS[i];
        if (year > e.startYear) return &e;
        if (year == e.startYear && month0 > e.startMonth) return &e;
        if (year == e.startYear && month0 == e.startMonth && day >= e.startDay) return &e;
    }
    return nullptr;
}

int eraStartYear(int eraNumber) {
    for (int i = 0; i < N_ERAS; ++i) {
        if (JAPANESE_ERAS[i].eraNumber == eraNumber) return JAPANESE_ERAS[i].startYear;
    }
    return 0;
}
} // namespace

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
    // YEAR is the year-within-era; translate to a Gregorian year for the base
    // math, run the base computation, then restore. (MONTH/DAY are Gregorian.)
    int jYear = isSet(YEAR) ? internalGet(YEAR) : 1;
    int era = isSet(ERA) ? internalGet(ERA) : CURRENT_ERA;
    int startYear = eraStartYear(era);
    int gYear = (startYear > 0) ? (jYear + startYear - 1) : jYear;
    internalSet(YEAR, gYear);
    GregorianCalendar::computeTime();
    internalSet(YEAR, jYear);
}

void JapaneseCalendar::computeFields() {
    GregorianCalendar::computeFields();
    int gYear = internalGet(YEAR);
    int month0 = internalGet(MONTH);
    int day = internalGet(DAY_OF_MONTH);
    const JapaneseEra* e = eraFor(gYear, month0, day);
    if (e) {
        internalSet(ERA, e->eraNumber);
        internalSet(YEAR, gYear - e->startYear + 1);
    } else {
        // Pre-Meiji (before 1868-09-08): no imperial era; fall back to AD.
        internalSet(ERA, 1 /*AD*/);
    }
}

} // namespace cdroid
