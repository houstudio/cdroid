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
#include <content/dateformatsymbols.h>
#include <porting/cdlog.h>
#include <gui_features.h>
#include <cstring>

#ifdef ENABLE_I18N
// The i18n engine stays behind this translation unit: nothing below leaks
// past the public header (apps include <content/dateformatsymbols.h> only).
#include <core/i18nbridge.h>
#include <i18n/data_resource.h>
#include <i18n/date_time_data.h>
#endif

namespace cdroid{

namespace {
// CLDR "narrow" is not in i18n.dat yet; approximate it with the first code
// point of the abbreviated name (matches CLDR for en "Jan"->"J", zh
// "1月"->"1"; documented approximation).
std::string firstCodePoint(const std::string& s) {
    if (s.empty()) return s;
    const unsigned char c = static_cast<unsigned char>(s[0]);
    size_t len = 1;
    if      ((c & 0xF8) == 0xF0) len = 4; // U+10000..
    else if ((c & 0xF0) == 0xE0) len = 3; // U+0800..
    else if ((c & 0xE0) == 0xC0) len = 2; // U+0080..
    return s.substr(0, std::min(len, s.size()));
}

// AOSP array layouts: months 13 (trailing UNDECIMBER slot), weekdays 8
// (leading empty slot; index = Calendar.DAY_OF_WEEK).
void padMonthArray(std::vector<std::string>& v) { if (v.size() < 13) v.resize(13); }
void padWeekdayArray(std::vector<std::string>& v) {
    v.insert(v.begin(), "");           // index 0 unused, like AOSP
    if (v.size() < 8) v.resize(8);
}

void applyLoadedLocale(DateFormatSymbols*,
        std::vector<std::string>& months, std::vector<std::string>& shortMonths,
        std::vector<std::string>& standAloneMonths, std::vector<std::string>& shortStandAloneMonths,
        std::vector<std::string>& weekdays, std::vector<std::string>& shortWeekdays,
        std::vector<std::string>& standAloneWeekdays, std::vector<std::string>& shortStandAloneWeekdays,
        std::vector<std::string>& amPm,
        std::vector<std::string>& tinyMonths, std::vector<std::string>& tinyWeekdays) {
    padMonthArray(months); padMonthArray(shortMonths);
    padMonthArray(standAloneMonths); padMonthArray(shortStandAloneMonths);
    padWeekdayArray(weekdays); padWeekdayArray(shortWeekdays);
    padWeekdayArray(standAloneWeekdays); padWeekdayArray(shortStandAloneWeekdays);
    tinyMonths = shortMonths;
    for (auto& m : tinyMonths) m = firstCodePoint(m);
    tinyWeekdays = shortWeekdays;
    for (auto& d : tinyWeekdays) d = firstCodePoint(d);
    (void)amPm;
}
} // namespace

DateFormatSymbols::DateFormatSymbols() {
    loadDefaults();
    loadFromI18n(Locale::getDefault());
}

DateFormatSymbols::DateFormatSymbols(const Locale& locale) {
    loadDefaults();
    loadFromI18n(locale);
}

void DateFormatSymbols::loadDefaults() {
    // English (root) fallback, mirroring AOSP's Locale.US symbol tables.
    eras = {"BC", "AD"};
    months = {"January", "February", "March", "April", "May", "June", "July",
              "August", "September", "October", "November", "December", ""};
    shortMonths = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul",
                   "Aug", "Sep", "Oct", "Nov", "Dec", ""};
    weekdays = {"", "Sunday", "Monday", "Tuesday", "Wednesday",
                "Thursday", "Friday", "Saturday"};
    shortWeekdays = {"", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    amPm = {"AM", "PM"};
    standAloneMonths = months;
    shortStandAloneMonths = shortMonths;
    standAloneWeekdays = weekdays;
    shortStandAloneWeekdays = shortWeekdays;
    tinyMonths = {"J","F","M","A","M","J","J","A","S","O","N","D",""};
    tinyWeekdays = {"","S","M","T","W","T","F","S"};
    tinyStandAloneMonths = tinyMonths;
    tinyStandAloneWeekdays = tinyWeekdays;
}

#ifdef ENABLE_I18N
void DateFormatSymbols::loadFromI18n(const Locale& locale) {
    // Same loading sequence as i18n's DateTimeFormatImpl::Init: DataResource
    // pulls the locale's pools out of i18n.dat, DateTimeData splits them.
    i18n::LocaleInfo localeInfo = I18nBridge::toLocaleInfo(locale);
    i18n::DataResource resource(&localeInfo);
    if (!resource.Init()) return;

    char* timeSeparator = resource.GetString(i18n::DataResourceType::TIME_SEPARATOR);
    char* defaultHour   = resource.GetString(i18n::DataResourceType::DEFAULT_HOUR);
    if (timeSeparator == nullptr || defaultHour == nullptr ||
        std::strlen(timeSeparator) < 1 || std::strlen(defaultHour) < 1) {
        return; // keep English defaults
    }
    char sepAndHour[2] = { timeSeparator[0], defaultHour[0] };

    i18n::DateTimeData data(resource.GetString(
            i18n::DataResourceType::GREGORIAN_AM_PMS), sepAndHour, 2);
    data.SetMonthNamesData(
            resource.GetString(i18n::DataResourceType::GREGORIAN_FORMAT_ABBR_MONTH),
            resource.GetString(i18n::DataResourceType::GREGORIAN_FORMAT_WIDE_MONTH),
            resource.GetString(i18n::DataResourceType::GREGORIAN_STANDALONE_ABBR_MONTH),
            resource.GetString(i18n::DataResourceType::GREGORIAN_STANDALONE_WIDE_MONTH));
    data.SetDayNamesData(
            resource.GetString(i18n::DataResourceType::GREGORIAN_FORMAT_ABBR_DAY),
            resource.GetString(i18n::DataResourceType::GREGORIAN_FORMAT_WIDE_DAY),
            resource.GetString(i18n::DataResourceType::GREGORIAN_STANDALONE_ABBR_DAY),
            resource.GetString(i18n::DataResourceType::GREGORIAN_STANDALONE_WIDE_DAY));

    // i18n GetMonthName/GetDayName index 0-11 / 0-6 (Sunday first).
    months.clear(); shortMonths.clear(); standAloneMonths.clear(); shortStandAloneMonths.clear();
    weekdays.clear(); shortWeekdays.clear(); standAloneWeekdays.clear(); shortStandAloneWeekdays.clear();
    using T = i18n::DateTimeDataType;
    for (int i = 0; i < 12; i++) {
        shortMonths.push_back(data.GetMonthName(i, T::FORMAT_ABBR));
        months.push_back(data.GetMonthName(i, T::FORMAT_WIDE));
        shortStandAloneMonths.push_back(data.GetMonthName(i, T::STANDALONE_ABBR));
        standAloneMonths.push_back(data.GetMonthName(i, T::STANDALONE_WIDE));
    }
    for (int i = 0; i < 7; i++) {
        shortWeekdays.push_back(data.GetDayName(i, T::FORMAT_ABBR));
        weekdays.push_back(data.GetDayName(i, T::FORMAT_WIDE));
        shortStandAloneWeekdays.push_back(data.GetDayName(i, T::STANDALONE_ABBR));
        standAloneWeekdays.push_back(data.GetDayName(i, T::STANDALONE_WIDE));
    }
    const std::string am = data.GetAmPmMarker(0, T::STANDALONE_ABBR);
    const std::string pm = data.GetAmPmMarker(1, T::STANDALONE_ABBR);
    applyLoadedLocale(nullptr, months, shortMonths, standAloneMonths, shortStandAloneMonths,
                      weekdays, shortWeekdays, standAloneWeekdays, shortStandAloneWeekdays,
                      amPm, tinyMonths, tinyWeekdays);
    if (!am.empty()) amPm[0] = am;
    if (!pm.empty()) amPm[1] = pm;
    tinyStandAloneMonths = tinyMonths;
    tinyStandAloneWeekdays = tinyWeekdays;
    // Era names (BC/AD) have no pool in i18n.dat; the English table stands.
}
#else
void DateFormatSymbols::loadFromI18n(const Locale&) {
    // i18n engine disabled at build time: keep the English defaults.
}
#endif

const std::vector<std::string>& DateFormatSymbols::getEras()const{ return eras; }
const std::vector<std::string>& DateFormatSymbols::getMonths()const{ return months; }
const std::vector<std::string>& DateFormatSymbols::getShortMonths()const{ return shortMonths; }
const std::vector<std::string>& DateFormatSymbols::getWeekdays()const{ return weekdays; }
const std::vector<std::string>& DateFormatSymbols::getShortWeekdays()const{ return shortWeekdays; }
const std::vector<std::string>& DateFormatSymbols::getAmPmStrings()const{ return amPm; }
const std::vector<std::string>& DateFormatSymbols::getStandAloneMonths()const{ return standAloneMonths; }
const std::vector<std::string>& DateFormatSymbols::getShortStandAloneMonths()const{ return shortStandAloneMonths; }
const std::vector<std::string>& DateFormatSymbols::getStandAloneWeekdays()const{ return standAloneWeekdays; }
const std::vector<std::string>& DateFormatSymbols::getShortStandAloneWeekdays()const{ return shortStandAloneWeekdays; }
const std::vector<std::string>& DateFormatSymbols::getTinyMonths()const{ return tinyMonths; }
const std::vector<std::string>& DateFormatSymbols::getTinyWeekdays()const{ return tinyWeekdays; }
const std::vector<std::string>& DateFormatSymbols::getTinyStandAloneMonths()const{ return tinyStandAloneMonths; }
const std::vector<std::string>& DateFormatSymbols::getTinyStandAloneWeekdays()const{ return tinyStandAloneWeekdays; }
const std::string& DateFormatSymbols::getLocalPatternChars()const{ return localPatternChars; }

void DateFormatSymbols::setEras(const std::vector<std::string>& newEras){ eras = newEras; }
void DateFormatSymbols::setMonths(const std::vector<std::string>& newMonths){ months = newMonths; }
void DateFormatSymbols::setShortMonths(const std::vector<std::string>& newShortMonths){ shortMonths = newShortMonths; }
void DateFormatSymbols::setWeekdays(const std::vector<std::string>& newWeekdays){ weekdays = newWeekdays; }
void DateFormatSymbols::setShortWeekdays(const std::vector<std::string>& newShortWeekdays){ shortWeekdays = newShortWeekdays; }
void DateFormatSymbols::setAmPmStrings(const std::vector<std::string>& newAmpm){ amPm = newAmpm; }
void DateFormatSymbols::setStandAloneMonths(const std::vector<std::string>& newMonths){ standAloneMonths = newMonths; }
void DateFormatSymbols::setShortStandAloneMonths(const std::vector<std::string>& newShortMonths){ shortStandAloneMonths = newShortMonths; }
void DateFormatSymbols::setStandAloneWeekdays(const std::vector<std::string>& newWeekdays){ standAloneWeekdays = newWeekdays; }
void DateFormatSymbols::setShortStandAloneWeekdays(const std::vector<std::string>& newShortWeekdays){ shortStandAloneWeekdays = newShortWeekdays; }
void DateFormatSymbols::setLocalPatternChars(const std::string& newLocalPatternChars){
    localPatternChars = newLocalPatternChars;
}

bool DateFormatSymbols::operator==(const DateFormatSymbols& that)const{
    return eras == that.eras && months == that.months && shortMonths == that.shortMonths
        && weekdays == that.weekdays && shortWeekdays == that.shortWeekdays
        && amPm == that.amPm && localPatternChars == that.localPatternChars;
}

}//endof namespace
