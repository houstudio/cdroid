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
#ifndef __DATE_UTILS_H__
#define __DATE_UTILS_H__
#include <string>
#include <cstdint>

namespace cdroid{

class Context;
class Calendar;

// Port of android.text.format.DateUtils (the in-tree subset): the elapsed-time
// and date-time format helpers plus the FORMAT_* vocabulary. formatDateRange /
// getRelativeTimeSpanString and the @hide bridge plumbing (DateIntervalFormat,
// DateTimeFormat, RelativeDateTimeFormatter) are deliberately not ported — no
// in-tree consumer; formatDateTime covers the single-date flags path through
// the already-ported getBestDateTimePattern.
namespace DateUtils{

// ---- millis constants (AOSP verbatim) --------------------------------------
constexpr int64_t SECOND_IN_MILLIS = 1000;
constexpr int64_t MINUTE_IN_MILLIS = SECOND_IN_MILLIS * 60;
constexpr int64_t HOUR_IN_MILLIS   = MINUTE_IN_MILLIS * 60;
constexpr int64_t DAY_IN_MILLIS    = HOUR_IN_MILLIS * 24;
constexpr int64_t WEEK_IN_MILLIS   = DAY_IN_MILLIS * 7;
constexpr int64_t YEAR_IN_MILLIS   = WEEK_IN_MILLIS * 52;

// ---- FORMAT_* flags (AOSP verbatim) ----------------------------------------
enum {
    FORMAT_SHOW_TIME       = 0x00001,
    FORMAT_SHOW_WEEKDAY    = 0x00002,
    FORMAT_SHOW_YEAR       = 0x00004,
    FORMAT_NO_YEAR         = 0x00008,
    FORMAT_SHOW_DATE       = 0x00010,
    FORMAT_NO_MONTH_DAY    = 0x00020,
    FORMAT_12HOUR          = 0x00040,
    FORMAT_24HOUR          = 0x00080,
    FORMAT_CAP_AMPM        = 0x00100,
    FORMAT_NO_NOON         = 0x00200,
    FORMAT_CAP_NOON        = 0x00400,
    FORMAT_NO_MIDNIGHT     = 0x00800,
    FORMAT_CAP_MIDNIGHT    = 0x01000,
    FORMAT_UTC             = 0x02000,
    FORMAT_ABBREV_TIME     = 0x04000,
    FORMAT_ABBREV_WEEKDAY  = 0x08000,
    FORMAT_ABBREV_MONTH    = 0x10000,
    FORMAT_NUMERIC_DATE    = 0x20000,
    FORMAT_ABBREV_RELATIVE = 0x40000,
    FORMAT_ABBREV_ALL      = 0x80000,
};

// ---- LENGTH_* selector for the name getters (AOSP verbatim) ----------------
enum {
    LENGTH_LONG     = 10,
    LENGTH_MEDIUM   = 20,
    LENGTH_SHORT    = 30,
    LENGTH_SHORTER  = 40,
    LENGTH_SHORTEST = 50,
};

// ---- name getters (AOSP :206/:234/:251, data from DateFormatSymbols) -------
std::string getDayOfWeekString(int dayOfWeek, int abbrev);
std::string getAMPMString(int ampm);
std::string getMonthString(int month, int abbrev);

// ---- elapsed time (AOSP formatElapsedTime) ---------------------------------
// Formats like "MM:SS" / "H:MM:SS" suited to the current locale. `recycle`
// mirrors AOSP's StringBuilder recycle (null → fresh string).
std::string formatElapsedTime(int64_t elapsedSeconds);
std::string formatElapsedTime(std::string* recycle, int64_t elapsedSeconds);

// ---- date-time (AOSP formatDateTime = same-millis range path) --------------
// flags→skeleton (DateUtilsBridge.toSkeleton) → getBestDateTimePattern →
// SimpleDateFormat. Only flags valid for a single instant apply (noon/midnight
// special-casing is range machinery and not reachable here).
std::string formatDateTime(Context* context, int64_t millis, int flags);

} // namespace DateUtils
}//namespace
#endif/*__DATE_UTILS_H__*/
