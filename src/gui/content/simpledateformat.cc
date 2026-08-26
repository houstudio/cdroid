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
#include <content/simpledateformat.h>
#include <content/parseexception.h>
#include <core/systemclock.h>
#include <porting/cdlog.h>
#include <gui_features.h>
#include <cctype>
#include <climits>
#include <cstring>
#include <stdexcept>
#include <vector>

#ifdef ENABLE_I18N
// Default locale patterns for the style constructors / DateFormat factories
// come from the i18n engine — kept inside this translation unit.
#include <content/i18nbridge.h>
#include <content/i18n/data_resource.h>
#include <content/i18n/date_time_data.h>
#include <content/i18n/str_util.h>
#endif

namespace cdroid{

namespace {

/* Pattern-letter indexes into DateFormatSymbols::PATTERN_CHARS, in AOSP
   order ("GyMdkHmsSEDFwWahKzZYuXLcbB"). */
enum {
    PATTERN_ERA = 0,                 // 'G'
    PATTERN_YEAR,                    // 'y'
    PATTERN_MONTH,                   // 'M'
    PATTERN_DAY_OF_MONTH,            // 'd'
    PATTERN_HOUR_OF_DAY1,            // 'k'
    PATTERN_HOUR_OF_DAY0,            // 'H'
    PATTERN_MINUTE,                  // 'm'
    PATTERN_SECOND,                  // 's'
    PATTERN_MILLISECOND,             // 'S'
    PATTERN_DAY_OF_WEEK,             // 'E'
    PATTERN_DAY_OF_YEAR,             // 'D'
    PATTERN_DAY_OF_WEEK_IN_MONTH,    // 'F'
    PATTERN_WEEK_OF_YEAR,            // 'w'
    PATTERN_WEEK_OF_MONTH,           // 'W'
    PATTERN_AM_PM,                   // 'a'
    PATTERN_HOUR1,                   // 'h'
    PATTERN_HOUR0,                   // 'K'
    PATTERN_ZONE_NAME,               // 'z'
    PATTERN_ZONE_VALUE,              // 'Z'
    PATTERN_WEEK_YEAR,               // 'Y'
    PATTERN_ISO_DAY_OF_WEEK,         // 'u'
    PATTERN_ISO_ZONE,                // 'X'
    PATTERN_MONTH_STANDALONE,        // 'L'
    PATTERN_STANDALONE_DAY_OF_WEEK,  // 'c'
    PATTERN_DAY_PERIOD,              // 'b'
    PATTERN_FLEXIBLE_DAY_PERIOD,     // 'B'
};

constexpr int TAG_QUOTE_ASCII_CHAR = 100;
constexpr int TAG_QUOTE_CHARS      = 101;
constexpr int MILLIS_PER_MINUTE    = 60 * 1000;
constexpr char GMT[] = "GMT";
} // namespace (pattern tags — reopened below)

/* java.text.CalendarBuilder (android-36) port: buffers field-value pairs for
   one Calendar and establishes them in pseudo-stamp order. WEEK_YEAR is the
   FIELD_COUNT-th pseudo field and ISO_DAY_OF_WEEK converts through to
   DAY_OF_WEEK. The Calendar libc exposes no week-date getters, so
   establish() always takes AOSP's "week date unsupported" branch (a lone
   'Y' degrades to YEAR). Declared at cdroid scope to match the forward
   declaration produced by the SimpleDateFormat method signatures. */
class CalendarBuilder {
public:
    static constexpr int WEEK_YEAR = Calendar::FIELD_COUNT;
    static constexpr int ISO_DAY_OF_WEEK = 1000;
    static constexpr int MAX_FIELD = Calendar::FIELD_COUNT + 1;

    CalendarBuilder() : field(MAX_FIELD * 2, 0),
            nextStamp(Calendar::MINIMUM_USER_STAMP), maxFieldIndex(-1) {}

    CalendarBuilder& set(int index, int value) {
        if (index == ISO_DAY_OF_WEEK) {
            index = Calendar::DAY_OF_WEEK;
            value = toCalendarDayOfWeek(value);
        }
        field[index] = nextStamp++;
        field[MAX_FIELD + index] = value;
        if (index > maxFieldIndex && index < Calendar::FIELD_COUNT) {
            maxFieldIndex = index;
        }
        return *this;
    }

    CalendarBuilder& addYear(int value) {
        field[MAX_FIELD + Calendar::YEAR] += value;
        field[MAX_FIELD + WEEK_YEAR] += value;
        return *this;
    }

    bool isSet(int index) const {
        if (index == ISO_DAY_OF_WEEK) index = Calendar::DAY_OF_WEEK;
        return field[index] > Calendar::UNSET;
    }

    CalendarBuilder& clear(int index) {
        if (index == ISO_DAY_OF_WEEK) index = Calendar::DAY_OF_WEEK;
        field[index] = Calendar::UNSET;
        field[MAX_FIELD + index] = 0;
        return *this;
    }

    Calendar* establish(Calendar* cal) {
        bool weekDate = isSet(WEEK_YEAR) && field[WEEK_YEAR] > field[Calendar::YEAR];
        if (weekDate) {
            // AOSP: weekDate && !cal.isWeekDateSupported() -> use YEAR instead.
            if (!isSet(Calendar::YEAR)) {
                set(Calendar::YEAR, field[MAX_FIELD + WEEK_YEAR]);
            }
            weekDate = false;
        }

        cal->clear();
        for (int stamp = Calendar::MINIMUM_USER_STAMP; stamp < nextStamp; stamp++) {
            for (int index = 0; index <= maxFieldIndex; index++) {
                if (field[index] == stamp) {
                    cal->set(index, field[MAX_FIELD + index]);
                    break;
                }
            }
        }
        return cal;
    }

    static int toISODayOfWeek(int calendarDayOfWeek) {
        return calendarDayOfWeek == Calendar::SUNDAY ? 7 : calendarDayOfWeek - 1;
    }
    static int toCalendarDayOfWeek(int isoDayOfWeek) {
        if (isoDayOfWeek <= 0 || isoDayOfWeek > 7) return isoDayOfWeek; // adjust later (lenient)
        return isoDayOfWeek == 7 ? Calendar::SUNDAY : isoDayOfWeek + 1;
    }
private:
    std::vector<int> field; // stamp[] (lower half) and value[] (upper half) combined
    int nextStamp;
    int maxFieldIndex;
};

namespace {

// Map pattern-letter index to Calendar field (AOSP PATTERN_INDEX_TO_CALENDAR_FIELD).
const int PATTERN_INDEX_TO_CALENDAR_FIELD[] = {
    Calendar::ERA,                  Calendar::YEAR,           Calendar::MONTH,
    Calendar::DATE,                 Calendar::HOUR_OF_DAY,    Calendar::HOUR_OF_DAY,
    Calendar::MINUTE,               Calendar::SECOND,         Calendar::MILLISECOND,
    Calendar::DAY_OF_WEEK,          Calendar::DAY_OF_YEAR,    Calendar::DAY_OF_WEEK_IN_MONTH,
    Calendar::WEEK_OF_YEAR,         Calendar::WEEK_OF_MONTH,  Calendar::AM_PM,
    Calendar::HOUR,                 Calendar::HOUR,           Calendar::ZONE_OFFSET,
    Calendar::ZONE_OFFSET,          CalendarBuilder::WEEK_YEAR, CalendarBuilder::ISO_DAY_OF_WEEK,
    Calendar::ZONE_OFFSET,          Calendar::MONTH,          Calendar::DAY_OF_WEEK,
    Calendar::AM_PM,                Calendar::AM_PM,
};

// java.util.TimeZone.createGmtOffsetString (libcore): "GMT+08:00" / "+0800".
std::string createGmtOffsetString(bool includeGmt, bool includeSeparator, int offsetMillis) {
    int offsetMinutes = offsetMillis / MILLIS_PER_MINUTE;
    char sign = '+';
    if (offsetMinutes < 0) {
        sign = '-';
        offsetMinutes = -offsetMinutes;
    }
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%c%02d", sign, offsetMinutes / 60);
    std::string out;
    if (includeGmt) out += GMT;
    out += buf;
    if (includeSeparator) out += ':';
    std::snprintf(buf, sizeof(buf), "%02d", offsetMinutes % 60);
    out += buf;
    return out;
}

// sun.util.calendar.CalendarUtils.sprintf0d: zero-padded decimal, given width.
void sprintf0d(std::string& buffer, int value, int width) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%0*d", width, value);
    buffer += buf;
}

bool isDigit(char c) { return c >= '0' && c <= '9'; }

// pow10 for the fractional-second widening (std::pow keeps doubles fuzzy).
long pow10Int(int n) {
    long r = 1;
    while (n-- > 0) r *= 10;
    return r;
}

// String.regionMatches(ignoreCase=true, toffset, other, ooffset, len) —
// ASCII case folding; non-ASCII bytes compare exactly (documented limit).
bool ciRegionMatches(const std::string& text, int toffset, const std::string& other,
                     int ooffset, int len) {
    if (toffset < 0 || ooffset < 0) return false;
    if ((int)text.size() - toffset < len || (int)other.size() - ooffset < len) return false;
    for (int i = 0; i < len; i++) {
        const unsigned char a = text[toffset + i];
        const unsigned char b = other[ooffset + i];
        if (std::tolower(a) != std::tolower(b)) return false;
    }
    return true;
}

} // namespace

// ---------------------------------------------------------------------------
// Construction / initialization
// ---------------------------------------------------------------------------

SimpleDateFormat::SimpleDateFormat()
    : SimpleDateFormat((int)Style::SHORT, (int)Style::SHORT, Locale::getDefault()) {
}

SimpleDateFormat::SimpleDateFormat(const std::string& pat)
    : SimpleDateFormat(pat, Locale::getDefault()) {
}

SimpleDateFormat::SimpleDateFormat(const std::string& pat, const Locale& loc)
    : pattern(pat), formatData(loc), locale(loc) {
    // initializeCalendar(): the base ctor already created the default
    // calendar; AOSP would create Calendar.getInstance(locale) here.
    calendar = Calendar::getInstance(loc);
    initialize(loc);
}

SimpleDateFormat::SimpleDateFormat(const std::string& pat, const DateFormatSymbols& symbols)
    : pattern(pat), formatData(symbols), locale(Locale::getDefault()) {
    calendar = Calendar::getInstance(locale);
    initialize(locale);
    useDateFormatSymbolsFlag = true;
}

/* AOSP private static getDateTimeFormat(timeStyle, dateStyle, locale):
   joins the locale's date and time patterns for the styles; a negative
   style keeps only the other half. The i18n pools only carry
   FULL/MEDIUM/SHORT skeletons plus the CLDR "long" date (yMMMMd) in the
   datePatterns pool, and no per-style time patterns: SHORT time is the
   locale's default hour+minute pattern, MEDIUM its +seconds variant, and
   LONG/FULL append 'z' (approximation, documented). */
static std::string getDateTimeFormat(int timeStyle, int dateStyle, const Locale& locale) {
    std::string datePattern;
    std::string timePattern;
#ifdef ENABLE_I18N
    i18n::LocaleInfo localeInfo = I18nBridge::toLocaleInfo(locale);
    i18n::DataResource resource(&localeInfo);
    if (resource.Init()) {
        char* timeSeparator = resource.GetString(i18n::DataResourceType::TIME_SEPARATOR);
        char* defaultHour   = resource.GetString(i18n::DataResourceType::DEFAULT_HOUR);
        char* hourMinuteSecondPatterns =
                resource.GetString(i18n::DataResourceType::GREGORIAN_HOUR_MINUTE_SECOND_PATTERN);
        char* fullMediumShortPatterns =
                resource.GetString(i18n::DataResourceType::GREGORIAN_FULL_MEDIUM_SHORT_PATTERN);
        char* datePatterns =
                resource.GetString(i18n::DataResourceType::GREGORIAN_DATE_PATTERNS);
        char* timePatterns =
                resource.GetString(i18n::DataResourceType::GREGORIAN_TIME_PATTERNS);
        const bool poolsUsable = timeSeparator && defaultHour && hourMinuteSecondPatterns
                && fullMediumShortPatterns && datePatterns && timePatterns
                && std::strlen(timeSeparator) >= 1 && std::strlen(defaultHour) >= 1
                && std::strlen(hourMinuteSecondPatterns) >= 1
                && std::strlen(fullMediumShortPatterns) >= 1
                && std::strlen(datePatterns) >= 1 && std::strlen(timePatterns) >= 1;
        if (poolsUsable) {
            if (dateStyle >= 0 && dateStyle <= 3) {
                if (dateStyle == DateFormat::Style::LONG) {
                    datePattern = i18n::Parse(datePatterns, 7);
                } else {
                    // fullMediumShortPatterns pool: 0=FULL, 1=MEDIUM, 2=SHORT.
                    datePattern = i18n::Parse(fullMediumShortPatterns, dateStyle);
                }
            }
            if (timeStyle >= 0 && timeStyle <= 3) {
                timePattern = (timeStyle == DateFormat::Style::MEDIUM)
                        ? i18n::Parse(hourMinuteSecondPatterns, 2)
                        : i18n::Parse(timePatterns, 2);
                if (timeStyle == DateFormat::Style::FULL || timeStyle == DateFormat::Style::LONG) timePattern += " z";
            }
        }
    }
#endif
    if (datePattern.empty() && dateStyle >= 0) {
        // English (US) fallback when the i18n engine is off or has no data.
        const char* dates[] = {"EEEE, MMMM d, y", "MMMM d, y", "MMM d, y", "M/d/yy"};
        datePattern = dates[dateStyle <= 3 ? dateStyle : 3];
    }
    if (timePattern.empty() && timeStyle >= 0) {
        const char* times[] = {"h:mm:ss a z", "h:mm:ss a z", "h:mm:ss a", "h:mm a"};
        timePattern = times[timeStyle <= 3 ? timeStyle : 3];
    }
    if (datePattern.empty()) return timePattern;
    if (timePattern.empty()) return datePattern;
    // AOSP: MessageFormat.format("{0} {1}", date, time).
    return datePattern + " " + timePattern;
}

SimpleDateFormat::SimpleDateFormat(int timeStyle, int dateStyle, const Locale& loc)
    : formatData(loc), locale(loc) {
    calendar = Calendar::getInstance(loc);
    pattern = getDateTimeFormat(timeStyle, dateStyle, loc);
    initialize(loc);
}

void SimpleDateFormat::initialize(const Locale&) {
    compiledPattern = compile(pattern);
    // AOSP also clones a cached integer NumberFormat per locale; CDROID
    // formats numbers through zeroPaddingNumber with zeroDigit ('0').
    initializeDefaultCentury();
}

void SimpleDateFormat::initializeDefaultCentury() {
    calendar->setTimeInMillis(SystemClock::currentTimeMillis());
    calendar->add(Calendar::YEAR, -80);
    parseAmbiguousDatesAsAfter(calendar->getTime());
}

void SimpleDateFormat::parseAmbiguousDatesAsAfter(int64_t startDateMillis) {
    defaultCenturyStart = startDateMillis;
    calendar->setTime(startDateMillis);
    defaultCenturyStartYear = calendar->get(Calendar::YEAR);
}

void SimpleDateFormat::set2DigitYearStart(int64_t startDateMillis) {
    parseAmbiguousDatesAsAfter(startDateMillis);
}

int64_t SimpleDateFormat::get2DigitYearStart() const {
    return defaultCenturyStart;
}

// ---------------------------------------------------------------------------
// Pattern compilation (AOSP compile/encode; the compiled form is a byte
// stream instead of a UTF-16 char[] — headers stay (tag, count) pairs with
// the 0xFF long-length escape followed by a big-endian uint32, then `count`
// literal bytes — so UTF-8 literal text needs no transcoding).
// ---------------------------------------------------------------------------

namespace {
void encodeEntry(int tag, int length, std::string& buffer) {
    if (tag == PATTERN_ISO_ZONE && length >= 4) {
        throw std::invalid_argument("invalid ISO 8601 format: length=" + std::to_string(length));
    }
    if (length < 255) {
        buffer.push_back((char)tag);
        buffer.push_back((char)length);
    } else {
        buffer.push_back((char)tag);
        buffer.push_back((char)0xff);
        buffer.push_back((char)((uint32_t)length >> 24));
        buffer.push_back((char)((uint32_t)length >> 16));
        buffer.push_back((char)((uint32_t)length >> 8));
        buffer.push_back((char)((uint32_t)length & 0xff));
    }
}
} // namespace

std::string SimpleDateFormat::compile(const std::string& pat) const {
    const int length = (int)pat.size();
    bool inQuote = false;
    std::string compiledCode;
    std::string tmpBuffer;
    int count = 0;
    int lastTag = -1;
    const std::string patternChars = DateFormatSymbols::PATTERN_CHARS;

    for (int i = 0; i < length; i++) {
        char c = pat[i];

        if (c == '\'') {
            // '' is treated as a single quote regardless of being
            // in a quoted section.
            if ((i + 1) < length) {
                c = pat[i + 1];
                if (c == '\'') {
                    i++;
                    if (count != 0) {
                        encodeEntry(lastTag, count, compiledCode);
                        lastTag = -1;
                        count = 0;
                    }
                    if (inQuote) {
                        tmpBuffer += c;
                    } else {
                        compiledCode.push_back((char)TAG_QUOTE_ASCII_CHAR);
                        compiledCode.push_back(c);
                    }
                    continue;
                }
            }
            if (!inQuote) {
                if (count != 0) {
                    encodeEntry(lastTag, count, compiledCode);
                    lastTag = -1;
                    count = 0;
                }
                tmpBuffer.clear();
                inQuote = true;
            } else {
                int len = (int)tmpBuffer.size();
                if (len == 1) {
                    char ch = tmpBuffer[0];
                    if (ch >= 0 && ch < 128) {
                        compiledCode.push_back((char)TAG_QUOTE_ASCII_CHAR);
                        compiledCode.push_back(ch);
                    } else {
                        encodeEntry(TAG_QUOTE_CHARS, 1, compiledCode);
                        compiledCode += tmpBuffer;
                    }
                } else {
                    encodeEntry(TAG_QUOTE_CHARS, len, compiledCode);
                    compiledCode += tmpBuffer;
                }
                inQuote = false;
            }
            continue;
        }
        if (inQuote) {
            tmpBuffer += c;
            continue;
        }
        if (!(c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z')) {
            if (count != 0) {
                encodeEntry(lastTag, count, compiledCode);
                lastTag = -1;
                count = 0;
            }
            if (c >= 0 && c < 128) {
                // In most cases, c would be a delimiter, such as ':'.
                compiledCode.push_back((char)TAG_QUOTE_ASCII_CHAR);
                compiledCode.push_back(c);
            } else {
                // Take any contiguous non-ASCII alphabet characters and
                // put them in a single TAG_QUOTE_CHARS.
                int j;
                for (j = i + 1; j < length; j++) {
                    char d = pat[j];
                    if (d == '\'' || (d >= 'a' && d <= 'z' || d >= 'A' && d <= 'Z')) {
                        break;
                    }
                }
                encodeEntry(TAG_QUOTE_CHARS, j - i, compiledCode);
                for (; i < j; i++) {
                    compiledCode += pat[i];
                }
                i--;
            }
            continue;
        }

        int tag = (int)patternChars.find(c);
        if (tag == -1) {
            throw std::invalid_argument(std::string("Illegal pattern character '") + c + "'");
        }
        if (lastTag == -1 || lastTag == tag) {
            lastTag = tag;
            count++;
            continue;
        }
        encodeEntry(lastTag, count, compiledCode);
        lastTag = tag;
        count = 1;
    }

    if (inQuote) {
        throw std::invalid_argument("Unterminated quote");
    }

    if (count != 0) {
        encodeEntry(lastTag, count, compiledCode);
    }

    return compiledCode;
}

// ---------------------------------------------------------------------------
// Formatting
// ---------------------------------------------------------------------------

void SimpleDateFormat::format(int64_t dateMillis, std::string& toAppendTo) {
    // Convert input date to time field list.
    calendar->setTime(dateMillis);

    const std::string& cp = compiledPattern;
    for (size_t i = 0; i < cp.size();) {
        int tag = (unsigned char)cp[i++];
        int count = (unsigned char)cp[i++];
        if (count == 255) {
            count = ((unsigned char)cp[i] << 24) | ((unsigned char)cp[i + 1] << 16)
                  | ((unsigned char)cp[i + 2] << 8) | (unsigned char)cp[i + 3];
            i += 4;
        }
        switch (tag) {
        case TAG_QUOTE_ASCII_CHAR:
            toAppendTo += (char)count;
            break;
        case TAG_QUOTE_CHARS:
            toAppendTo.append(cp, i, count);
            i += count;
            break;
        default:
            subFormat(tag, count, toAppendTo);
            break;
        }
    }
}

std::string SimpleDateFormat::format(int64_t dateMillis) {
    std::string out;
    format(dateMillis, out);
    return out;
}

std::string SimpleDateFormat::formatWeekday(int count, int value, bool standalone) const {
    // AOSP formatWeekday with useDateFormatSymbols==true (Android always
    // takes this path for GregorianCalendar, CDROID's only calendar).
    const std::vector<std::string>* weekdays = nullptr;
    if (count == 4) {
        weekdays = standalone ? &formatData.getStandAloneWeekdays() : &formatData.getWeekdays();
    } else if (count == 5) {
        weekdays = standalone ? &formatData.getTinyStandAloneWeekdays() : &formatData.getTinyWeekdays();
    } else { // count < 4, use abbreviated form if exists
        weekdays = standalone ? &formatData.getShortStandAloneWeekdays() : &formatData.getShortWeekdays();
    }
    if (value >= 0 && value < (int)weekdays->size()) {
        return (*weekdays)[value];
    }
    return std::string();
}

std::string SimpleDateFormat::formatMonth(int count, int value, int maxIntCount,
                                          std::string& buffer, bool standalone) const {
    const std::vector<std::string>* months = nullptr;
    if (count == 4) {
        months = standalone ? &formatData.getStandAloneMonths() : &formatData.getMonths();
    } else if (count == 5) {
        months = standalone ? &formatData.getTinyStandAloneMonths() : &formatData.getTinyMonths();
    } else if (count == 3) {
        months = standalone ? &formatData.getShortStandAloneMonths() : &formatData.getShortMonths();
    }
    std::string current;
    if (months != nullptr && value >= 0 && value < (int)months->size()) {
        current = (*months)[value];
    }
    if (current.empty()) {
        zeroPaddingNumber(value + 1, count, maxIntCount, buffer);
    }
    return current;
}

void SimpleDateFormat::subFormat(int patternCharIndex, int count, std::string& buffer) {
    const int maxIntCount = INT32_MAX;
    std::string current;

    int field = PATTERN_INDEX_TO_CALENDAR_FIELD[patternCharIndex];
    int value;
    if (field == CalendarBuilder::WEEK_YEAR) {
        // The Calendar libc has no getWeekYear(): take AOSP's
        // isWeekDateSupported()==false branch — use calendar year 'y'.
        patternCharIndex = PATTERN_YEAR;
        field = PATTERN_INDEX_TO_CALENDAR_FIELD[patternCharIndex];
        value = calendar->get(field);
    } else if (field == CalendarBuilder::ISO_DAY_OF_WEEK) {
        value = CalendarBuilder::toISODayOfWeek(calendar->get(Calendar::DAY_OF_WEEK));
    } else {
        value = calendar->get(field);
    }

    switch (patternCharIndex) {
    case PATTERN_ERA: { // 'G'
        const auto& eras = formatData.getEras();
        if (value >= 0 && value < (int)eras.size()) {
            current = eras[value];
        }
        break;
    }

    case PATTERN_WEEK_YEAR: // 'Y'
    case PATTERN_YEAR:      // 'y'
        // Gregorian (CDROID's only calendar): AOSP clips to 2 digits on count==2.
        if (count != 2) {
            zeroPaddingNumber(value, count, maxIntCount, buffer);
        } else {
            zeroPaddingNumber(value, 2, 2, buffer); // clip 1996 to 96
        }
        break;

    case PATTERN_MONTH:            // 'M' (context sensitive)
        current = formatMonth(count, value, maxIntCount, buffer, false);
        break;

    case PATTERN_MONTH_STANDALONE: // 'L'
        current = formatMonth(count, value, maxIntCount, buffer, true);
        break;

    case PATTERN_HOUR_OF_DAY1: // 'k' 1-based.  eg, 23:59 + 1 hour =>> 24:59
        if (value == 0) {
            zeroPaddingNumber(calendar->getMaximum(Calendar::HOUR_OF_DAY) + 1,
                              count, maxIntCount, buffer);
        } else {
            zeroPaddingNumber(value, count, maxIntCount, buffer);
        }
        break;

    case PATTERN_DAY_OF_WEEK: // 'E'
        current = formatWeekday(count, value, false);
        break;

    case PATTERN_STANDALONE_DAY_OF_WEEK: // 'c'
        current = formatWeekday(count, value, true);
        break;

    case PATTERN_AM_PM: { // 'a'
        const auto& ampm = formatData.getAmPmStrings();
        if (value >= 0 && value < (int)ampm.size()) {
            current = ampm[value];
        }
        break;
    }

    // 'b'/'B' (CLDR 32+ day periods) are ignored, like Android.
    case PATTERN_DAY_PERIOD:
    case PATTERN_FLEXIBLE_DAY_PERIOD:
        current = "";
        break;

    case PATTERN_HOUR1: { // 'h' 1-based.  eg, 11PM + 1 hour =>> 12 AM
        if (value == 0) {
            zeroPaddingNumber(calendar->getLeastMaximum(Calendar::HOUR) + 1,
                              count, maxIntCount, buffer);
        } else {
            zeroPaddingNumber(value, count, maxIntCount, buffer);
        }
        break;
    }

    case PATTERN_ZONE_NAME: { // 'z'
        // No zone display-name table in the i18n engine: always the AOSP
        // fallback — the custom GMT offset ID.
        value = calendar->get(Calendar::ZONE_OFFSET) + calendar->get(Calendar::DST_OFFSET);
        buffer += createGmtOffsetString(true, true, value);
        break;
    }

    case PATTERN_ZONE_VALUE: { // 'Z' ("-/+hhmm" form)
        value = calendar->get(Calendar::ZONE_OFFSET) + calendar->get(Calendar::DST_OFFSET);
        const bool includeSeparator = (count >= 4);
        const bool includeGmt = (count == 4);
        buffer += createGmtOffsetString(includeGmt, includeSeparator, value);
        break;
    }

    case PATTERN_ISO_ZONE: { // 'X'
        value = calendar->get(Calendar::ZONE_OFFSET) + calendar->get(Calendar::DST_OFFSET);

        if (value == 0) {
            buffer += 'Z';
            break;
        }

        value /= 60000;
        if (value >= 0) {
            buffer += '+';
        } else {
            buffer += '-';
            value = -value;
        }

        sprintf0d(buffer, value / 60, 2);
        if (count == 1) {
            break;
        }

        if (count == 3) {
            buffer += ':';
        }
        sprintf0d(buffer, value % 60, 2);
        break;
    }

    case PATTERN_MILLISECOND: // 'S'
        // Fractional seconds: convert to a fractional second [0, 1) and
        // widen out to the formatted size (".7" / ".78" / ".789" / ".7890").
        value = (int)(((double)value / 1000) * pow10Int(count));
        zeroPaddingNumber(value, count, count, buffer);
        break;

    default:
        // 'd' 'H' 'm' 's' 'D' 'F' 'w' 'W' 'K' 'u' — generic numeric fields.
        zeroPaddingNumber(value, count, maxIntCount, buffer);
        break;
    }

    // AOSP: if (current != null) buffer.append(current). Numeric paths leave
    // current empty here (their output already went through the buffer).
    buffer += current;
}

void SimpleDateFormat::zeroPaddingNumber(long value, int minDigits, int maxDigits,
                                         std::string& buffer) const {
    // Optimization for 1, 2 and 4 digit numbers (AOSP).
    if (value >= 0) {
        if (value < 100 && minDigits >= 1 && minDigits <= 2) {
            if (value < 10) {
                if (minDigits == 2) {
                    buffer += zeroDigit;
                }
                buffer += (char)(zeroDigit + value);
            } else {
                buffer += (char)(zeroDigit + value / 10);
                buffer += (char)(zeroDigit + value % 10);
            }
            return;
        } else if (value >= 1000 && value < 10000) {
            if (minDigits == 4) {
                buffer += (char)(zeroDigit + value / 1000);
                value %= 1000;
                buffer += (char)(zeroDigit + value / 100);
                value %= 100;
                buffer += (char)(zeroDigit + value / 10);
                buffer += (char)(zeroDigit + value % 10);
                return;
            }
            if (minDigits == 2 && maxDigits == 2) {
                zeroPaddingNumber(value % 100, 2, 2, buffer);
                return;
            }
        }
    }

    // General path: decimal digits with the requested minimum width; a
    // maximum width keeps the low-order digits (AOSP NumberFormat
    // maxIntegerDigits behavior for the 2-digit-year clip).
    bool negative = value < 0;
    if (negative) value = -value;
    std::string num = std::to_string(value);
    if (maxDigits < (int)num.size()) {
        num = num.substr(num.size() - maxDigits);
    }
    if (negative && num.find_first_not_of('0') != std::string::npos) {
        buffer += '-';
    }
    if ((int)num.size() < minDigits) {
        buffer.append(minDigits - (int)num.size(), zeroDigit);
    }
    buffer += num;
}

// ---------------------------------------------------------------------------
// Parsing
// ---------------------------------------------------------------------------

int64_t SimpleDateFormat::parse(const std::string& text, ParsePosition& pos) {
    // Keep this formatter's zone (set via setTimeZone) untouched by parsing.
    const int tz = getTimeZone();
    const int64_t result = parseInternal(text, pos);
    setTimeZone(tz);
    return result;
}

int64_t SimpleDateFormat::parse(const std::string& text) {
    ParsePosition pos(0);
    parse(text, pos);
    // AOSP Format.parse(String): failure means no progress was made.
    if (pos.getIndex() == 0) {
        throw ParseException("Unparseable date: \"" + text + "\"", pos.getErrorIndex());
    }
    return calendar->getTime();
}

int64_t SimpleDateFormat::parseInternal(const std::string& text, ParsePosition& pos) {
    int start = pos.getIndex();
    const int oldStart = start;
    const int textLength = (int)text.size();

    bool ambiguousYear = false;

    CalendarBuilder calb;

    const std::string& cp = compiledPattern;
    for (size_t i = 0; i < cp.size();) {
        int tag = (unsigned char)cp[i++];
        int count = (unsigned char)cp[i++];
        if (count == 255) {
            count = ((unsigned char)cp[i] << 24) | ((unsigned char)cp[i + 1] << 16)
                  | ((unsigned char)cp[i + 2] << 8) | (unsigned char)cp[i + 3];
            i += 4;
        }

        switch (tag) {
        case TAG_QUOTE_ASCII_CHAR:
            if (start >= textLength || text[start] != (char)count) {
                pos.setIndex(oldStart);
                pos.setErrorIndex(start);
                return 0;
            }
            start++;
            break;

        case TAG_QUOTE_CHARS:
            while (count-- > 0) {
                if (start >= textLength || text[start] != cp[i++]) {
                    pos.setIndex(oldStart);
                    pos.setErrorIndex(start);
                    return 0;
                }
                start++;
            }
            break;

        default: {
            // Peek the next pattern to determine if we need to obey the
            // number of pattern letters for parsing contiguous digit text
            // (e.g., "20010704" with "yyyyMMdd").
            bool obeyCount = false;
            bool useFollowingMinusSignAsDelimiter = false;

            if (i < cp.size()) {
                int nextTag = (unsigned char)cp[i];
                int nextCount = (unsigned char)cp[i + 1];
                obeyCount = shouldObeyCount(nextTag, nextCount);

                if (hasFollowingMinusSign &&
                    (nextTag == TAG_QUOTE_ASCII_CHAR || nextTag == TAG_QUOTE_CHARS)) {
                    if (nextTag != TAG_QUOTE_ASCII_CHAR) {
                        nextCount = (unsigned char)cp[i + 2];
                    }
                    if (nextCount == minusSign) {
                        useFollowingMinusSignAsDelimiter = true;
                    }
                }
            }
            start = subParse(text, start, tag, count, obeyCount, ambiguousYear, pos,
                             useFollowingMinusSignAsDelimiter, calb);
            if (start < 0) {
                pos.setIndex(oldStart);
                return 0;
            }
            break;
        }
        }
    }

    // At this point the fields of the Calendar have been set; it fills in
    // default values for missing fields when the time is computed.
    pos.setIndex(start);

    int64_t parsedDate = 0;
    try {
        calb.establish(calendar.get());
        parsedDate = calendar->getTime();
        // If the year value is ambiguous, then the two-digit year == the
        // default start year.
        if (ambiguousYear) {
            calendar->setTime(defaultCenturyStart);
            if (parsedDate < defaultCenturyStart) {
                calb.addYear(100).establish(calendar.get());
                parsedDate = calendar->getTime();
            }
        }
    } catch (std::exception&) {
        // Calendar.getTime() throws on out-of-range fields (e.g. MONTH == 17).
        pos.setErrorIndex(start);
        pos.setIndex(oldStart);
        return 0;
    }

    return parsedDate;
}

bool SimpleDateFormat::shouldObeyCount(int tag, int count) const {
    switch (tag) {
    case PATTERN_MONTH:
    case PATTERN_MONTH_STANDALONE:
        return count <= 2;
    case PATTERN_YEAR:
    case PATTERN_DAY_OF_MONTH:
    case PATTERN_HOUR_OF_DAY1:
    case PATTERN_HOUR_OF_DAY0:
    case PATTERN_MINUTE:
    case PATTERN_SECOND:
    case PATTERN_MILLISECOND:
    case PATTERN_DAY_OF_YEAR:
    case PATTERN_DAY_OF_WEEK_IN_MONTH:
    case PATTERN_WEEK_OF_YEAR:
    case PATTERN_WEEK_OF_MONTH:
    case PATTERN_HOUR1:
    case PATTERN_HOUR0:
    case PATTERN_WEEK_YEAR:
    case PATTERN_ISO_DAY_OF_WEEK:
        return true;
    default:
        return false;
    }
}

int SimpleDateFormat::matchString(const std::string& text, int start, int field,
                                  const std::vector<std::string>& data,
                                  CalendarBuilder& calb) const {
    int i = 0;
    const int count = (int)data.size();

    if (field == Calendar::DAY_OF_WEEK) {
        i = 1;
    }

    // Keep the LONGEST match (multiple entries may share a prefix, e.g.
    // Cerven/Cervenec in Czech); abbreviations ending with '.' also match
    // without the period.
    int bestMatchLength = 0, bestMatch = -1;
    for (; i < count; ++i) {
        const int length = (int)data[i].size();
        if (length == 0) continue;
        if (length > bestMatchLength &&
            ciRegionMatches(text, start, data[i], 0, length)) {
            bestMatch = i;
            bestMatchLength = length;
        }
        if (data[i][length - 1] == '.' &&
            ((length - 1) > bestMatchLength) &&
            ciRegionMatches(text, start, data[i], 0, length - 1)) {
            bestMatch = i;
            bestMatchLength = length - 1;
        }
    }
    if (bestMatch >= 0) {
        calb.set(field, bestMatch);
        return start + bestMatchLength;
    }
    return -start;
}

int SimpleDateFormat::subParseNumericZone(const std::string& text, int start, int sign,
                                          int count, bool colon, CalendarBuilder& calb) const {
    int index = start;
    auto charAt = [&](int i) -> char {
        return (i >= 0 && i < (int)text.size()) ? text[i] : '\0';
    };

    // parse hh
    char c = charAt(index++);
    if (!isDigit(c)) return 1 - index;
    int hours = c - '0';
    c = charAt(index++);
    if (isDigit(c)) {
        hours = hours * 10 + (c - '0');
    } else {
        // Be tolerant of the colon (Android b/26426526).
        --index;
    }
    if (hours > 23) return 1 - index;
    int minutes = 0;
    if (count != 1) {
        // Proceed with parsing mm
        c = charAt(index++);
        if (c == ':') {
            c = charAt(index++);
        } else if (colon) {
            return 1 - index;
        }
        if (!isDigit(c)) return 1 - index;
        minutes = c - '0';
        c = charAt(index++);
        if (!isDigit(c)) return 1 - index;
        minutes = minutes * 10 + (c - '0');
        if (minutes > 59) return 1 - index;
    }
    minutes += hours * 60;
    calb.set(Calendar::ZONE_OFFSET, minutes * MILLIS_PER_MINUTE * sign)
       .set(Calendar::DST_OFFSET, 0);
    return index;
}

int SimpleDateFormat::parseMonth(const std::string& text, int count, int value, int start,
                                 int field, ParsePosition& pos, bool standalone,
                                 CalendarBuilder& out) const {
    if (count <= 2) { // i.e., M or MM: numeric style — value computed by caller.
        out.set(Calendar::MONTH, value - 1);
        return pos.getIndex();
    }

    // count >= 3 (MMM/MMMM): try the wide table first, then the abbreviated.
    int index = -1;
    if ((index = matchString(text, start, Calendar::MONTH,
            standalone ? formatData.getStandAloneMonths() : formatData.getMonths(), out)) > 0) {
        return index;
    }
    if ((index = matchString(text, start, Calendar::MONTH,
            standalone ? formatData.getShortStandAloneMonths() : formatData.getShortMonths(),
            out)) > 0) {
        return index;
    }
    return index;
}

int SimpleDateFormat::parseWeekday(const std::string& text, int start, int field,
                                   bool standalone, CalendarBuilder& out) const {
    int index = -1;
    if ((index = matchString(text, start, Calendar::DAY_OF_WEEK,
            standalone ? formatData.getStandAloneWeekdays() : formatData.getWeekdays(), out)) > 0) {
        return index;
    }
    if ((index = matchString(text, start, Calendar::DAY_OF_WEEK,
            standalone ? formatData.getShortStandAloneWeekdays() : formatData.getShortWeekdays(),
            out)) > 0) {
        return index;
    }
    return index;
}

int SimpleDateFormat::subParse(const std::string& text, int start, int patternCharIndex,
                               int count, bool obeyCount, bool& ambiguousYear,
                               ParsePosition& origPos, bool useFollowingMinusSignAsDelimiter,
                               CalendarBuilder& calb) {
    long value = 0;
    ParsePosition pos(0);
    pos.setIndex(start);
    if (patternCharIndex == PATTERN_WEEK_YEAR) {
        // No week-date getters in the Calendar libc: use calendar year 'y'.
        patternCharIndex = PATTERN_YEAR;
    }
    const int field = PATTERN_INDEX_TO_CALENDAR_FIELD[patternCharIndex];

    // Skip over any spaces; hitting the end of the string fails.
    for (;;) {
        if (pos.getIndex() >= (int)text.size()) {
            origPos.setErrorIndex(start);
            return -1;
        }
        char c = text[pos.getIndex()];
        if (c != ' ' && c != '\t') {
            break;
        }
        pos.setIndex(pos.getIndex() + 1);
    }
    const int actualStart = pos.getIndex();

    // AOSP parses through its integer NumberFormat; CDROID reads digits
    // (optional '-' sign) directly, optionally bounded by obeyCount.
    auto parseInt = [&](int from, int end, long* out) -> bool {
        int i = from;
        if (i >= end) return false;
        bool negative = false;
        if (text[i] == '-') { negative = true; i++; }
        int digits = 0;
        long v = 0;
        while (i < end && isDigit(text[i])) {
            v = v * 10 + (text[i] - '0');
            i++;
            digits++;
        }
        if (digits == 0) return false;
        if (out) *out = negative ? -v : v;
        pos.setIndex(i);
        return true;
    };

    // Lambdas give the labeled-break control flow of AOSP's `parsing:` block:
    // return >= 0 for success, -1 for "break parsing".
    auto body = [&]() -> int {
        // Special number cases processed up front (extra handling below).
        if (patternCharIndex == PATTERN_HOUR_OF_DAY1 ||
            patternCharIndex == PATTERN_HOUR1 ||
            (patternCharIndex == PATTERN_MONTH && count <= 2) ||
            (patternCharIndex == PATTERN_MONTH_STANDALONE && count <= 2) ||
            patternCharIndex == PATTERN_YEAR ||
            patternCharIndex == PATTERN_WEEK_YEAR) {
            bool ok;
            if (obeyCount) {
                if ((start + count) > (int)text.size()) {
                    return -1;
                }
                ok = parseInt(start, start + count, &value);
            } else {
                ok = parseInt(pos.getIndex(), (int)text.size(), &value);
            }
            if (!ok) {
                return -1;
            }
            if (useFollowingMinusSignAsDelimiter && (value < 0) &&
                (((pos.getIndex() < (int)text.size()) &&
                  (text[pos.getIndex()] != minusSign)) ||
                 ((pos.getIndex() == (int)text.size()) &&
                  (text[pos.getIndex() - 1] == minusSign)))) {
                value = -value;
                pos.setIndex(pos.getIndex() - 1);
            }
        }

        int index;
        switch (patternCharIndex) {
        case PATTERN_ERA: // 'G'
            if ((index = matchString(text, start, Calendar::ERA, formatData.getEras(), calb)) > 0) {
                return index;
            }
            return -1;

        case PATTERN_WEEK_YEAR: // 'Y'
        case PATTERN_YEAR:      // 'y'
            // 3+ pattern letters: year treated literally; otherwise the
            // 2-digit year lands in the default century (80 back / 20 fwd).
            if (count <= 2 && (pos.getIndex() - actualStart) == 2
                && isDigit(text[actualStart])
                && isDigit(text[actualStart + 1])) {
                const int ambiguousTwoDigitYear = defaultCenturyStartYear % 100;
                ambiguousYear = (value == ambiguousTwoDigitYear);
                value += (defaultCenturyStartYear / 100) * 100
                       + (value < ambiguousTwoDigitYear ? 100 : 0);
            }
            calb.set(field, (int)value);
            return pos.getIndex();

        case PATTERN_MONTH: // 'M'
        {
            const int idx = parseMonth(text, count, (int)value, start, field, pos,
                                       false /* standalone */, calb);
            if (idx > 0) {
                return idx;
            }
            return -1;
        }

        case PATTERN_MONTH_STANDALONE: // 'L'
        {
            const int idx = parseMonth(text, count, (int)value, start, field, pos,
                                       true /* standalone */, calb);
            if (idx > 0) {
                return idx;
            }
            return -1;
        }

        case PATTERN_HOUR_OF_DAY1: // 'k' 1-based.  eg, 23:59 + 1 hour =>> 24:59
            if (!calendar->isLenient()) {
                if (value < 1 || value > 24) {
                    return -1;
                }
            }
            if (value == calendar->getMaximum(Calendar::HOUR_OF_DAY) + 1) {
                value = 0;
            }
            calb.set(Calendar::HOUR_OF_DAY, (int)value);
            return pos.getIndex();

        case PATTERN_DAY_OF_WEEK: // 'E'
        {
            const int idx = parseWeekday(text, start, field, false /* standalone */, calb);
            if (idx > 0) {
                return idx;
            }
            return -1;
        }

        case PATTERN_STANDALONE_DAY_OF_WEEK: // 'c'
        {
            const int idx = parseWeekday(text, start, field, true /* standalone */, calb);
            if (idx > 0) {
                return idx;
            }
            return -1;
        }

        case PATTERN_AM_PM: // 'a'
            if ((index = matchString(text, start, Calendar::AM_PM,
                                     formatData.getAmPmStrings(), calb)) > 0) {
                return index;
            }
            return -1;

        case PATTERN_HOUR1: // 'h' 1-based.  eg, 11PM + 1 hour =>> 12 AM
            if (!calendar->isLenient()) {
                if (value < 1 || value > 12) {
                    return -1;
                }
            }
            if (value == calendar->getLeastMaximum(Calendar::HOUR) + 1) {
                value = 0;
            }
            calb.set(Calendar::HOUR, (int)value);
            return pos.getIndex();

        case PATTERN_ZONE_NAME:  // 'z'
        case PATTERN_ZONE_VALUE: // 'Z'
        {
            int sign = 0;
            const int pindex = pos.getIndex();
            if (pindex >= (int)text.size()) return -1;
            char c = text[pindex];
            if (c == '+') {
                sign = 1;
            } else if (c == '-') {
                sign = -1;
            }
            if (sign == 0) {
                // Try parsing a custom time zone "GMT+hh:mm" or "GMT".
                if ((c == 'G' || c == 'g')
                    && ((int)text.size() - start) >= 3
                    && ciRegionMatches(text, start, GMT, 0, 3)) {
                    pos.setIndex(start + 3);

                    if (((int)text.size() - pos.getIndex()) > 0) {
                        c = text[pos.getIndex()];
                        if (c == '+') {
                            sign = 1;
                        } else if (c == '-') {
                            sign = -1;
                        }
                    }

                    if (sign == 0) { /* "GMT" without offset */
                        calb.set(Calendar::ZONE_OFFSET, 0).set(Calendar::DST_OFFSET, 0);
                        return pos.getIndex();
                    }

                    // Parse the rest as "hh[:]?mm" (colon tolerated, b/26426526).
                    const int i = subParseNumericZone(text, pos.getIndex() + 1, sign, 0,
                                                      false, calb);
                    if (i > 0) {
                        return i;
                    }
                    pos.setIndex(-i);
                    return -1;
                } else {
                    // No zone display-name table: a named zone fails to parse.
                    pos.setIndex(start);
                    return -1;
                }
            } else {
                // RFC 822 style "hh[:]?mm".
                const int i = subParseNumericZone(text, pos.getIndex() + 1, sign, 0,
                                                  false, calb);
                if (i > 0) {
                    return i;
                }
                pos.setIndex(-i);
                return -1;
            }
        }

        case PATTERN_ISO_ZONE: // 'X'
        {
            if (((int)text.size() - pos.getIndex()) <= 0) {
                return -1;
            }

            int sign;
            char c = text[pos.getIndex()];
            if (c == 'Z') {
                calb.set(Calendar::ZONE_OFFSET, 0).set(Calendar::DST_OFFSET, 0);
                pos.setIndex(pos.getIndex() + 1);
                return pos.getIndex();
            }

            // parse text as "+/-hh[[:]mm]" based on count
            if (c == '+') {
                sign = 1;
            } else if (c == '-') {
                sign = -1;
            } else {
                pos.setIndex(pos.getIndex() + 1);
                return -1;
            }
            const int i = subParseNumericZone(text, pos.getIndex() + 1, sign, count,
                                              count == 3, calb);
            if (i > 0) {
                return i;
            }
            pos.setIndex(-i);
            return -1;
        }

        default:
            // 'd' 'H' 'm' 's' 'S' 'D' 'F' 'w' 'W' 'K' 'u': generic numeric.
            const int parseStart = pos.getIndex();
            bool ok;
            if (obeyCount) {
                if ((start + count) > (int)text.size()) {
                    return -1;
                }
                ok = parseInt(pos.getIndex(), start + count, &value);
            } else {
                ok = parseInt(pos.getIndex(), (int)text.size(), &value);
            }
            if (ok) {
                if (patternCharIndex == PATTERN_MILLISECOND) {
                    // Fractional seconds normalize to [0, 1) then to millis:
                    // "7890" is 789ms, "78" is 780ms, "7" is 700ms.
                    const int width = pos.getIndex() - parseStart;
                    value = (long)(((double)value / pow10Int(width)) * 1000);
                }

                if (useFollowingMinusSignAsDelimiter && (value < 0) &&
                    (((pos.getIndex() < (int)text.size()) &&
                      (text[pos.getIndex()] != minusSign)) ||
                     ((pos.getIndex() == (int)text.size()) &&
                      (text[pos.getIndex() - 1] == minusSign)))) {
                    value = -value;
                    pos.setIndex(pos.getIndex() - 1);
                }

                calb.set(field, (int)value);
                return pos.getIndex();
            }
            return -1;
        }
    };

    const int result = body();
    if (result < 0) {
        // Parsing failed.
        origPos.setErrorIndex(pos.getIndex());
        return -1;
    }
    return result;
}

// ---------------------------------------------------------------------------
// Pattern / symbols / DateFormat surface
// ---------------------------------------------------------------------------

void SimpleDateFormat::applyPattern(const std::string& pat) {
    applyPatternImpl(pat);
}

void SimpleDateFormat::applyPatternImpl(const std::string& pat) {
    compiledPattern = compile(pat);
    pattern = pat;
}

std::string SimpleDateFormat::toPattern() const {
    return pattern;
}

std::string SimpleDateFormat::translatePattern(const std::string& pat, const std::string& from,
                                               const std::string& to) const {
    std::string result;
    bool inQuote = false;
    for (size_t i = 0; i < pat.size(); ++i) {
        char c = pat[i];
        if (inQuote) {
            if (c == '\'') {
                inQuote = false;
            }
        } else {
            if (c == '\'') {
                inQuote = true;
            } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                size_t ci = from.find(c);
                if (ci != std::string::npos) {
                    // patternChars is longer than localPatternChars due to
                    // serialization compatibility; unsupported letters pass through.
                    if (ci < to.size()) {
                        c = to[ci];
                    }
                } else {
                    throw std::invalid_argument(std::string("Illegal pattern character '") + c + "'");
                }
            }
        }
        result += c;
    }
    if (inQuote) {
        throw std::invalid_argument("Unfinished quote in pattern");
    }
    return result;
}

std::string SimpleDateFormat::toLocalizedPattern() const {
    return translatePattern(pattern, DateFormatSymbols::PATTERN_CHARS,
                            formatData.getLocalPatternChars());
}

void SimpleDateFormat::applyLocalizedPattern(const std::string& pat) {
    const std::string p = translatePattern(pat, formatData.getLocalPatternChars(),
                                           DateFormatSymbols::PATTERN_CHARS);
    compiledPattern = compile(p);
    pattern = p;
}

DateFormatSymbols SimpleDateFormat::getDateFormatSymbols() const {
    return formatData;
}

void SimpleDateFormat::setDateFormatSymbols(const DateFormatSymbols& newFormatSymbols) {
    formatData = newFormatSymbols;
    useDateFormatSymbolsFlag = true;
}

Calendar& SimpleDateFormat::getCalendar() const { return *calendar; }

void SimpleDateFormat::setLenient(bool lenient) { calendar->setLenient(lenient); }
bool SimpleDateFormat::isLenient() const { return calendar->isLenient(); }
void SimpleDateFormat::setTimeZone(int offsetSeconds) { calendar->setTimeZone(offsetSeconds); }
int SimpleDateFormat::getTimeZone() const { return calendar->getTimeZone(); }

bool SimpleDateFormat::operator==(const DateFormat& that) const {
    const auto* o = dynamic_cast<const SimpleDateFormat*>(&that);
    if (o == nullptr) return false;
    return pattern == o->pattern && formatData == o->formatData;
}

}//endof namespace
