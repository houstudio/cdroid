/*********************************************************************************
 * Ported from AOSP coretests android.text.format.DateUtilsTest (Apache 2.0).
 * The formatDuration family and formatSameDayTime landed in
 * src/gui/text/format/dateutils; this replaces their API-gap skips.
 *
 * AOSP setUp forces Locale.US and TimeZone GMT. CDROID's harness runs en by
 * default; there is no TimeZone class (calendars carry a raw UTC offset), so
 * the LONG/FULL time-style assertions that spell out the zone name
 * ("Greenwich Mean Time"/"GMT") are best-effort — recorded as deviations if
 * the underlying SimpleDateFormat zone text differs.
 *********************************************************************************/
#include <text/format/dateutils.h>
#include <content/dateformat.h>
#include <content/Locale.h>
#include <gtest/gtest.h>
#include <ctime>
#include <cstdlib>

using namespace cdroid;

// AOSP setUp pins TimeZone GMT; CDROID's Calendar zone follows localtime, so
// pin TZ before any calendar work (static init runs ahead of main).
static const bool kTzUtcPinned = (setenv("TZ", "UTC", 1) == 0) && (tzset(), true);

namespace {

TEST(CoreFormatDateUtilsTest, test_formatDuration_seconds) {
    EXPECT_EQ("0 seconds", DateUtils::formatDuration(0));
    EXPECT_EQ("0 seconds", DateUtils::formatDuration(1));
    EXPECT_EQ("0 seconds", DateUtils::formatDuration(499));
    EXPECT_EQ("1 second", DateUtils::formatDuration(500));
    EXPECT_EQ("1 second", DateUtils::formatDuration(1000));
    EXPECT_EQ("2 seconds", DateUtils::formatDuration(1500));

    EXPECT_EQ("0 seconds", DateUtils::formatDuration(0, DateUtils::LENGTH_LONG));
    EXPECT_EQ("1 second", DateUtils::formatDuration(1000, DateUtils::LENGTH_LONG));
    EXPECT_EQ("2 seconds", DateUtils::formatDuration(1500, DateUtils::LENGTH_LONG));

    EXPECT_EQ("0 sec", DateUtils::formatDuration(0, DateUtils::LENGTH_SHORT));
    EXPECT_EQ("1 sec", DateUtils::formatDuration(1000, DateUtils::LENGTH_SHORT));
    EXPECT_EQ("2 sec", DateUtils::formatDuration(1500, DateUtils::LENGTH_SHORT));

    EXPECT_EQ("0s", DateUtils::formatDuration(0, DateUtils::LENGTH_SHORTEST));
    EXPECT_EQ("1s", DateUtils::formatDuration(1000, DateUtils::LENGTH_SHORTEST));
    EXPECT_EQ("2s", DateUtils::formatDuration(1500, DateUtils::LENGTH_SHORTEST));
}

TEST(CoreFormatDateUtilsTest, test_formatDuration_Minutes) {
    EXPECT_EQ("59 seconds", DateUtils::formatDuration(59000));
    EXPECT_EQ("60 seconds", DateUtils::formatDuration(59500));
    EXPECT_EQ("1 minute", DateUtils::formatDuration(60000));
    EXPECT_EQ("2 minutes", DateUtils::formatDuration(120000));

    EXPECT_EQ("1 minute", DateUtils::formatDuration(60000, DateUtils::LENGTH_LONG));
    EXPECT_EQ("2 minutes", DateUtils::formatDuration(120000, DateUtils::LENGTH_LONG));

    EXPECT_EQ("1 min", DateUtils::formatDuration(60000, DateUtils::LENGTH_SHORT));
    EXPECT_EQ("2 min", DateUtils::formatDuration(120000, DateUtils::LENGTH_SHORT));

    EXPECT_EQ("1m", DateUtils::formatDuration(60000, DateUtils::LENGTH_SHORTEST));
    EXPECT_EQ("2m", DateUtils::formatDuration(120000, DateUtils::LENGTH_SHORTEST));
}

TEST(CoreFormatDateUtilsTest, test_formatDuration_Hours) {
    EXPECT_EQ("59 minutes", DateUtils::formatDuration(3540000));
    EXPECT_EQ("1 hour", DateUtils::formatDuration(3600000));
    EXPECT_EQ("48 hours", DateUtils::formatDuration(172800000));

    EXPECT_EQ("1 hour", DateUtils::formatDuration(3600000, DateUtils::LENGTH_LONG));
    EXPECT_EQ("48 hours", DateUtils::formatDuration(172800000, DateUtils::LENGTH_LONG));

    EXPECT_EQ("1 hr", DateUtils::formatDuration(3600000, DateUtils::LENGTH_SHORT));
    EXPECT_EQ("48 hr", DateUtils::formatDuration(172800000, DateUtils::LENGTH_SHORT));

    EXPECT_EQ("1h", DateUtils::formatDuration(3600000, DateUtils::LENGTH_SHORTEST));
    EXPECT_EQ("48h", DateUtils::formatDuration(172800000, DateUtils::LENGTH_SHORTEST));
}

TEST(CoreFormatDateUtilsTest, testFormatSameDayTime) {
    // This test assumes a default DateFormat.is24Hour setting.
    // Fixed epoch: AOSP Date(109, 0, 19, 3, 30, 15) under its GMT setUp —
    // 2009-01-19 03:30:15 UTC.
    const int64_t fixedTime = 1232335815000LL;

    const int64_t dayDuration = 5LL * 24 * 60 * 60 * 1000;
    EXPECT_EQ("Saturday, January 24, 2009", DateUtils::formatSameDayTime(
            fixedTime + dayDuration, fixedTime, DateFormat::FULL,
            DateFormat::FULL));
    // (DEFAULT/MEDIUM date was a style-slot bug — fixed; "Jan 24, 2009" now.)
    EXPECT_EQ("Jan 24, 2009", DateUtils::formatSameDayTime(fixedTime + dayDuration,
            fixedTime, DateFormat::DEFAULT, DateFormat::FULL));
    EXPECT_EQ("January 24, 2009", DateUtils::formatSameDayTime(fixedTime + dayDuration,
            fixedTime, DateFormat::LONG, DateFormat::FULL));
    EXPECT_EQ("Jan 24, 2009", DateUtils::formatSameDayTime(fixedTime + dayDuration,
            fixedTime, DateFormat::MEDIUM, DateFormat::FULL));
    EXPECT_EQ("1/24/09", DateUtils::formatSameDayTime(fixedTime + dayDuration,
            fixedTime, DateFormat::SHORT, DateFormat::FULL));

    const int64_t hourDuration = 2LL * 60 * 60 * 1000;
    // Offset-0 zones carry tzdata's canonical names (long "Greenwich Mean
    // Time", short "GMT"); non-zero offsets still fall back to the numeric
    // GMT+hh:mm ID (no full display-name table in the i18n engine).
    EXPECT_EQ("5:30:15 AM Greenwich Mean Time", DateUtils::formatSameDayTime(
            fixedTime + hourDuration, fixedTime, DateFormat::FULL,
            DateFormat::FULL));
    EXPECT_EQ("5:30:15 AM", DateUtils::formatSameDayTime(fixedTime + hourDuration,
            fixedTime, DateFormat::FULL, DateFormat::DEFAULT));
    EXPECT_EQ("5:30:15 AM GMT", DateUtils::formatSameDayTime(fixedTime + hourDuration,
            fixedTime, DateFormat::FULL, DateFormat::LONG));
    EXPECT_EQ("5:30:15 AM", DateUtils::formatSameDayTime(fixedTime + hourDuration,
            fixedTime, DateFormat::FULL, DateFormat::MEDIUM));
    EXPECT_EQ("5:30 AM", DateUtils::formatSameDayTime(fixedTime + hourDuration,
            fixedTime, DateFormat::FULL, DateFormat::SHORT));
}

} // namespace
