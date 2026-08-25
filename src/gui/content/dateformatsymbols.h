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
#ifndef __CDROID_DATEFORMATSYMBOLS_H__
#define __CDROID_DATEFORMATSYMBOLS_H__

#include <string>
#include <vector>
#include <core/Locale.h>

namespace cdroid{

/* java.text.DateFormatSymbols (android-36 port).
   Encapsulates localizable date-time formatting data: month names, weekday
   names, era strings and AM/PM markers, in format / standalone / abbreviated
   forms. AOSP loads everything from ICU/CLDR; CDROID loads it from the
   vendored i18n engine (i18n.dat) inside the .cc — the i18n headers stay
   internal to the library and never appear in this public face.

   Array layouts match AOSP exactly:
   - months/shortMonths and their standalone variants: 13 entries, index 0-11
     = JANUARY..DECEMBER, index 12 (UNDECIMBER) always "".
   - weekdays/shortWeekdays and their standalone variants: 8 entries, index
     0 always "", index 1-7 = SUNDAY..SATURDAY (Calendar.DAY_OF_WEEK).
   - eras: 2 entries (BC, AD). amPmStrings: 2 entries (AM, PM).
   - tiny* variants: CLDR "narrow" names; i18n.dat has no narrow pool, so
     they are approximated from the short names — the first code point for
     Latin-style tables (en "Jan"->"J"), or the distinguishing part when the
     table shares an affix (zh 周一..周日 -> 一..日, 1月..12月 -> 1..12). */
class DateFormatSymbols{
public:
    // AOSP DateFormatSymbols.patternChars — the pattern-letter alphabet,
    // indexed by the PATTERN_* constants in SimpleDateFormat.
    static constexpr const char* PATTERN_CHARS = "GyMdkHmsSEDFwWahKzZYuXLcbB";

    /* AOSP DateFormatSymbols(): symbols for the default FORMAT locale. */
    DateFormatSymbols();
    /* AOSP DateFormatSymbols(Locale): symbols for the given locale. Falls
       back to English when the i18n engine has no data for the locale. */
    explicit DateFormatSymbols(const Locale& locale);

    /* AOSP getters return clones of the internal arrays; this C++ value
       type returns const refs — callers needing a mutable copy take it. */
    const std::vector<std::string>& getEras()const;
    const std::vector<std::string>& getMonths()const;
    const std::vector<std::string>& getShortMonths()const;
    const std::vector<std::string>& getWeekdays()const;
    const std::vector<std::string>& getShortWeekdays()const;
    const std::vector<std::string>& getAmPmStrings()const;
    /* android.icu.text.DateFormatSymbols.getAmpmNarrowStrings(); i18n.dat has
       no narrow AM/PM pool, so this approximates from the wide markers (first
       code point), like the tiny* tables. */
    const std::vector<std::string>& getAmpmNarrowStrings()const;
    const std::vector<std::string>& getStandAloneMonths()const;
    const std::vector<std::string>& getShortStandAloneMonths()const;
    const std::vector<std::string>& getStandAloneWeekdays()const;
    const std::vector<std::string>& getShortStandAloneWeekdays()const;
    const std::vector<std::string>& getTinyMonths()const;
    const std::vector<std::string>& getTinyWeekdays()const;
    const std::vector<std::string>& getTinyStandAloneMonths()const;
    const std::vector<std::string>& getTinyStandAloneWeekdays()const;
    const std::string& getLocalPatternChars()const;

    /* AOSP setters: replace the given table (sizes follow the layouts above;
       a wrong-size table is accepted and used as-is, like AOSP). */
    void setEras(const std::vector<std::string>& newEras);
    void setMonths(const std::vector<std::string>& newMonths);
    void setShortMonths(const std::vector<std::string>& newShortMonths);
    void setWeekdays(const std::vector<std::string>& newWeekdays);
    void setShortWeekdays(const std::vector<std::string>& newShortWeekdays);
    void setAmPmStrings(const std::vector<std::string>& newAmpm);
    void setStandAloneMonths(const std::vector<std::string>& newMonths);
    void setShortStandAloneMonths(const std::vector<std::string>& newShortMonths);
    void setStandAloneWeekdays(const std::vector<std::string>& newWeekdays);
    void setShortStandAloneWeekdays(const std::vector<std::string>& newShortWeekdays);
    void setLocalPatternChars(const std::string& newLocalPatternChars);

    /* AOSP getZoneStrings/setZoneStrings carry custom time-zone display
       names. CDROID has no zone-name table in the i18n engine, so 'z'/'Z'
       always format as GMT offsets; these remain no-ops for API parity. */
    void setZoneStrings(const std::vector<std::vector<std::string>>& zoneStrings) {(void)zoneStrings;}

    bool operator==(const DateFormatSymbols& that)const;
    bool operator!=(const DateFormatSymbols& that)const { return !(*this == that); }
private:
    void loadDefaults();
    void loadFromI18n(const Locale& locale);

    std::vector<std::string> eras;
    std::vector<std::string> months;
    std::vector<std::string> shortMonths;
    std::vector<std::string> weekdays;
    std::vector<std::string> shortWeekdays;
    std::vector<std::string> amPm;
    std::vector<std::string> amPmNarrow;
    std::vector<std::string> standAloneMonths;
    std::vector<std::string> shortStandAloneMonths;
    std::vector<std::string> standAloneWeekdays;
    std::vector<std::string> shortStandAloneWeekdays;
    std::vector<std::string> tinyMonths;
    std::vector<std::string> tinyWeekdays;
    std::vector<std::string> tinyStandAloneMonths;
    std::vector<std::string> tinyStandAloneWeekdays;
    std::string localPatternChars = PATTERN_CHARS;
};

}//endof namespace
#endif//__CDROID_DATEFORMATSYMBOLS_H__
