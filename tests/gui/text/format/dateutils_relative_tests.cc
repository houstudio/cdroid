/*********************************************************************************
 * Relative-time tests for DateUtils (getRelativeTimeSpanString /
 * getRelativeDateTimeString). AOSP coretests has no cases for these (they
 * live in CTS); the expectations here follow the AOSP javadoc semantics and
 * the CLDR en word table: "42 minutes ago"/"in 42 minutes"/"yesterday"/
 * "today"/"tomorrow"/"2 days ago" (en has no LAST_2 word, AOSP falls back).
 *
 * The word table is en-only (ICU RelativeDateTimeFormatter stand-in, see
 * text/format/dateutils.cc) — deviations for other locales are recorded.
 *********************************************************************************/
#include <text/format/dateutils.h>
#include <content/dateformat.h>
#include <content/Locale.h>
#include <gtest/gtest.h>
#include <cstdlib>
#include <core/systemclock.h>
#include <ctime>

using namespace cdroid;

// Pin the calendar zone to UTC (the harness' calendars follow localtime).
static const bool kTzUtcPinned2 = (setenv("TZ", "UTC", 1) == 0) && (tzset(), true);

namespace {

constexpr int64_t SECOND = 1000;
constexpr int64_t MINUTE = 60 * SECOND;
constexpr int64_t HOUR = 60 * MINUTE;
constexpr int64_t DAY = 24 * HOUR;
constexpr int64_t WEEK = 7 * DAY;

// 2009-01-19T03:30:15Z (same anchor as the same-day tests).
constexpr int64_t NOW = 1232335815000LL;

TEST(CoreRelativeTimeTest, secondsRange) {
    EXPECT_EQ("42 seconds ago",
            DateUtils::getRelativeTimeSpanString(NOW - 42 * SECOND, NOW, 0, 0));
    EXPECT_EQ("in 42 seconds",
            DateUtils::getRelativeTimeSpanString(NOW + 42 * SECOND, NOW, 0, 0));
    EXPECT_EQ("1 second ago",
            DateUtils::getRelativeTimeSpanString(NOW - 1 * SECOND, NOW, 0, 0));
}

TEST(CoreRelativeTimeTest, minutesRange) {
    EXPECT_EQ("42 minutes ago",
            DateUtils::getRelativeTimeSpanString(NOW - 42 * MINUTE, NOW, MINUTE, 0));
    EXPECT_EQ("in 42 minutes",
            DateUtils::getRelativeTimeSpanString(NOW + 42 * MINUTE, NOW, MINUTE, 0));
    // Sub-minute duration with MINUTE minResolution collapses to "0 minutes ago".
    EXPECT_EQ("0 minutes ago",
            DateUtils::getRelativeTimeSpanString(NOW - 3 * SECOND, NOW, MINUTE, 0));
    EXPECT_EQ("1 minute ago",
            DateUtils::getRelativeTimeSpanString(NOW - 90 * SECOND, NOW, MINUTE, 0));
}

TEST(CoreRelativeTimeTest, hoursRange) {
    EXPECT_EQ("3 hours ago",
            DateUtils::getRelativeTimeSpanString(NOW - 3 * HOUR, NOW, MINUTE, 0));
    EXPECT_EQ("in 3 hours",
            DateUtils::getRelativeTimeSpanString(NOW + 3 * HOUR, NOW, MINUTE, 0));
}

TEST(CoreRelativeTimeTest, dayWords) {
    // Same local day (3.5h apart, both on 2009-01-19 UTC).
    EXPECT_EQ("today",
            DateUtils::getRelativeTimeSpanString(NOW - 3 * HOUR, NOW, DAY, 0));
    // One day apart: "yesterday"/"tomorrow", not "1 day ago".
    EXPECT_EQ("yesterday",
            DateUtils::getRelativeTimeSpanString(NOW - 24 * HOUR, NOW, DAY, 0));
    EXPECT_EQ("tomorrow",
            DateUtils::getRelativeTimeSpanString(NOW + 24 * HOUR, NOW, DAY, 0));
    // Two days: en has no LAST_2 word — the AOSP fallback yields the count form.
    EXPECT_EQ("2 days ago",
            DateUtils::getRelativeTimeSpanString(NOW - 48 * HOUR, NOW, DAY, 0));
    EXPECT_EQ("in 2 days",
            DateUtils::getRelativeTimeSpanString(NOW + 48 * HOUR, NOW, DAY, 0));
}

TEST(CoreRelativeTimeTest, weeksRange) {
    EXPECT_EQ("2 weeks ago",
            DateUtils::getRelativeTimeSpanString(NOW - 2 * WEEK, NOW, WEEK, 0));
    EXPECT_EQ("in 2 weeks",
            DateUtils::getRelativeTimeSpanString(NOW + 2 * WEEK, NOW, WEEK, 0));
}

TEST(CoreRelativeTimeTest, absoluteDateBeyondWeek) {
    // > 1 week with minResolution < WEEK: absolute date. Flags 0 keep the
    // full month name; same year as the anchor elides it, next year shows it.
    // KNOWN DEVIATION (i18n pattern pools): the yMMMMd skeleton comes back
    // with the abbreviated month ("Jan 5" / "Jan 4, 2010") — the full-month
    // pattern pool is not populated; year elision/shown is correct.
    EXPECT_EQ("January 5",
            DateUtils::getRelativeTimeSpanString(NOW - 2 * WEEK, NOW, DAY, 0));
    EXPECT_EQ("January 4, 2010",
            DateUtils::getRelativeTimeSpanString(NOW + 350 * DAY, NOW, DAY, 0));
}

TEST(CoreRelativeTimeTest, abbrevRelative) {
    EXPECT_EQ("42 min. ago",
            DateUtils::getRelativeTimeSpanString(NOW - 42 * MINUTE, NOW, MINUTE,
                    DateUtils::FORMAT_ABBREV_RELATIVE));
    EXPECT_EQ("in 42 min.",
            DateUtils::getRelativeTimeSpanString(NOW + 42 * MINUTE, NOW, MINUTE,
                    DateUtils::FORMAT_ABBREV_RELATIVE));
}

TEST(CoreRelativeTimeTest, relativeDateTimeString) {
    // Inside the transition window: "[relative], [time]". The time clause
    // formats the anchor's clock time (03:40 AM for +10 minutes in 12-hour).
    const std::string s = DateUtils::getRelativeDateTimeString(nullptr,
            SystemClock::currentTimeMillis() + 10 * MINUTE, MINUTE, DAY, 0);
    EXPECT_NE(std::string::npos, s.find(", "));
    EXPECT_NE(std::string::npos, s.find("in 10 minutes"));
}

// ---- formatDateRange (AOSP javadoc examples, en-US) --------------------------
// Anchors in 2007/2009 (UTC). The year shows whenever it isn't the current
// one, so these examples all carry it (matches AOSP for the same anchors).

TEST(CoreFormatDateRangeTest, singleInstant) {
    // 2007-10-09 10:15 UTC.
    const int64_t t = 1191924900000LL;
    EXPECT_EQ("Oct 9, 2007", DateUtils::formatDateRange(nullptr, t, t,
            DateUtils::FORMAT_SHOW_DATE));
    EXPECT_EQ("10:15 AM", DateUtils::formatDateRange(nullptr, t, t,
            DateUtils::FORMAT_SHOW_TIME | DateUtils::FORMAT_12HOUR));
}

TEST(CoreFormatDateRangeTest, timeRange) {
    // 15:00-16:00 on the same day.
    const int64_t day = 1191888000000LL;          // 2007-10-09T00:00Z
    EXPECT_EQ("3:00 \u2013 4:00 PM", DateUtils::formatDateRange(nullptr,
            day + 15 * HOUR, day + 16 * HOUR,
            DateUtils::FORMAT_SHOW_TIME | DateUtils::FORMAT_12HOUR));
}

TEST(CoreFormatDateRangeTest, sameMonthDayRange) {
    // Oct 9-10, 2007.
    const int64_t day = 1191888000000LL;
    EXPECT_EQ("Oct 9 \u2013 10, 2007", DateUtils::formatDateRange(nullptr,
            day + 15 * HOUR, day + 36 * HOUR, DateUtils::FORMAT_SHOW_DATE));
}

TEST(CoreFormatDateRangeTest, midnightEndFudge) {
    // Nov 10 8pm → Nov 12 00:00 with no time shown: the end day rolls back
    // (AOSP javadoc: "Nov 10 – 11").
    const int64_t nov10 = 1194652800000LL;        // 2007-11-10T00:00Z
    EXPECT_EQ("Nov 10 \u2013 11, 2007", DateUtils::formatDateRange(nullptr,
            nov10 + 20 * HOUR, nov10 + 48 * HOUR, DateUtils::FORMAT_SHOW_DATE));
}

TEST(CoreFormatDateRangeTest, crossYearRange) {
    // Dec 31, 2007 – Jan 1, 2008.
    const int64_t dec31 = 1199059200000LL;        // 2007-12-31T00:00Z
    EXPECT_EQ("Dec 31, 2007 \u2013 Jan 1, 2008", DateUtils::formatDateRange(nullptr,
            dec31 + 12 * HOUR, dec31 + 36 * HOUR, DateUtils::FORMAT_SHOW_DATE));
}

} // namespace
