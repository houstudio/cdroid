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
#include <memory>

namespace cdroid{
namespace DateUtils{

// ---- name getters (AOSP :206/:234/:251) ------------------------------------
// LENGTH_SHORTEST→tiny, LENGTH_SHORT/MEDIUM/SHORTER→short, else wide (the
// CDROID DateFormatSymbols three-tier; AOSP's MEDIUM maps through ICU's
// abbreviated tier, which our short tier carries).
std::string getDayOfWeekString(int dayOfWeek, int abbrev) {
    const DateFormatSymbols dfs(Locale::getDefault());
    const std::vector<std::string>& names = (abbrev == LENGTH_SHORTEST) ? dfs.getTinyWeekdays()
            : (abbrev >= LENGTH_SHORT) ? dfs.getShortWeekdays() : dfs.getWeekdays();
    // Calendar weekday indices are 1-based (SUNDAY..SATURDAY) over an array
    // whose slot 0 is the empty leading entry.
    if (dayOfWeek < 1 || dayOfWeek >= (int)names.size()) return std::string();
    return names[dayOfWeek];
}

std::string getAMPMString(int ampm) {
    const DateFormatSymbols dfs(Locale::getDefault());
    const std::vector<std::string>& names = dfs.getAmPmStrings();
    if (ampm < 0 || ampm >= (int)names.size()) return std::string();
    return names[ampm];
}

std::string getMonthString(int month, int abbrev) {
    const DateFormatSymbols dfs(Locale::getDefault());
    const std::vector<std::string>& names = (abbrev == LENGTH_SHORTEST) ? dfs.getTinyMonths()
            : (abbrev >= LENGTH_SHORT) ? dfs.getShortMonths() : dfs.getMonths();
    if (month < 0 || month >= (int)names.size()) return std::string();
    return names[month];
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
        static std::string tag;
        static std::unique_ptr<i18n::DateTimeFormat> cache;
        const std::string cur = Locale::getDefault().toLanguageTag();
        if (tag != cur || cache == nullptr) {
            tag = cur;
            i18n::LocaleInfo info = I18nBridge::toLocaleInfo(Locale::getDefault());
            cache.reset(new i18n::DateTimeFormat(i18n::AvailableDateTimeFormatPattern::FULL, info));
        }
        i18n::I18nStatus status = i18n::I18nStatus::ISUCCESS;
        // The engine takes milliseconds; H:MM:SS when hours are present,
        // MM:SS otherwise (AOSP's two format strings).
        out = cache->FormatElapsedDuration(
                (int32_t)((hours * 3600 + minutes * 60 + seconds) * 1000),
                (hours > 0) ? i18n::ELAPSED_HOUR_MINUTE_SECOND : i18n::ELAPSED_MINUTE_SECOND,
                status);
        if (status == i18n::I18nStatus::ISUCCESS && !out.empty()) {
            if (recycle) { *recycle = out; return *recycle; }
            return out;
        }
    }
#endif
    // No-i18n / engine-miss fallback: the plain "%d:%02d:%02d" / "%02d:%02d".
    char buf[32];
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

// AOSP DateUtilsBridge.isThisYear.
static bool isThisYear(Calendar& c) {
    auto now = Calendar::getInstance(Locale::getDefault());
    return now->get(Calendar::YEAR) == c.get(Calendar::YEAR);
}

// AOSP DateUtilsBridge.toSkeleton, single-calendar form (start == end, so
// every fallOnDifferent*/fallInSame* check is trivially resolved).
static std::string toSkeleton(Calendar& calendar, int flags) {
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

    // Not abbreviating, or 24-hour: include minutes ("4 PM", never "16").
    if ((flags & FORMAT_ABBREV_TIME) == 0 || (flags & FORMAT_24HOUR) != 0) {
        timePart += "m";
    } else if (onTheHour(calendar)) {
        // Abbreviated 12-hour on the hour: no minutes.
    } else {
        timePart += "m";
    }

    if ((flags & (FORMAT_SHOW_DATE | FORMAT_SHOW_TIME | FORMAT_SHOW_WEEKDAY)) == 0) {
        flags |= FORMAT_SHOW_DATE;
    }

    // Show the year? Explicit SHOW_YEAR/NO_YEAR wins; else this-year elision.
    if ((flags & FORMAT_SHOW_DATE) != 0) {
        if ((flags & FORMAT_SHOW_YEAR) != 0) {
            // The caller explicitly wants us to show the year.
        } else if ((flags & FORMAT_NO_YEAR) != 0) {
            // The caller explicitly doesn't want the year.
        } else if (!isThisYear(calendar)) {
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

std::string formatDateTime(Context* /*context*/, int64_t millis, int flags) {
    auto calendar = Calendar::getInstance(Locale::getDefault());
    calendar->setTimeInMillis(millis);
    const std::string skeleton = toSkeleton(*calendar, flags);
    const Locale locale = Locale::getDefault();
    const std::string pattern = DateFormat::getBestDateTimePattern(locale, skeleton);
    SimpleDateFormat formatter(pattern, locale);
    return formatter.format(calendar->getTimeInMillis());
}

// ---- duration (AOSP formatDuration, DateUtils.java:384/:400) ----------------

// ICU MeasureFormat WIDE/SHORT/NARROW stand-in for the duration units, same
// en-US unit-word convention as Formatter's elapsed-time table (see
// formatter.cc): non-en locales get the English units until an i18n
// measure-word table exists.
static std::string durationMeasure(int64_t value, int width,
        const char* wideSingle, const char* widePlural,
        const char* shortUnit, const char* narrowUnit) {
    char buf[48];
    switch (width) {
        case 0:  // WIDE
            snprintf(buf, sizeof(buf), "%lld %s", (long long)value,
                    value == 1 ? wideSingle : widePlural);
            break;
        case 1:  // SHORT
            snprintf(buf, sizeof(buf), "%lld %s", (long long)value, shortUnit);
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
        return durationMeasure(hours, width, "hour", "hours", "hr", "h");
    } else if (millis >= MINUTE_IN_MILLIS) {
        const int minutes = (int)((millis + 30000) / MINUTE_IN_MILLIS);
        return durationMeasure(minutes, width, "minute", "minutes", "min", "m");
    } else {
        const int seconds = (int)((millis + 500) / SECOND_IN_MILLIS);
        return durationMeasure(seconds, width, "second", "seconds", "sec", "s");
    }
}

// ---- same-day (AOSP formatSameDayTime, DateUtils.java:499) ------------------

std::string formatSameDayTime(int64_t then, int64_t now, int dateStyle, int timeStyle) {
    auto thenCal = Calendar::getInstance(Locale::getDefault());
    thenCal->setTimeInMillis(then);
    auto nowCal = Calendar::getInstance(Locale::getDefault());
    nowCal->setTimeInMillis(now);

    // AOSP getTimeInstance for the same day, getDateInstance otherwise; the
    // factories return a new DateFormat the caller owns.
    DateFormat* f;
    if (thenCal->get(Calendar::YEAR) == nowCal->get(Calendar::YEAR)
            && thenCal->get(Calendar::MONTH) == nowCal->get(Calendar::MONTH)
            && thenCal->get(Calendar::DAY_OF_MONTH) == nowCal->get(Calendar::DAY_OF_MONTH)) {
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
    auto one = Calendar::getInstance(Locale::getDefault());
    one->setTimeInMillis(oneMillis);
    auto two = Calendar::getInstance(Locale::getDefault());
    two->setTimeInMillis(twoMillis);
    return one->get(Calendar::YEAR) == two->get(Calendar::YEAR)
            && one->get(Calendar::MONTH) == two->get(Calendar::MONTH)
            && one->get(Calendar::DAY_OF_MONTH) == two->get(Calendar::DAY_OF_MONTH);
}

bool isToday(int64_t when) {
    return isSameDate(when, SystemClock::currentTimeMillis());
}

} // namespace DateUtils
}//namespace
