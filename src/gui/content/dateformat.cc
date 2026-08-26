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
#include <content/dateformat.h>
#include <content/simpledateformat.h>
#include <content/Locale.h>
#include <cstring>
#include <stdexcept>

#ifdef ENABLE_I18N
// Locale-dependent hour cycle and hour+minute pattern pools come from the
// i18n engine; these includes stay inside the library.
#include <content/i18nbridge.h>
#include <content/i18n/data_resource.h>
#include <content/i18n/str_util.h>
#endif

namespace cdroid {

// ---- android.text.format.DateFormat statics ----

#ifdef ENABLE_I18N
// The locale's default hour cycle, straight from the DEFAULT_HOUR pool
// ('H' = 24-hour, 'h' = 12-hour).
static bool localePrefers24Hour(const Locale& locale) {
    i18n::LocaleInfo localeInfo = I18nBridge::toLocaleInfo(locale);
    i18n::DataResource resource(&localeInfo);
    if (!resource.Init()) return false;
    char* defaultHour = resource.GetString(i18n::DataResourceType::DEFAULT_HOUR);
    return defaultHour && std::strlen(defaultHour) >= 1 && defaultHour[0] == 'H';
}

// Hour+minute pattern from the i18n time-pattern pool: index 0 = 12-hour
// ("h:mm a"), index 1 = 24-hour ("H:mm"); both already localized.
static std::string localeHourMinutePattern(const Locale& locale, bool hour12) {
    i18n::LocaleInfo localeInfo = I18nBridge::toLocaleInfo(locale);
    i18n::DataResource resource(&localeInfo);
    if (!resource.Init()) return std::string();
    char* timePatterns = resource.GetString(i18n::DataResourceType::GREGORIAN_TIME_PATTERNS);
    if (timePatterns == nullptr || std::strlen(timePatterns) == 0) return std::string();
    return i18n::Parse(timePatterns, hour12 ? 0 : 1);
}
#endif

bool DateFormat::is24HourFormat(Context* context) {
    // AOSP consults Settings.System.TIME_12_24 first; CDROID has no settings
    // store, so the locale's natural hour cycle is the answer (the context's
    // resources locale once per-Context config lookups grow, default for now).
    (void)context;
#ifdef ENABLE_I18N
    return localePrefers24Hour(Locale::getDefault());
#else
    return false; // en default hour cycle is 12-hour
#endif
}

bool DateFormat::hasSeconds(const std::string& inFormat) {
    return inFormat.find('s') != std::string::npos
        || inFormat.find('S') != std::string::npos;
}

std::string DateFormat::format(const std::string& inFormat, Calendar& inCalendar) {
    SimpleDateFormat formatter(inFormat, Locale::getDefault());
    // AOSP android.text.format.DateFormat.format(CharSequence, Calendar):
    // the formatter adopts the calendar's time zone.
    formatter.setTimeZone(inCalendar.getTimeZone());
    return formatter.format(inCalendar.getTimeInMillis());
}

std::string DateFormat::getBestDateTimePattern(const Locale& locale, const std::string& skeleton) {
#ifdef ENABLE_I18N
    // DTPG is not ported; the two skeletons TextClock uses map onto the
    // i18n hour+minute pools directly (already locale-appropriate).
    if (skeleton == "hm") return localeHourMinutePattern(locale, true);
    if (skeleton == "Hm") return localeHourMinutePattern(locale, false);
#else
    (void)locale;
#endif
    // Unsupported skeletons: hand back the skeleton itself (DTPG gap).
    return skeleton;
}

} // namespace cdroid

namespace cdroid{

DateFormat::DateFormat() {
    // AOSP leaves the calendar null until the concrete formatter initializes
    // it; CDROID always formats through a calendar, so create the default
    // one up front (SimpleDateFormat keeps it unless replaced).
    calendar = Calendar::getInstance();
}

std::string DateFormat::format(int64_t dateMillis) {
    std::string toAppendTo;
    format(dateMillis, toAppendTo);
    return toAppendTo;
}

Calendar& DateFormat::getCalendar() const { return *calendar; }

void DateFormat::setCalendar(std::unique_ptr<Calendar> newCalendar) {
    if (newCalendar) calendar = std::move(newCalendar);
}

void DateFormat::setLenient(bool lenient) { calendar->setLenient(lenient); }
bool DateFormat::isLenient() const { return calendar->isLenient(); }
void DateFormat::setTimeZone(int offsetSeconds) { calendar->setTimeZone(offsetSeconds); }
int DateFormat::getTimeZone() const { return calendar->getTimeZone(); }

bool DateFormat::operator==(const DateFormat& that) const {
    return getTimeZone() == that.getTimeZone() && isLenient() == that.isLenient();
}

// AOSP private get(timeStyle, dateStyle, flags, loc): flags bit0 = time
// requested, bit1 = date requested; the other half becomes -1 and the
// requested halves must be 0..3.
static DateFormat* get(int timeStyle, int dateStyle, int flags, const Locale& loc) {
    if ((flags & 1) != 0) {
        if (timeStyle < 0 || timeStyle > 3) {
            throw std::invalid_argument("Illegal time style " + std::to_string(timeStyle));
        }
    } else {
        timeStyle = -1;
    }
    if ((flags & 2) != 0) {
        if (dateStyle < 0 || dateStyle > 3) {
            throw std::invalid_argument("Illegal date style " + std::to_string(dateStyle));
        }
    } else {
        dateStyle = -1;
    }
    return new SimpleDateFormat(timeStyle, dateStyle, loc);
}

DateFormat* DateFormat::getTimeInstance() {
    return get(DEFAULT, 0, 1, Locale::getDefault());
}

DateFormat* DateFormat::getTimeInstance(int style) {
    return get(style, 0, 1, Locale::getDefault());
}

DateFormat* DateFormat::getTimeInstance(int style, const Locale& aLocale) {
    return get(style, 0, 1, aLocale);
}

DateFormat* DateFormat::getDateInstance() {
    return get(0, DEFAULT, 2, Locale::getDefault());
}

DateFormat* DateFormat::getDateInstance(int style) {
    return get(0, style, 2, Locale::getDefault());
}

DateFormat* DateFormat::getDateInstance(int style, const Locale& aLocale) {
    return get(0, style, 2, aLocale);
}

DateFormat* DateFormat::getDateTimeInstance() {
    return get(DEFAULT, DEFAULT, 3, Locale::getDefault());
}

DateFormat* DateFormat::getDateTimeInstance(int dateStyle, int timeStyle) {
    return get(timeStyle, dateStyle, 3, Locale::getDefault());
}

DateFormat* DateFormat::getDateTimeInstance(int dateStyle, int timeStyle, const Locale& aLocale) {
    return get(timeStyle, dateStyle, 3, aLocale);
}

}//endof namespace
