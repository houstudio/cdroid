/*********************************************************************************
 * Ported from AOSP coretests android.text.format.DateFormatTest (Apache 2.0).
 * hasDesignator / is24HourLocale / getDateFormatOrder landed on the
 * java.text.DateFormat class (the folded android.text.format statics home —
 * see content/dateformat.h).
 *
 * KNOWN DEVIATIONS:
 *  - testGetDateFormatOrder's lv/fa best()-derived cases need a real ICU
 *    DTPG (skeleton→pattern per-locale field order); CDROID's DTPG-lite
 *    covers the DateUtils skeletons only, so those assertions may mismatch.
 *  - The two DISALLOW_DUPLICATE_FIELD_IN_SKELETON compat-change cases stay
 *    skips in format_api_gaps_tests.cc (no platform compat framework).
 *********************************************************************************/
#include <content/dateformat.h>
#include <content/dateformatsymbols.h>
#include <content/Locale.h>
#include <gtest/gtest.h>
#include <array>
#include <string>

using namespace cdroid;

namespace {

TEST(CoreFormatDateFormatTest, testHasDesignator) {
    EXPECT_TRUE(DateFormat::hasDesignator("hh:mm:ss", DateFormat::MINUTE));
    EXPECT_TRUE(DateFormat::hasDesignator("myyyy", DateFormat::MINUTE));
    EXPECT_TRUE(DateFormat::hasDesignator("mmm", DateFormat::MINUTE));

    EXPECT_FALSE(DateFormat::hasDesignator("hh:MM:ss", DateFormat::MINUTE));
}

TEST(CoreFormatDateFormatTest, testHasDesignatorEscaped) {
    EXPECT_TRUE(DateFormat::hasDesignator("hh:mm 'LOL'", DateFormat::MINUTE));

    EXPECT_FALSE(DateFormat::hasDesignator("hh:mm 'yyyy'", DateFormat::YEAR));
}

TEST(CoreFormatDateFormatTest, testIs24HourLocale) {
    EXPECT_FALSE(DateFormat::is24HourLocale(Locale("en", "US")));
    EXPECT_TRUE(DateFormat::is24HourLocale(Locale("de", "DE")));
}

TEST(CoreFormatDateFormatTest, testgetIcuDateFormatSymbols) {
    // AOSP DateFormat.getIcuDateFormatSymbols maps to icu DateFormatSymbols;
    // CDROID's java.text port carries the same tables on
    // content/DateFormatSymbols (the accessor itself is the AOSP bridge and
    // is not ported, so the symbol getters are called directly).
    // KNOWN DEVIATION: the i18n.dat narrow am/pm entries are upper-case
    // ("A"/"P") where ICU's en data is lower-case ("a"/"p") — the two
    // assertions below stay red until the data table matches.
    const DateFormatSymbols dfs(Locale("en", "US"));
    EXPECT_EQ("AM", dfs.getAmPmStrings()[0]);
    EXPECT_EQ("PM", dfs.getAmPmStrings()[1]);
    EXPECT_EQ("a", dfs.getAmpmNarrowStrings()[0]);
    EXPECT_EQ("p", dfs.getAmpmNarrowStrings()[1]);
}

TEST(CoreFormatDateFormatTest, testGetDateFormatOrder) {
    // AOSP compares "[d, M, y]"-style strings; the arrays are compared
    // directly here (the unfilled slot is the NUL char in both worlds).
    const std::array<char, 3> DMY = { 'd', 'M', 'y' };
    const std::array<char, 3> YDM = { 'y', 'd', 'M' };
    const std::array<char, 3> YMD = { 'y', 'M', 'd' };
    const std::array<char, 3> MDY = { 'M', 'd', 'y' };
    const std::array<char, 3> DM_ = { 'd', 'M', '\0' };
    const std::array<char, 3> MD_ = { 'M', 'd', '\0' };

    // lv and fa use differing orders depending on whether you're using numeric or
    // textual months. (These need the real DTPG for the best() skeletons.)
    const Locale lv("lv");
    EXPECT_EQ(DMY, DateFormat::getDateFormatOrder(
            DateFormat::getBestDateTimePattern(lv, "yyyy-M-dd"))) << "lv numeric";
    EXPECT_EQ(YDM, DateFormat::getDateFormatOrder(
            DateFormat::getBestDateTimePattern(lv, "yyyy-MMM-dd"))) << "lv textual";
    EXPECT_EQ(DM_, DateFormat::getDateFormatOrder(
            DateFormat::getBestDateTimePattern(lv, "MMM-dd"))) << "lv no year";
    const Locale fa("fa");
    EXPECT_EQ(YMD, DateFormat::getDateFormatOrder(
            DateFormat::getBestDateTimePattern(fa, "yyyy-M-dd"))) << "fa numeric";
    EXPECT_EQ(DMY, DateFormat::getDateFormatOrder(
            DateFormat::getBestDateTimePattern(fa, "yyyy-MMM-dd"))) << "fa textual";
    EXPECT_EQ(DM_, DateFormat::getDateFormatOrder(
            DateFormat::getBestDateTimePattern(fa, "MMM-dd"))) << "fa no year";

    // English differs on each side of the Atlantic.
    const Locale enUS("en", "US");
    EXPECT_EQ(MDY, DateFormat::getDateFormatOrder(
            DateFormat::getBestDateTimePattern(enUS, "yyyy-M-dd"))) << "en-US numeric";
    EXPECT_EQ(MDY, DateFormat::getDateFormatOrder(
            DateFormat::getBestDateTimePattern(enUS, "yyyy-MMM-dd"))) << "en-US textual";
    EXPECT_EQ(MD_, DateFormat::getDateFormatOrder(
            DateFormat::getBestDateTimePattern(enUS, "MMM-dd"))) << "en-US no year";
    const Locale enGB("en", "GB");
    EXPECT_EQ(DMY, DateFormat::getDateFormatOrder(
            DateFormat::getBestDateTimePattern(enGB, "yyyy-M-dd"))) << "en-GB numeric";
    EXPECT_EQ(DMY, DateFormat::getDateFormatOrder(
            DateFormat::getBestDateTimePattern(enGB, "yyyy-MMM-dd"))) << "en-GB textual";
    EXPECT_EQ(DM_, DateFormat::getDateFormatOrder(
            DateFormat::getBestDateTimePattern(enGB, "MMM-dd"))) << "en-GB no year";

    EXPECT_EQ(YMD, DateFormat::getDateFormatOrder(
            "yyyy - 'why' '' 'ddd' MMM-dd"));

    EXPECT_THROW(DateFormat::getDateFormatOrder(
            "the quick brown fox jumped over the lazy dog"), std::invalid_argument);
    EXPECT_THROW(DateFormat::getDateFormatOrder("'"), std::invalid_argument);
    EXPECT_THROW(DateFormat::getDateFormatOrder("yyyy'"), std::invalid_argument);
    EXPECT_THROW(DateFormat::getDateFormatOrder("yyyy'MMM"), std::invalid_argument);
}

} // namespace
