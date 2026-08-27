/*********************************************************************************
 * AOSP coretests android.text.format.* port — API-gap records.
 *
 * The android.text.format package is only partially ported (see
 * src/gui/content/dateutils.h / dateformat.h): DateUtils has the elapsed-time
 * and formatDateTime paths, DateFormat has is24HourFormat/hasSeconds/format/
 * getBestDateTimePattern (no DTPG engine). Everything these seven AOSP test
 * files exercise is in the un-ported remainder, so the suites are recorded
 * here as skips, one per AOSP file (long files collapsed to one skip with the
 * full method list in the message), following the text_api_gaps_tests.cc
 * convention: the gap becomes executable bookkeeping, and the file to extend
 * when the framework side lands.
 *
 * Two tiers among the skips: (a) APIs deliberately excluded from the
 * DateUtils/DateFormat in-tree subsets (dateutils.h / dateformat.h name them
 * — no in-tree consumer, no DTPG engine); (b) whole un-ported classes
 * (Formatter, Time, TimeMigrationUtils). Tier matters when triaging: (a) is a
 * scope decision to revisit, (b) is plain missing surface.
 *********************************************************************************/
#include <gtest/gtest.h>

namespace {

// ---- DateUtilsTest.java (4 cases) ------------------------------------------
// Ported for real in dateutils_tests.cc: formatDuration (the en-US unit table
// for the ICU MeasureFormat widths) and formatSameDayTime (Calendar compare +
// java.text factories; the zone-name spellings of FULL/LONG time styles are
// the no-TimeZone-class deviation candidates).

// ---- DateFormatTest.java (7 cases) -----------------------------------------
// hasDesignator/hasDesignatorEscaped/is24HourLocale/getIcuDateFormatSymbols/
// getDateFormatOrder are ported for real in dateformat_tests.cc (the statics
// live on the folded java.text.DateFormat class — see content/dateformat.h).
// Still skipped here: the two DISALLOW_DUPLICATE_FIELD_IN_SKELETON
// compat-change cases (no platform compat framework, no full DTPG).
TEST(CoreFormatDateFormatTest, testGetBestDateTimePattern_disableDuplicateField) {
    GTEST_SKIP() << "DISALLOW_DUPLICATE_FIELD_IN_SKELETON compat change + DTPG not ported";
}
TEST(CoreFormatDateFormatTest, testGetBestDateTimePattern_enableDuplicateField) {
    GTEST_SKIP() << "getBestDateTimePattern has no DTPG engine (hm/Hm only)";
}

// ---- FormatterTest.java (5 cases) -------------------------------------------
// Ported for real in formatter_tests.cc (android.text.format.Formatter landed
// in text/format/): testFormatBytes/Si/Iec pass; the FR/RU locale variants of
// the two elapsed-time cases stay red (en-US unit-word table only — ICU
// MeasureFormat is not ported).

// ---- TimeTest.java (33 cases: normalize/switchTimezone/ctor/
//      getActualMaximum/clear/compare/format/parse/... the android.text.format
//      Time calendar class) ---------------------------------------------------
TEST(CoreFormatTimeTest, Time_not_ported) {
    GTEST_SKIP() << "android.text.format.Time (33 AOSP cases: testNormalize0/1, "
                    "testSwitchTimezone0, testCtor0, testGetActualMaximum0, testClear0, "
                    "testCompare0, testFormat0, ...) not ported";
}

// ---- DateIntervalFormatTest.java (32 cases: test_formatDateInterval,
//      test8862241, test10089890, test10318326, test10560853_*, ...) ----------
// Deliberately excluded from the DateUtils in-tree subset (dateutils.h names
// the @hide DateIntervalFormat bridge plumbing as not ported — no in-tree
// consumer).
TEST(CoreFormatDateIntervalFormatTest, DateIntervalFormat_not_ported) {
    GTEST_SKIP() << "deliberately excluded from the in-tree subset (dateutils.h): "
                    "android.icu-based DateIntervalFormat bridge (32 AOSP cases: "
                    "test_formatDateInterval, test8862241, test10089890, ...)";
}

// ---- RelativeDateTimeFormatterTest.java (28 cases:
//      test_getRelativeTimeSpanString*, test_getRelativeDateTimeString*, ...) --
// Deliberately excluded from the DateUtils in-tree subset (dateutils.h names
// getRelativeTimeSpanString / RelativeDateTimeFormatter as not ported — no
// in-tree consumer).
TEST(CoreFormatRelativeDateTimeFormatterTest, RelativeDateTimeFormatter_not_ported) {
    GTEST_SKIP() << "deliberately excluded from the in-tree subset (dateutils.h): "
                    "RelativeDateTimeFormatter / DateUtils.getRelativeTimeSpanString "
                    "(28 AOSP cases)";
}

// ---- TimeMigrationUtilsTest.java (2 cases:
//      formatMillisWithFixedFormat_fixes2038Issue,
//      formatMillisAsDateTime_matchesOldBehavior) -----------------------------
TEST(CoreFormatTimeMigrationUtilsTest, formatMillisWithFixedFormat_fixes2038Issue) {
    GTEST_SKIP() << "android.text.format.TimeMigrationUtils not ported";
}
TEST(CoreFormatTimeMigrationUtilsTest, formatMillisAsDateTime_matchesOldBehavior) {
    GTEST_SKIP() << "TimeMigrationUtils not ported";
}

} // namespace
