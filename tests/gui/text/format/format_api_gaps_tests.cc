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
 *********************************************************************************/
#include <gtest/gtest.h>

namespace {

// ---- DateUtilsTest.java (4 cases) ------------------------------------------
// formatDuration(millis[, LENGTH_*]) — the "1 second"/"1 min"/"1h" relative
// duration strings — is not ported (formatElapsedTime's "MM:SS" clock format
// is a different API and has no coretests coverage). formatSameDayTime is not
// ported (java.text.DateFormat style constants exist, the API does not).
TEST(CoreFormatDateUtilsTest, test_formatDuration_seconds) {
    GTEST_SKIP() << "android.text.format.DateUtils.formatDuration not ported";
}
TEST(CoreFormatDateUtilsTest, test_formatDuration_Minutes) {
    GTEST_SKIP() << "formatDuration not ported";
}
TEST(CoreFormatDateUtilsTest, test_formatDuration_Hours) {
    GTEST_SKIP() << "formatDuration not ported";
}
TEST(CoreFormatDateUtilsTest, testFormatSameDayTime) {
    GTEST_SKIP() << "DateUtils.formatSameDayTime not ported";
}

// ---- DateFormatTest.java (7 cases) -----------------------------------------
// hasDesignator/Char constants, is24HourLocale, getDateFormatOrder and
// getIcuDateFormatSymbols are not ported. getBestDateTimePattern exists but
// without the ICU DTPG engine (documented gap in content/dateformat.h:36), so
// the skeleton-order tests cannot assert AOSP values.
TEST(CoreFormatDateFormatTest, testHasDesignator) {
    GTEST_SKIP() << "android.text.format.DateFormat.hasDesignator not ported";
}
TEST(CoreFormatDateFormatTest, testHasDesignatorEscaped) {
    GTEST_SKIP() << "hasDesignator not ported";
}
TEST(CoreFormatDateFormatTest, testIs24HourLocale) {
    GTEST_SKIP() << "DateFormat.is24HourLocale not ported";
}
TEST(CoreFormatDateFormatTest, testgetIcuDateFormatSymbols) {
    GTEST_SKIP() << "DateFormat.getIcuDateFormatSymbols not ported";
}
TEST(CoreFormatDateFormatTest, testGetDateFormatOrder) {
    GTEST_SKIP() << "DateFormat.getDateFormatOrder not ported (needs DTPG)";
}
TEST(CoreFormatDateFormatTest, testGetBestDateTimePattern_disableDuplicateField) {
    GTEST_SKIP() << "DISALLOW_DUPLICATE_FIELD_IN_SKELETON compat change + DTPG not ported";
}
TEST(CoreFormatDateFormatTest, testGetBestDateTimePattern_enableDuplicateField) {
    GTEST_SKIP() << "getBestDateTimePattern has no DTPG engine (hm/Hm only)";
}

// ---- FormatterTest.java (5 cases: testFormatBytes, testFormatBytesSi,
//      testFormatBytesIec, testFormatShortElapsedTime,
//      testFormatShortElapsedTimeRoundingUpToMinutes) -------------------------
TEST(CoreFormatFormatterTest, testFormatBytes) {
    GTEST_SKIP() << "android.text.format.Formatter.formatFileSize/formatBytes not ported";
}
TEST(CoreFormatFormatterTest, testFormatBytesSi) {
    GTEST_SKIP() << "Formatter.formatBytes not ported";
}
TEST(CoreFormatFormatterTest, testFormatBytesIec) {
    GTEST_SKIP() << "Formatter.formatBytes not ported";
}
TEST(CoreFormatFormatterTest, testFormatShortElapsedTime) {
    GTEST_SKIP() << "Formatter.formatShortElapsedTime not ported";
}
TEST(CoreFormatFormatterTest, testFormatShortElapsedTimeRoundingUpToMinutes) {
    GTEST_SKIP() << "Formatter.formatShortElapsedTime not ported";
}

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
TEST(CoreFormatDateIntervalFormatTest, DateIntervalFormat_not_ported) {
    GTEST_SKIP() << "android.icu-based DateIntervalFormat bridge (32 AOSP cases: "
                    "test_formatDateInterval, test8862241, test10089890, ...) not ported";
}

// ---- RelativeDateTimeFormatterTest.java (28 cases:
//      test_getRelativeTimeSpanString*, test_getRelativeDateTimeString*, ...) --
TEST(CoreFormatRelativeDateTimeFormatterTest, RelativeDateTimeFormatter_not_ported) {
    GTEST_SKIP() << "RelativeDateTimeFormatter / DateUtils.getRelativeTimeSpanString "
                    "(28 AOSP cases) not ported";
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
