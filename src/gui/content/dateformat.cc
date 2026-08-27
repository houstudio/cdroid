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
#include <cctype>
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

bool DateFormat::hasDesignator(const std::string& inFormat, char designator) {
    const size_t length = inFormat.length();

    bool insideQuote = false;
    for (size_t i = 0; i < length; i++) {
        const char c = inFormat[i];
        if (c == QUOTE) {
            insideQuote = !insideQuote;
        } else if (!insideQuote) {
            if (c == designator) {
                return true;
            }
        }
    }

    return false;
}

bool DateFormat::is24HourLocale(const Locale& locale) {
    // AOSP caches one (locale, result) pair behind sLocaleLock; CDROID's UI is
    // single-threaded, so the plain statics below carry the same behavior.
    static Locale cachedLocale;
    static bool cachedResult = false;
    static bool cacheValid = false;
    if (cacheValid && cachedLocale == locale) {
        return cachedResult;
    }

    const bool is24Hour = [&]() {
        // java.text.DateFormat.getTimeInstance(LONG, locale); AOSP down-casts
        // to SimpleDateFormat and inspects the pattern for 'H'.
        DateFormat* natural = getTimeInstance(LONG, locale);
        SimpleDateFormat* sdf = dynamic_cast<SimpleDateFormat*>(natural);
        bool result = false;
        if (sdf != nullptr) {
            result = hasDesignator(sdf->toPattern(), 'H');
        }
        delete natural;
        return result;
    }();

    cachedLocale = locale;
    cachedResult = is24Hour;
    cacheValid = true;
    return is24Hour;
}

std::array<char, 3> DateFormat::getDateFormatOrder(const std::string& pattern) {
    std::array<char, 3> result = { '\0', '\0', '\0' };
    int resultIndex = 0;
    bool sawDay = false;
    bool sawMonth = false;
    bool sawYear = false;

    const size_t length = pattern.length();
    for (size_t i = 0; i < length; ++i) {
        const char ch = pattern[i];
        if (ch == 'd' || ch == 'L' || ch == 'M' || ch == 'y') {
            if (ch == 'd' && !sawDay) {
                result[resultIndex++] = 'd';
                sawDay = true;
            } else if ((ch == 'L' || ch == 'M') && !sawMonth) {
                result[resultIndex++] = 'M';
                sawMonth = true;
            } else if ((ch == 'y') && !sawYear) {
                result[resultIndex++] = 'y';
                sawYear = true;
            }
        } else if (ch == 'G') {
            // Ignore the era specifier, if present.
        } else if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            throw std::invalid_argument("Bad pattern character '" + std::string(1, ch)
                    + "' in " + pattern);
        } else if (ch == '\'') {
            if (i < length - 1 && pattern[i + 1] == '\'') {
                ++i;
            } else {
                const size_t close = pattern.find('\'', i + 1);
                if (close == std::string::npos) {
                    throw std::invalid_argument("Bad quoting in " + pattern);
                }
                i = close;
                ++i;
            }
        } else {
            // Ignore spaces and punctuation.
        }
    }
    return result;
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
    // DTPG-lite: map the skeleton's field requests onto the engine pools —
    // GREGORIAN_FULL_MEDIUM_SHORT_PATTERN holds the locale-ordered combined
    // date patterns [full(with weekday), medium, short], and the hour-minute
    // pool carries the localized time pattern. ICU's real DTPG would
    // per-field best-match; this subset covers the DateUtils skeletons.
    i18n::LocaleInfo localeInfo = I18nBridge::toLocaleInfo(locale);
    i18n::DataResource resource(&localeInfo);
    if (!resource.Init()) return skeleton;
    char* fms = resource.GetString(i18n::DataResourceType::GREGORIAN_FULL_MEDIUM_SHORT_PATTERN);
    if (fms == nullptr || std::strlen(fms) == 0) return skeleton;

    // Field census of the skeleton.
    auto has = [&](char c) { return skeleton.find(c) != std::string::npos; };
    const bool wantWeekday = has('E');
    const bool wantYear    = has('y');
    const bool wantTime    = has('h') || has('H') || has('j') || has('K') || has('k');
    const bool numeric     = has('M') && skeleton.find("MMM") == std::string::npos;

    // Date part: full (weekday) / medium (y+M+d) / short (numeric), all in
    // the locale's own field order.
    std::string datePart;
    if (wantWeekday) {
        datePart = i18n::Parse(fms, 0);
    } else if (numeric) {
        datePart = i18n::Parse(fms, 2);
    } else {
        datePart = i18n::Parse(fms, 1);
        if (!wantYear) {
            // Month+day only: strip the year token and one adjacent
            // separator ("," / "年" style joins degrade to the space).
            size_t ypos = datePart.find('y');
            if (ypos != std::string::npos) {
                size_t b = ypos, e = ypos;
                while (b > 0 && !std::isalpha((unsigned char)datePart[b-1])) b--;
                while (e < datePart.size() && !std::isalpha((unsigned char)datePart[e])) e++;
                while (e < datePart.size() && std::isalpha((unsigned char)datePart[e])) e++;
                datePart = datePart.substr(0, b) + datePart.substr(e);
            }
        }
    }

    if (!wantTime) return datePart;

    // Time part: 'j' resolves by the locale's natural hour cycle; an explicit
    // h/H skeleton forces it (AOSP DateTimeFormat semantics).
    bool hour12;
    if (has('H')) hour12 = false;
    else if (has('h')) hour12 = true;
    else hour12 = !localePrefers24Hour(locale);
    const std::string timePart = localeHourMinutePattern(locale, hour12);

    // A time-only skeleton (no date fields requested at all) maps to the bare
    // time pattern, like ICU's DTPG ("hm" → "h:mm a", not a glued date-time).
    const bool wantAnyDateField = wantWeekday || wantYear || has('M') || has('d')
            || has('L') || has('E');
    if (!wantAnyDateField) return timePart;

    // ICU joins date and time through the locale's {1}/{0} glue; the engine
    // pool has no glue slot, so the common ", " join stands in (a DTPG-gap
    // simplification, fine for the subset).
    return datePart + ", " + timePart;
#else
    (void)locale;
    return skeleton;
#endif
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
