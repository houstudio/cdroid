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
#include <text/format/dateutils.h>
#include <content/Locale.h>
#include <content/dateformat.h>
#include <content/dateformatsymbols.h>
#include <content/simpledateformat.h>
#include <content/numberformat.h>
#include <core/calendar.h>
#include <core/context.h>
#include <core/systemclock.h>
#include <gui_features.h>
#include <content/i18nbridge.h>
#ifdef ENABLE_I18N
#include <content/i18n/date_time_format.h>
#include <content/i18n/types.h>
#endif
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <mutex>

namespace cdroid{
namespace DateUtils{

// ---- name getters (AOSP :206/:234/:251) ------------------------------------
/*AOSP's width switch, transcribed: LONG→WIDE, SHORTEST→NARROW, and
  MEDIUM/SHORT/SHORTER plus every undefined value→ABBREVIATED ("in most
  languages LENGTH_SHORT returns the same as LENGTH_MEDIUM"). The old
  `abbrev >= LENGTH_SHORT` range test sent LENGTH_MEDIUM (20) and undefined
  values to the wide tier. CDROID's DateFormatSymbols is three-tier, so
  WIDE→get*(), ABBREVIATED→getShort*(), NARROW→getTiny*().*/
enum NameWidth { WIDTH_WIDE, WIDTH_ABBREVIATED, WIDTH_NARROW };

static NameWidth widthFor(int abbrev) {
    switch (abbrev) {
        case LENGTH_LONG:     return WIDTH_WIDE;
        case LENGTH_SHORTEST: return WIDTH_NARROW;
        case LENGTH_MEDIUM:
        case LENGTH_SHORT:
        case LENGTH_SHORTER:
        default:              return WIDTH_ABBREVIATED;
    }
}

/*DateFormatSymbols builds its full name tables per construction; the name
  getters run per format call, so the default-locale instance is cached and
  rebuilt only when the default locale changes. The cache hands out a
  shared_ptr: the tables are returned by reference, and a rebuild must not
  yank them out from under a concurrent reader (the old instance lives until
  its last holder lets go).*/
static std::shared_ptr<const DateFormatSymbols> defaultSymbols() {
    static std::mutex sMutex;
    std::lock_guard<std::mutex> lock(sMutex);
    static std::string tag;
    static std::shared_ptr<const DateFormatSymbols> cache;
    const std::string cur = Locale::getDefault().toLanguageTag();
    if (cache == nullptr || tag != cur) {
        tag = cur;
        cache.reset(new DateFormatSymbols(Locale::getDefault()));
    }
    return cache;
}

std::string getDayOfWeekString(int dayOfWeek, int abbrev) {
    const std::shared_ptr<const DateFormatSymbols> dfs = defaultSymbols();
    const std::vector<std::string>* names = nullptr;
    switch (widthFor(abbrev)) {
        case WIDTH_NARROW:     names = &dfs->getTinyWeekdays(); break;
        case WIDTH_ABBREVIATED:names = &dfs->getShortWeekdays(); break;
        default:               names = &dfs->getWeekdays(); break;
    }
    // Calendar weekday indices are 1-based (SUNDAY..SATURDAY) over an array
    // whose slot 0 is the empty leading entry.
    if (dayOfWeek < 1 || dayOfWeek >= (int)names->size()) return std::string();
    return (*names)[dayOfWeek];
}

std::string getAMPMString(int ampm) {
    const std::shared_ptr<const DateFormatSymbols> dfs = defaultSymbols();
    const std::vector<std::string>& names = dfs->getAmPmStrings();
    if (ampm < 0 || ampm >= (int)names.size()) return std::string();
    return names[ampm];
}

std::string getMonthString(int month, int abbrev) {
    const std::shared_ptr<const DateFormatSymbols> dfs = defaultSymbols();
    const std::vector<std::string>* names = nullptr;
    switch (widthFor(abbrev)) {
        case WIDTH_NARROW:     names = &dfs->getTinyMonths(); break;
        case WIDTH_ABBREVIATED:names = &dfs->getShortMonths(); break;
        default:               names = &dfs->getMonths(); break;
    }
    if (month < 0 || month >= (int)names->size()) return std::string();
    return (*names)[month];
}

// ---- elapsed time (AOSP formatElapsedTime, body verbatim) -------------------
std::string formatElapsedTime(int64_t elapsedSeconds) {
    return formatElapsedTime(nullptr, elapsedSeconds);
}

std::string formatElapsedTime(std::string* recycle, int64_t elapsedSeconds) {
    // Break the elapsed seconds into hours, minutes, and seconds.
    int64_t hours = 0, minutes = 0, seconds = 0;
    if (elapsedSeconds >= 3600) {
        hours = elapsedSeconds / 3600;
        elapsedSeconds -= hours * 3600;
    }
    if (elapsedSeconds >= 60) {
        minutes = elapsedSeconds / 60;
        elapsedSeconds -= minutes * 60;
    }
    seconds = elapsedSeconds;

    // Format the broken-down time in a locale-appropriate way: the engine's
    // elapsed patterns (AOSP goes through String.format(Locale.getDefault())).
    std::string out;
#ifdef ENABLE_I18N
    {
        /*The engine cache is process-wide (Java guards its equivalents with
          synchronized statics); formatElapsedTime is callable from any
          thread, so the tag/cache pair takes a mutex (LocaleList's
          DefaultState pattern).*/
        static std::mutex sEngineMutex;
        std::lock_guard<std::mutex> engineLock(sEngineMutex);
        static std::string tag;
        static std::unique_ptr<i18n::DateTimeFormat> cache;
        const std::string cur = Locale::getDefault().toLanguageTag();
        if (tag != cur || cache == nullptr) {
            tag = cur;
            i18n::LocaleInfo info = I18nBridge::toLocaleInfo(Locale::getDefault());
            cache.reset(new i18n::DateTimeFormat(i18n::AvailableDateTimeFormatPattern::FULL, info));
        }
        i18n::I18nStatus status = i18n::I18nStatus::ISUCCESS;
        // The engine takes int32 milliseconds; totalMs overflows past
        // 2147483 s (~24.86 days) — embedded uptimes reach that in weeks.
        // Take the localized engine only inside its range, plain fallback
        // beyond (AOSP formats the broken-down longs directly, so this only
        // affects which formatter spells the same H:MM:SS).
        const int64_t totalMs =
                (int64_t)(hours * 3600 + minutes * 60 + seconds) * 1000;
        if (totalMs <= INT32_MAX) {
            out = cache->FormatElapsedDuration(
                    (int32_t)totalMs,
                    (hours > 0) ? i18n::ELAPSED_HOUR_MINUTE_SECOND
                                : i18n::ELAPSED_MINUTE_SECOND,
                    status);
        }
        if (status == i18n::I18nStatus::ISUCCESS && !out.empty()) {
            if (recycle) { *recycle = out; return *recycle; }
            return out;
        }
    }
#endif
    // No-i18n / engine-miss fallback: the plain "%d:%02d:%02d" / "%02d:%02d".
    char buf[64];
    if (hours > 0) snprintf(buf, sizeof(buf), "%lld:%02lld:%02lld",
                            (long long)hours, (long long)minutes, (long long)seconds);
    else           snprintf(buf, sizeof(buf), "%02lld:%02lld",
                            (long long)minutes, (long long)seconds);
    out = buf;
    if (recycle) { *recycle = out; return *recycle; }
    return out;
}

// ---- date-time (AOSP formatDateTime = the same-millis range path) -----------

// AOSP DateUtilsBridge.onTheHour.
static bool onTheHour(Calendar& c) {
    return c.get(Calendar::HOUR_OF_DAY) == 0 && c.get(Calendar::MINUTE) == 0
            && c.get(Calendar::SECOND) == 0;
}

/*Calendar boilerplate shared by the bridge helpers: the default-locale
  calendar positioned at a UTC millis (AOSP Calendar.getInstance() +
  setTimeInMillis()), and the calendar-field date equality the AOSP
  LocalDateTime comparisons reduce to (three call sites below).*/
static std::unique_ptr<Calendar> calendarAt(int64_t millis) {
    auto cal = Calendar::getInstance(Locale::getDefault());
    cal->setTimeInMillis(millis);
    return cal;
}

static bool sameDateFields(const Calendar& a, const Calendar& b) {
    return a.get(Calendar::YEAR) == b.get(Calendar::YEAR)
            && a.get(Calendar::MONTH) == b.get(Calendar::MONTH)
            && a.get(Calendar::DAY_OF_MONTH) == b.get(Calendar::DAY_OF_MONTH);
}

// AOSP DateUtilsBridge.isThisYear.
static bool isThisYear(Calendar& c) {
    auto now = Calendar::getInstance(Locale::getDefault());
    return now->get(Calendar::YEAR) == c.get(Calendar::YEAR);
}

// AOSP DateUtilsBridge.fallOnDifferentDates / fallInSameMonth / fallInSameYear
// and isDisplayMidnightUsingSkeleton (:165-187).
static bool fallOnDifferentDates(Calendar& c1, Calendar& c2) {
    return !sameDateFields(c1, c2);
}

static bool fallInSameMonth(Calendar& c1, Calendar& c2) {
    return c1.get(Calendar::MONTH) == c2.get(Calendar::MONTH);
}

static bool fallInSameYear(Calendar& c1, Calendar& c2) {
    return c1.get(Calendar::YEAR) == c2.get(Calendar::YEAR);
}

static bool isDisplayMidnightUsingSkeleton(Calendar& c) {
    // All the skeletons returned by toSkeleton have minute precision (they may
    // abbreviate 4:00 PM to 4 PM but will still show the following minute).
    return c.get(Calendar::HOUR_OF_DAY) == 0 && c.get(Calendar::MINUTE) == 0;
}

// AOSP DateUtilsBridge.toSkeleton(startCalendar, endCalendar, flags) — the
// single-instant callers pass the same calendar twice (the AOSP same-millis
// range path).
static std::string toSkeleton(Calendar& startCalendar, Calendar& endCalendar, int flags) {
    if ((flags & FORMAT_ABBREV_ALL) != 0) {
        flags |= FORMAT_ABBREV_MONTH | FORMAT_ABBREV_TIME | FORMAT_ABBREV_WEEKDAY;
    }

    std::string monthPart = "MMMM";
    if ((flags & FORMAT_NUMERIC_DATE) != 0) {
        monthPart = "M";
    } else if ((flags & FORMAT_ABBREV_MONTH) != 0) {
        monthPart = "MMM";
    }

    std::string weekPart = "EEEE";
    if ((flags & FORMAT_ABBREV_WEEKDAY) != 0) {
        weekPart = "EEE";
    }

    std::string timePart = "j";  // 12/24-hour per the current locale
    if ((flags & FORMAT_24HOUR) != 0) {
        timePart = "H";
    } else if ((flags & FORMAT_12HOUR) != 0) {
        timePart = "h";
    }

    // If we've not been asked to abbreviate times, or we're using the 24-hour
    // clock (where it never makes sense to leave out the minutes), include
    // minutes. This gets us times like "4 PM" while avoiding "16" for 16:00.
    if ((flags & FORMAT_ABBREV_TIME) == 0 || (flags & FORMAT_24HOUR) != 0) {
        timePart += "m";
    } else {
        // Abbreviating a 12-hour time: only show the minutes if they're not
        // both "00".
        if (!(onTheHour(startCalendar) && onTheHour(endCalendar))) {
            timePart += "m";
        }
    }

    if (fallOnDifferentDates(startCalendar, endCalendar)) {
        flags |= FORMAT_SHOW_DATE;
    }

    if (fallInSameMonth(startCalendar, endCalendar) && (flags & FORMAT_NO_MONTH_DAY) != 0) {
        flags &= ~FORMAT_SHOW_WEEKDAY;
        flags &= ~FORMAT_SHOW_TIME;
    }

    if ((flags & (FORMAT_SHOW_DATE | FORMAT_SHOW_TIME | FORMAT_SHOW_WEEKDAY)) == 0) {
        flags |= FORMAT_SHOW_DATE;
    }

    // If we've been asked to show the date, work out whether to show the year.
    if ((flags & FORMAT_SHOW_DATE) != 0) {
        if ((flags & FORMAT_SHOW_YEAR) != 0) {
            // The caller explicitly wants us to show the year.
        } else if ((flags & FORMAT_NO_YEAR) != 0) {
            // The caller explicitly doesn't want the year, even if we
            // otherwise would.
        } else if (!fallInSameYear(startCalendar, endCalendar) || !isThisYear(startCalendar)) {
            flags |= FORMAT_SHOW_YEAR;
        }
    }

    std::string builder;
    if ((flags & (FORMAT_SHOW_DATE | FORMAT_NO_MONTH_DAY)) != 0) {
        if ((flags & FORMAT_SHOW_YEAR) != 0) {
            builder += "y";
        }
        builder += monthPart;
        if ((flags & FORMAT_NO_MONTH_DAY) == 0) {
            builder += "d";
        }
    }
    if ((flags & FORMAT_SHOW_WEEKDAY) != 0) {
        builder += weekPart;
    }
    if ((flags & FORMAT_SHOW_TIME) != 0) {
        builder += timePart;
    }
    return builder;
}

std::string formatDateTime(Context* context, int64_t millis, int flags) {
    /*AOSP: formatDateTime(context, millis, flags) == formatDateRange(context,
      millis, millis, flags) — which includes the user-preference 12/24
      forcing (a bare FORMAT_SHOW_TIME resolves through
      DateFormat.is24HourFormat, not the locale's 'j' default). The old
      direct toSkeleton path skipped that forcing, so a SHOW_TIME-only call
      always followed the locale's hour cycle.*/
    return formatDateRange(context, millis, millis, flags);
}

// ---- duration (AOSP formatDuration, DateUtils.java:384/:400) ----------------

/*AOSP formats through ICU MeasureFormat with the locale's unit words per
  WIDE/SHORT/NARROW. The engine's MEASURE_FORMAT_PATTERN table currently
  carries the SHORT tier only (4 locales, mined from ICU — see
  scripts/extract_measure_units.cc), so SHORT routes through I18nBridge
  (formatter.cc's measureShort uses the same bridge) and WIDE/NARROW keep
  the inline en-US words until wide/narrow unit data exists. A bridge miss
  (locale/unit not in the table) falls back to the inline words too, so
  uncovered locales keep today's behavior.*/
static std::string durationMeasure(int64_t value, int width, const char* unit,
        const char* wideSingle, const char* widePlural,
        const char* shortUnit, const char* narrowUnit) {
    char buf[48];
    switch (width) {
        case 1: {  // SHORT
#ifdef ENABLE_I18N
            const std::string formatted =
                    I18nBridge::measureUnitShort(Locale::getDefault(), (int)value, unit);
            if (!formatted.empty()) return formatted;
#endif
            snprintf(buf, sizeof(buf), "%lld %s", (long long)value, shortUnit);
            break;
        }
        case 0:  // WIDE
            snprintf(buf, sizeof(buf), "%lld %s", (long long)value,
                    value == 1 ? wideSingle : widePlural);
            break;
        default: // NARROW
            snprintf(buf, sizeof(buf), "%lld%s", (long long)value, narrowUnit);
            break;
    }
    return buf;
}

std::string formatDuration(int64_t millis) {
    return formatDuration(millis, LENGTH_LONG);
}

std::string formatDuration(int64_t millis, int abbrev) {
    int width;  // MeasureFormat FormatWidth: 0=WIDE, 1=SHORT, 2=NARROW
    switch (abbrev) {
        case LENGTH_LONG:
            width = 0;
            break;
        case LENGTH_SHORT:
        case LENGTH_SHORTER:
        case LENGTH_MEDIUM:
            width = 1;
            break;
        case LENGTH_SHORTEST:
            width = 2;
            break;
        default:
            width = 0;
    }
    if (millis >= HOUR_IN_MILLIS) {
        const int hours = (int)((millis + 1800000) / HOUR_IN_MILLIS);
        return durationMeasure(hours, width, "hour", "hour", "hours", "hr", "h");
    } else if (millis >= MINUTE_IN_MILLIS) {
        const int minutes = (int)((millis + 30000) / MINUTE_IN_MILLIS);
        return durationMeasure(minutes, width, "minute", "minute", "minutes", "min", "m");
    } else {
        const int seconds = (int)((millis + 500) / SECOND_IN_MILLIS);
        return durationMeasure(seconds, width, "second", "second", "seconds", "sec", "s");
    }
}

// ---- same-day (AOSP formatSameDayTime, DateUtils.java:499) ------------------

std::string formatSameDayTime(int64_t then, int64_t now, int dateStyle, int timeStyle) {
    auto thenCal = calendarAt(then);
    auto nowCal = calendarAt(now);

    // AOSP getTimeInstance for the same day, getDateInstance otherwise; the
    // factories return a new DateFormat the caller owns.
    DateFormat* f;
    if (sameDateFields(*thenCal, *nowCal)) {
        f = DateFormat::getTimeInstance(timeStyle);
    } else {
        f = DateFormat::getDateInstance(dateStyle);
    }
    const std::string result = f->format(thenCal->getTimeInMillis());
    delete f;
    return result;
}

// ---- day check (AOSP isToday / isSameDate, DateUtils.java:522/:526) ---------

static bool isSameDate(int64_t oneMillis, int64_t twoMillis) {
    // AOSP compares LocalDateTime fields in the system zone; Calendar in the
    // default zone carries the same fields.
    auto one = calendarAt(oneMillis);
    auto two = calendarAt(twoMillis);
    return sameDateFields(*one, *two);
}

bool isToday(int64_t when) {
    return isSameDate(when, SystemClock::currentTimeMillis());
}

// ---- relative time (AOSP RelativeDateTimeFormatter.java, the @hide framework
// helper DateUtils forwards to; skeleton ported line-for-line, the ICU
// formatter itself is an en-US CLDR word table) -------------------------------

namespace {

// RelativeDateTimeFormatter.Direction / RelativeUnit / AbsoluteUnit.DAY,
// internalized.
enum class RelDirection { LAST, NEXT, THIS, LAST_2, NEXT_2 };
enum class RelUnit { SECONDS, MINUTES, HOURS, DAYS, WEEKS };

// CLDR en relative-unit patterns (LONG / SHORT). ICU resolves the plural
// category per count; the English table only distinguishes one/other.
struct RelUnitWords {
    const char* oneLong;  const char* oneShort;
    const char* otherLong; const char* otherShort;
};
const RelUnitWords kRelUnits[] = {
    { "second", "sec.",  "seconds", "sec."  },  // SECONDS
    { "minute", "min.",  "minutes", "min."  },  // MINUTES
    { "hour",   "hr.",   "hours",   "hr."   },  // HOURS
    { "day",    "dy.",   "days",    "dy."   },  // DAYS
    { "week",   "wk.",   "weeks",   "wk."   },  // WEEKS
};

// icu RelativeDateTimeFormatter.format(count, direction, unit): CLDR en
// patterns are "{0} seconds ago" (LAST) / "in {0} seconds" (NEXT), with the
// short variant "in {0} sec.".
std::string relFormatNumeric(int count, RelDirection direction, RelUnit unit, bool shortStyle) {
    const RelUnitWords& words = kRelUnits[(int)unit];
    const bool one = (count == 1);
    const char* unitWord = shortStyle ? (one ? words.oneShort : words.otherShort)
                                      : (one ? words.oneLong : words.otherLong);
    char buf[64];
    if (direction == RelDirection::LAST) {
        snprintf(buf, sizeof(buf), "%d %s ago", count, unitWord);
    } else {  // NEXT
        snprintf(buf, sizeof(buf), "in %d %s", count, unitWord);
    }
    return buf;
}

// icu RelativeDateTimeFormatter.format(direction, AbsoluteUnit.DAY): the
// absolute day words. English has no "2 days ago" special (LAST_2/NEXT_2 come
// back empty and the caller falls through — preserved by returning "").
std::string relFormatAbsoluteDay(RelDirection direction) {
    switch (direction) {
        case RelDirection::THIS:   return "today";
        case RelDirection::LAST:   return "yesterday";
        case RelDirection::NEXT:   return "tomorrow";
        default:                   return std::string();  // LAST_2 / NEXT_2: none in en
    }
}

// icu RelativeDateTimeFormatter.combineDateAndTime: "{date}, {time}".
std::string relCombineDateAndTime(const std::string& dateClause, const std::string& timeClause) {
    return dateClause + ", " + timeClause;
}

// RelativeDateTimeFormatter.julianDay over the default-zone calendar fields.
// Gregorian calendar date → Julian day number (integer division truncates
// toward zero, same as the Java arithmetic the formula is written for).
static int julianDayOf(const Calendar& cal) {
    const int y = cal.get(Calendar::YEAR);
    const int m = cal.get(Calendar::MONTH) + 1;   // Calendar.MONTH is 0-based
    const int d = cal.get(Calendar::DAY_OF_MONTH);
    const int a = (m - 14) / 12;
    return (1461 * (y + 4800 + a)) / 4
            + (367 * (m - 2 - 12 * a)) / 12
            - (3 * ((y + 4900 + a) / 100)) / 4
            + d - 32075;
}

// RelativeDateTimeFormatter.dayDistance: end's local day minus start's.
static int dayDistance(int64_t startTime, int64_t endTime) {
    auto startCal = calendarAt(startTime);
    auto endCal = calendarAt(endTime);
    return julianDayOf(*endCal) - julianDayOf(*startCal);
}

// DateUtilsBridge.DateTimeFormat.format(locale, calendar, flags, context):
// the flags→skeleton→pattern path formatDateTime runs (no Context face).
static std::string dateTimeFormatFlags(Calendar& calendar, int flags) {
    const std::string skeleton = toSkeleton(calendar, calendar, flags);
    const Locale locale = Locale::getDefault();
    const std::string pattern = DateFormat::getBestDateTimePattern(locale, skeleton);
    SimpleDateFormat formatter(pattern, locale);
    return formatter.format(calendar.getTimeInMillis());
}

// RelativeDateTimeFormatter.getRelativeTimeSpanString core (:118).
std::string relativeTimeSpanString(int64_t time, int64_t now, int64_t minResolution,
        int flags) {
    const int64_t duration = (now - time) < 0 ? -(now - time) : (now - time);
    const bool past = (now >= time);

    const bool shortStyle = (flags & (FORMAT_ABBREV_RELATIVE | FORMAT_ABBREV_ALL)) != 0;

    RelDirection direction = past ? RelDirection::LAST : RelDirection::NEXT;

    // 'relative' defaults to true; set false for the no-quantity day words.
    bool relative = true;
    int count = 0;
    RelUnit unit = RelUnit::SECONDS;

    if (duration < MINUTE_IN_MILLIS && minResolution < MINUTE_IN_MILLIS) {
        count = (int)(duration / SECOND_IN_MILLIS);
        unit = RelUnit::SECONDS;
    } else if (duration < HOUR_IN_MILLIS && minResolution < HOUR_IN_MILLIS) {
        count = (int)(duration / MINUTE_IN_MILLIS);
        unit = RelUnit::MINUTES;
    } else if (duration < DAY_IN_MILLIS && minResolution < DAY_IN_MILLIS) {
        // Even if 'time' actually happened yesterday, we don't format it as
        // "yesterday" in this case. Unless the duration is longer than a day,
        // or minResolution is specified as DAY_IN_MILLIS by user.
        count = (int)(duration / HOUR_IN_MILLIS);
        unit = RelUnit::HOURS;
    } else if (duration < WEEK_IN_MILLIS && minResolution < WEEK_IN_MILLIS) {
        count = dayDistance(time, now);
        if (count < 0) count = -count;
        unit = RelUnit::DAYS;

        if (count == 2) {
            // Some locales have special terms for "2 days ago". Return them if
            // available (English has none — the empty result falls through to
            // "2 days ago", mirroring the AOSP structure).
            std::string str = relFormatAbsoluteDay(past ? RelDirection::LAST_2
                                                        : RelDirection::NEXT_2);
            if (!str.empty()) {
                return str;
            }
            // Fall back to show something like "2 days ago".
        } else if (count == 1) {
            // Show "yesterday / tomorrow" instead of "1 day ago / in 1 day".
            relative = false;
        } else if (count == 0) {
            // Show "today" if time and now are on the same day.
            direction = RelDirection::THIS;
            relative = false;
        }
    } else if (minResolution == WEEK_IN_MILLIS) {
        count = (int)(duration / WEEK_IN_MILLIS);
        unit = RelUnit::WEEKS;
    } else {
        auto timeCalendar = calendarAt(time);
        // The duration is longer than a week and minResolution is not
        // WEEK_IN_MILLIS. Return the absolute date instead of relative time.

        // Bug 19822016: without an explicit year flag, show/hide the year
        // based on time vs now, not the current system time.
        if ((flags & (FORMAT_NO_YEAR | FORMAT_SHOW_YEAR)) == 0) {
            auto nowCalendar = calendarAt(now);
            if (timeCalendar->get(Calendar::YEAR) != nowCalendar->get(Calendar::YEAR)) {
                flags |= FORMAT_SHOW_YEAR;
            } else {
                flags |= FORMAT_NO_YEAR;
            }
        }
        return dateTimeFormatFlags(*timeCalendar, flags);
    }

    if (relative) {
        return relFormatNumeric(count, direction, unit, shortStyle);
    } else {
        // The absolute-day words: direction was flipped to THIS for count==0;
        // count==1 keeps the LAST/NEXT direction set at the top.
        return relFormatAbsoluteDay(direction);
    }
}

} // namespace

std::string getRelativeTimeSpanString(int64_t startTime) {
    return getRelativeTimeSpanString(startTime, SystemClock::currentTimeMillis(),
            MINUTE_IN_MILLIS);
}

std::string getRelativeTimeSpanString(int64_t time, int64_t now, int64_t minResolution) {
    const int flags = FORMAT_SHOW_DATE | FORMAT_SHOW_YEAR | FORMAT_ABBREV_MONTH;
    return getRelativeTimeSpanString(time, now, minResolution, flags);
}

std::string getRelativeTimeSpanString(int64_t time, int64_t now, int64_t minResolution,
        int flags) {
    return relativeTimeSpanString(time, now, minResolution, flags);
}

std::string getRelativeDateTimeString(Context* c, int64_t time, int64_t minResolution,
        int64_t transitionResolution, int flags) {
    // Same reason as in formatDateRange() to explicitly indicate 12- or 24-hour format.
    if ((flags & (FORMAT_SHOW_TIME | FORMAT_12HOUR | FORMAT_24HOUR)) == FORMAT_SHOW_TIME) {
        flags |= DateFormat::is24HourFormat(c) ? FORMAT_24HOUR : FORMAT_12HOUR;
    }

    const int64_t now = SystemClock::currentTimeMillis();
    /*AOSP: Math.abs(now - time). Java's subtraction wraps; C++ signed
      overflow and the INT64_MIN negation are UB, so wrap in uint64 and map
      back — identical bits to Java for every |diff| < 2^63, and a 2^63
      difference lands on INT64_MIN exactly like Java's abs does.*/
    const uint64_t rawDiff = (uint64_t)now - (uint64_t)time;
    const int64_t duration = rawDiff <= (uint64_t)INT64_MAX
            ? (int64_t)rawDiff
            : (int64_t)(UINT64_MAX - rawDiff + 1);
    // It doesn't make much sense to have results like: "1 week ago, 10:50 AM".
    if (transitionResolution > WEEK_IN_MILLIS) {
        transitionResolution = WEEK_IN_MILLIS;
    }

    auto timeCalendar = calendarAt(time);
    auto nowCalendar = calendarAt(now);

    // AOSP: Math.abs(dayDistance(time, now)) — one call, not three.
    const int days = std::abs(dayDistance(time, now));

    // Now get the date clause, either in relative format or the actual date.
    std::string dateClause;
    if (duration < transitionResolution) {
        // Bug 5252772: any date difference promotes minResolution to
        // DAY_IN_MILLIS so the date shows instead of "x hours/minutes ago".
        if (days > 0 && minResolution < DAY_IN_MILLIS) {
            minResolution = DAY_IN_MILLIS;
        }
        dateClause = relativeTimeSpanString(time, now, minResolution, flags);
    } else {
        // We always use fixed flags to format the date clause. User-supplied
        // flags are ignored.
        if (timeCalendar->get(Calendar::YEAR) != nowCalendar->get(Calendar::YEAR)) {
            // Different years
            flags = FORMAT_SHOW_DATE | FORMAT_SHOW_YEAR | FORMAT_NUMERIC_DATE;
        } else {
            // Default
            flags = FORMAT_SHOW_DATE | FORMAT_NO_YEAR | FORMAT_ABBREV_MONTH;
        }
        dateClause = dateTimeFormatFlags(*timeCalendar, flags);
    }

    std::string timeClause = dateTimeFormatFlags(*timeCalendar, FORMAT_SHOW_TIME);

    // Combine the two clauses, such as '5 days ago, 10:50 AM'.
    return relCombineDateAndTime(dateClause, timeClause);
}

// ---- date range (AOSP DateUtils.formatDateRange + the @hide framework
// DateIntervalFormat.formatDateRange, :751/DateIntervalFormat.java:68) --------
// The ICU DateIntervalFormat merge (per-locale interval patterns, per-field
// elision) is approximated by formatting both ends with the shared skeleton
// pattern and stripping the common prefix/suffix — the en-US "{start} – {end}"
// behavior ("Oct 9 – 10", "Oct 28 – Nov 3, 2007", "3:00 – 4:00 PM").
// KNOWN DEVIATIONS: java.util.Formatter-append overloads are not ported
// (string returns); the olsonId/FORMAT_UTC zone selection has no TimeZone
// class to act on (the default zone is used); FORMAT_NO_NOON/CAP_NOON/
// NO_MIDNIGHT/CAP_MIDNIGHT are ignored (android_12 relies on ICU patterns).

// android.icu.text.DateIntervalFormat.format(start, end) stand-in.
static std::string formatInterval(Calendar& startCalendar, Calendar& endCalendar,
        const std::string& skeleton) {
    const Locale locale = Locale::getDefault();
    const std::string pattern = DateFormat::getBestDateTimePattern(locale, skeleton);
    SimpleDateFormat formatter(pattern, locale);
    const std::string s1 = formatter.format(startCalendar.getTimeInMillis());
    if (&startCalendar == &endCalendar) return s1;
    const std::string s2 = formatter.format(endCalendar.getTimeInMillis());
    if (s1 == s2) return s1;

    // Longest common prefix/suffix; the differing middles join with u8" \u2013 "
    // (CLDR en interval separator, en dash).
    size_t prefix = 0;
    const size_t minLen = s1.size() < s2.size() ? s1.size() : s2.size();
    while (prefix < minLen && s1[prefix] == s2[prefix]) prefix++;
    size_t suffix = 0;
    while (suffix < minLen - prefix && s1[s1.size() - 1 - suffix] == s2[s2.size() - 1 - suffix]) {
        suffix++;
    }
    // Field-boundary sanity (ICU merges at field granularity, we work on
    // strings): if the common prefix ends inside a digit run, back it off to
    // the run start so half a number never lands in the prefix ("Nov 10, 2007"
    // vs "Nov 11, 2007" keeps mid "10"/"11" instead of "0"/"1"). Same for the
    // suffix.
    const auto isDigit = [](char c) { return c >= '0' && c <= '9'; };
    if (prefix > 0 && isDigit(s1[prefix - 1])) {
        while (prefix > 0 && isDigit(s1[prefix - 1])) prefix--;
    }
    if (suffix > 0 && isDigit(s1[s1.size() - suffix])) {
        while (suffix > 0 && isDigit(s1[s1.size() - suffix])) suffix--;
    }
    // Time-field sanity: a bare numeric mid whose suffix starts with
    // separator+digit-run is a split minute ("3" + ":00 PM") — hand the
    // separator+run back so the middles become "3:00"/"4:00" (ICU keeps whole
    // time fields: "3:00 – 4:00 PM"). Guarded by a non-letter prefix end so a
    // date mid ("Nov " + "10") never hands its ", 2007" year suffix over.
    if (suffix > 0 && prefix < s1.size() && suffix <= s1.size()) {
        const bool midWouldBeNumeric =
                prefix < s1.size() - suffix
                && std::all_of(s1.begin() + prefix, s1.begin() + s1.size() - suffix,
                        [](char c) { return c >= '0' && c <= '9'; });
        const bool prefixEndsNonLetter = prefix == 0 || !std::isalpha((unsigned char)s1[prefix - 1]);
        const size_t f = s1.size() - suffix;      // suffix's first char index
        if (midWouldBeNumeric && prefixEndsNonLetter && f < s1.size()
                && !isDigit(s1[f]) && !std::isalpha((unsigned char)s1[f])) {
            size_t k = f + 1;
            while (k < s1.size() && isDigit(s1[k])) k++;
            if (k > f + 1) {
                suffix -= (k - f);   // separator + digit run leave the suffix
            }
        }
    }
    const std::string mid1 = s1.substr(prefix, s1.size() - prefix - suffix);
    const std::string mid2 = s2.substr(prefix, s2.size() - prefix - suffix);
    if (mid1.empty()) return s2;   // s1's differing part vanished: show s2 only
    if (mid2.empty()) return s1;
    return s1.substr(0, prefix) + mid1 + u8" \u2013 " + mid2
            + s1.substr(s1.size() - suffix);
}

// DateIntervalFormat.isExactlyMidnight.
static bool isExactlyMidnight(Calendar& c) {
    return c.get(Calendar::HOUR_OF_DAY) == 0 && c.get(Calendar::MINUTE) == 0
            && c.get(Calendar::SECOND) == 0 && c.get(Calendar::MILLISECOND) == 0;
}

static std::string formatDateRangeCore(int64_t startMs, int64_t endMs, int flags) {
    auto startCalendar = calendarAt(startMs);
    // AOSP shares the Calendar reference for start == end; the pointer alias
    // below carries that (formatInterval compares addresses).
    Calendar* endCalendar = startCalendar.get();
    std::unique_ptr<Calendar> endCalendarOwner;
    if (startMs != endMs) {
        endCalendarOwner = calendarAt(endMs);
        endCalendar = endCalendarOwner.get();
    }

    // Special handling when the range ends at midnight (DateIntervalFormat.java:78):
    // - not showing times and the range is non-empty → fudge the end date so
    //   we don't count the day that's about to start;
    // - showing times and the range ends at exactly 00:00 of the day following
    //   its start (24:00 the same day) → fudge so the dates aren't shown,
    //   unless the start is itself displayed as 00:00 (disambiguate).
    if (isExactlyMidnight(*endCalendar)) {
        const bool showTime = (flags & FORMAT_SHOW_TIME) == FORMAT_SHOW_TIME;
        const bool endsDayAfterStart = julianDayOf(*endCalendar) - julianDayOf(*startCalendar) == 1;
        if ((!showTime && startMs != endMs)
                || (endsDayAfterStart && !isDisplayMidnightUsingSkeleton(*startCalendar))) {
            endCalendar->set(Calendar::DAY_OF_MONTH, endCalendar->get(Calendar::DAY_OF_MONTH) - 1);
        }
    }

    const std::string skeleton = toSkeleton(*startCalendar, *endCalendar, flags);
    return formatInterval(*startCalendar, *endCalendar, skeleton);
}

std::string formatDateRange(Context* context, int64_t startMillis, int64_t endMillis,
        int flags) {
    return formatDateRange(context, startMillis, endMillis, flags, std::string());
}

std::string formatDateRange(Context* context, int64_t startMillis, int64_t endMillis,
        int flags, const std::string& timeZone) {
    // If we're being asked to format a time without being explicitly told
    // whether to use the 12- or 24-hour clock, fall back to the user's
    // preference (AOSP comment; icu4c would fall back to the locale's).
    if ((flags & (FORMAT_SHOW_TIME | FORMAT_12HOUR | FORMAT_24HOUR)) == FORMAT_SHOW_TIME) {
        flags |= DateFormat::is24HourFormat(context) ? FORMAT_24HOUR : FORMAT_12HOUR;
    }
    // `timeZone` (olsonId) and FORMAT_UTC: no TimeZone class — the default
    // zone is used (see the section note above).
    (void)timeZone;
    return formatDateRangeCore(startMillis, endMillis, flags);
}

} // namespace DateUtils
}//namespace
