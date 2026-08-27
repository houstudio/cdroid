/*********************************************************************************
 * Ported from AOSP coretests android.text.format.FormatterTest (Apache 2.0).
 * Exercises Formatter::formatBytes (the rounding ladder + per-locale number
 * format) and the short-elapsed-time ladder.
 *
 * AOSP drives the locale through Resources.updateConfiguration; CDROID's
 * Formatter reads Locale::getDefault() (see src/gui/text/format/formatter.h),
 * so the fixture swaps Locale::setDefault instead and restores it after.
 *
 * KNOWN DEVIATIONS (red, recorded): formatShortElapsedTime's unit words are
 * the inline en-US table (ICU MeasureFormat is not ported) — the FR
 * ("2 j") and RU ("1 мин") assertions fail until an i18n measure-word
 * table exists.
 *********************************************************************************/
#include <text/format/formatter.h>
#include <content/Locale.h>
#include <content/LocaleList.h>
#include <view/configuration.h>
#include <core/app.h>
#include <gtest/gtest.h>
#include <string>

using namespace cdroid;

namespace {

constexpr int64_t SECOND = 1000;
constexpr int64_t MINUTE = 60 * SECOND;
constexpr int64_t HOUR = 60 * MINUTE;
constexpr int64_t DAY = 24 * HOUR;

class FormatterTest : public testing::Test {
protected:
    void SetUp() override {
        mOriginalLocale = Locale::getDefault();
    }

    void TearDown() override {
        Locale::setDefault(mOriginalLocale);
    }

    void setLocale(const Locale& locale) {
        // AOSP FormatterTest.setLocale: update the Resources configuration
        // (what formatBytes reads) and Locale.setDefault.
        Configuration config = App::getInstance().getResources().getConfiguration();
        config.setLocales(LocaleList(locale, nullptr));
        App::getInstance().getResources().updateConfiguration(&config, nullptr);
        Locale::setDefault(locale);
    }

    void checkFormatBytes(int64_t bytes, bool useShort,
            const std::string& expectedString, int64_t expectedRounded) {
        checkFormatBytes(bytes, (useShort ? Formatter::FLAG_SHORTER : 0),
                expectedString, expectedRounded);
    }

    void checkFormatBytes(int64_t bytes, int flags,
            const std::string& expectedString, int64_t expectedRounded) {
        const Formatter::BytesResult r = Formatter::formatBytes(
                App::getInstance().getResources(), bytes,
                Formatter::FLAG_CALCULATE_ROUNDED | flags);
        EXPECT_EQ(expectedString, r.value);
        EXPECT_EQ(expectedRounded, r.roundedBytes);
    }

    Locale mOriginalLocale;
};

TEST_F(FormatterTest, testFormatBytes) {
    setLocale(Locale("en", "US"));

    checkFormatBytes(0, true, "0", 0);
    checkFormatBytes(0, false, "0", 0);

    checkFormatBytes(1, true, "1", 1);
    checkFormatBytes(1, false, "1", 1);

    checkFormatBytes(12, true, "12", 12);
    checkFormatBytes(12, false, "12", 12);

    checkFormatBytes(123, true, "123", 123);
    checkFormatBytes(123, false, "123", 123);

    checkFormatBytes(900, true, "900", 900);
    checkFormatBytes(900, false, "900", 900);

    checkFormatBytes(901, true, "0.90", 900);
    checkFormatBytes(901, false, "0.90", 900);

    checkFormatBytes(912, true, "0.91", 910);
    checkFormatBytes(912, false, "0.91", 910);

    checkFormatBytes(9123, true, "9.1", 9100);
    checkFormatBytes(9123, false, "9.12", 9120);

    checkFormatBytes(9123456, true, "9.1", 9100000);
    checkFormatBytes(9123456, false, "9.12", 9120000);

    checkFormatBytes(-1, true, "-1", -1);
    checkFormatBytes(-1, false, "-1", -1);

    checkFormatBytes(-914, true, "-0.91", -910);
    checkFormatBytes(-914, false, "-0.91", -910);

    // Missing FLAG_CALCULATE_ROUNDED case.
    const Formatter::BytesResult r = Formatter::formatBytes(
            App::getInstance().getResources(), 1, 0);
    EXPECT_EQ("1", r.value);
    EXPECT_EQ(0, r.roundedBytes); // Didn't pass FLAG_CALCULATE_ROUNDED

    // Make sure it works on different locales.
    setLocale(Locale("es", "ES"));
    checkFormatBytes(9123000, false, "9,12", 9120000);
}

TEST_F(FormatterTest, testFormatBytesSi) {
    setLocale(Locale("en", "US"));

    checkFormatBytes(1'000, Formatter::FLAG_SI_UNITS, "1.00", 1'000);
    checkFormatBytes(1'024, Formatter::FLAG_SI_UNITS, "1.02", 1'020);
    checkFormatBytes(1'500, Formatter::FLAG_SI_UNITS, "1.50", 1'500);
    checkFormatBytes(12'582'912L, Formatter::FLAG_SI_UNITS, "12.58", 12'580'000L);
}

TEST_F(FormatterTest, testFormatBytesIec) {
    setLocale(Locale("en", "US"));

    checkFormatBytes(1'000, Formatter::FLAG_IEC_UNITS, "0.98", 1'003);
    checkFormatBytes(1'024, Formatter::FLAG_IEC_UNITS, "1.00", 1'024);
    checkFormatBytes(1'500, Formatter::FLAG_IEC_UNITS, "1.46", 1'495);
    checkFormatBytes(12'500'000L, Formatter::FLAG_IEC_UNITS, "11.92", 12'499'025L);
    checkFormatBytes(12'582'912L, Formatter::FLAG_IEC_UNITS, "12.00", 12'582'912L);
}

TEST_F(FormatterTest, testFormatShortElapsedTime) {
    setLocale(Locale("en", "US"));
    App& context = App::getInstance();
    EXPECT_EQ("3 days", Formatter::formatShortElapsedTime(context, 2 * DAY + 12 * HOUR));
    EXPECT_EQ("2 days", Formatter::formatShortElapsedTime(context, 2 * DAY + 11 * HOUR));
    EXPECT_EQ("2 days", Formatter::formatShortElapsedTime(context, 2 * DAY));
    EXPECT_EQ("1 day, 23 hr",
            Formatter::formatShortElapsedTime(context, 1 * DAY + 23 * HOUR + 59 * MINUTE));
    EXPECT_EQ("1 day", Formatter::formatShortElapsedTime(context, 1 * DAY + 59 * MINUTE));
    EXPECT_EQ("1 day", Formatter::formatShortElapsedTime(context, 1 * DAY));
    EXPECT_EQ("24 hr", Formatter::formatShortElapsedTime(context, 23 * HOUR + 30 * MINUTE));
    EXPECT_EQ("3 hr", Formatter::formatShortElapsedTime(context, 2 * HOUR + 30 * MINUTE));
    EXPECT_EQ("2 hr", Formatter::formatShortElapsedTime(context, 2 * HOUR));
    EXPECT_EQ("1 hr", Formatter::formatShortElapsedTime(context, 1 * HOUR));
    EXPECT_EQ("60 min", Formatter::formatShortElapsedTime(context, 59 * MINUTE + 30 * SECOND));
    EXPECT_EQ("59 min", Formatter::formatShortElapsedTime(context, 59 * MINUTE));
    EXPECT_EQ("3 min", Formatter::formatShortElapsedTime(context, 2 * MINUTE + 30 * SECOND));
    EXPECT_EQ("2 min", Formatter::formatShortElapsedTime(context, 2 * MINUTE));
    EXPECT_EQ("1 min, 59 sec",
            Formatter::formatShortElapsedTime(context, 1 * MINUTE + 59 * SECOND + 999));
    EXPECT_EQ("1 min", Formatter::formatShortElapsedTime(context, 1 * MINUTE));
    EXPECT_EQ("59 sec", Formatter::formatShortElapsedTime(context, 59 * SECOND + 999));
    EXPECT_EQ("1 sec", Formatter::formatShortElapsedTime(context, 1 * SECOND));
    EXPECT_EQ("0 sec", Formatter::formatShortElapsedTime(context, 1));
    EXPECT_EQ("0 sec", Formatter::formatShortElapsedTime(context, 0));

    // Make sure it works on different locales.
    // KNOWN DEVIATION: MeasureFormat unit words are an inline en-US table;
    // the French narrow-NBSP "2 j" expectation is red until an i18n
    // measure-word table exists.
    setLocale(Locale("fr", "FR"));
    EXPECT_EQ(u8"2\u202fj", Formatter::formatShortElapsedTime(context, 2 * DAY));
}

TEST_F(FormatterTest, testFormatShortElapsedTimeRoundingUpToMinutes) {
    setLocale(Locale("en", "US"));
    App& context = App::getInstance();
    EXPECT_EQ("3 days", Formatter::formatShortElapsedTimeRoundingUpToMinutes(
            context, 2 * DAY + 12 * HOUR));
    EXPECT_EQ("2 days", Formatter::formatShortElapsedTimeRoundingUpToMinutes(
            context, 2 * DAY + 11 * HOUR));
    EXPECT_EQ("2 days", Formatter::formatShortElapsedTimeRoundingUpToMinutes(context, 2 * DAY));
    EXPECT_EQ("1 day, 23 hr", Formatter::formatShortElapsedTimeRoundingUpToMinutes(
            context, 1 * DAY + 23 * HOUR + 59 * MINUTE));
    EXPECT_EQ("1 day", Formatter::formatShortElapsedTimeRoundingUpToMinutes(
            context, 1 * DAY + 59 * MINUTE));
    EXPECT_EQ("1 day", Formatter::formatShortElapsedTimeRoundingUpToMinutes(context, 1 * DAY));
    EXPECT_EQ("24 hr", Formatter::formatShortElapsedTimeRoundingUpToMinutes(
            context, 23 * HOUR + 30 * MINUTE));
    EXPECT_EQ("3 hr", Formatter::formatShortElapsedTimeRoundingUpToMinutes(
            context, 2 * HOUR + 30 * MINUTE));
    EXPECT_EQ("2 hr", Formatter::formatShortElapsedTimeRoundingUpToMinutes(context, 2 * HOUR));
    EXPECT_EQ("1 hr", Formatter::formatShortElapsedTimeRoundingUpToMinutes(context, 1 * HOUR));
    EXPECT_EQ("1 hr", Formatter::formatShortElapsedTimeRoundingUpToMinutes(
            context, 59 * MINUTE + 30 * SECOND));
    EXPECT_EQ("59 min", Formatter::formatShortElapsedTimeRoundingUpToMinutes(
            context, 59 * MINUTE));
    EXPECT_EQ("3 min", Formatter::formatShortElapsedTimeRoundingUpToMinutes(
            context, 2 * MINUTE + 30 * SECOND));
    EXPECT_EQ("2 min", Formatter::formatShortElapsedTimeRoundingUpToMinutes(
            context, 2 * MINUTE));
    EXPECT_EQ("2 min", Formatter::formatShortElapsedTimeRoundingUpToMinutes(
            context, 1 * MINUTE + 59 * SECOND + 999));
    EXPECT_EQ("1 min", Formatter::formatShortElapsedTimeRoundingUpToMinutes(
            context, 1 * MINUTE));
    EXPECT_EQ("1 min", Formatter::formatShortElapsedTimeRoundingUpToMinutes(
            context, 59 * SECOND + 999));
    EXPECT_EQ("1 min", Formatter::formatShortElapsedTimeRoundingUpToMinutes(
            context, 1 * SECOND));
    EXPECT_EQ("1 min", Formatter::formatShortElapsedTimeRoundingUpToMinutes(context, 1));
    EXPECT_EQ("0 min", Formatter::formatShortElapsedTimeRoundingUpToMinutes(context, 0));

    // Make sure it works on different locales.
    // KNOWN DEVIATION: same en-US unit-word table; the Russian "1 мин"
    // expectation is red until an i18n measure-word table exists.
    setLocale(Locale("ru", "RU"));
    EXPECT_EQ(u8"1\u043c\u0438\u043d", Formatter::formatShortElapsedTimeRoundingUpToMinutes(
            context, 1 * SECOND));
}

} // namespace
