#include <gtest/gtest.h>
#include <cdroid.h>
#include <core/calendar.h>
#include <core/buddhistcalendar.h>
#include <core/taiwancalendar.h>
#include <core/copticcalendar.h>
#include <core/ethiopiccalendar.h>
#include <core/hebrewcalendar.h>
#include <core/persiancalendar.h>
#include <core/islamiccalendar.h>
#include <core/indiancalendar.h>
#include <core/japanesecalendar.h>

// Calendar alignment tests (android-36 java.util.Calendar). All instances are
// obtained via getInstance()/GregorianCalendar; CDROID keeps Calendar concrete
// (ICU-style) but these exercise the real Gregorian path. SUNDAY=1..SATURDAY=7.

class CALENDAR:public testing::Test{
   public:
   virtual void SetUp(){}
   virtual void TearDown(){}
};

// ---- computeFields (millis -> fields) ----

// 1970-01-01 00:00:00 UTC == epoch 0; it was a Thursday.
TEST_F(CALENDAR, epoch_utc){
    auto cal = Calendar::getInstance(0);
    cal->set(1970, 0, 1, 0, 0, 0);
    ASSERT_EQ(cal->getTime(), 0);
    ASSERT_EQ(cal->get(Calendar::YEAR), 1970);
    ASSERT_EQ(cal->get(Calendar::MONTH), Calendar::JANUARY);
    ASSERT_EQ(cal->get(Calendar::DAY_OF_MONTH), 1);
    ASSERT_EQ(cal->get(Calendar::DAY_OF_WEEK), Calendar::THURSDAY);
    ASSERT_EQ(cal->get(Calendar::DAY_OF_YEAR), 1);
}

// Local 1970-01-01 00:00 at +08:00 == UTC 1969-12-31 16:00 == -28800000 ms.
TEST_F(CALENDAR, epoch_zone_plus8){
    auto cal = Calendar::getInstance(0);
    cal->setTimeZone(8 * 3600);
    cal->set(1970, 0, 1, 0, 0, 0);
    ASSERT_EQ(cal->getTime(), -28800000LL);
}

// ZONE_OFFSET is exposed in milliseconds (Android convention).
TEST_F(CALENDAR, zone_offset_is_millis){
    auto cal = Calendar::getInstance(8 * 3600);
    cal->set(1970, 0, 1, 0, 0, 0);
    ASSERT_EQ(cal->get(Calendar::ZONE_OFFSET), 8 * 3600 * 1000);
    ASSERT_EQ(cal->get(Calendar::DST_OFFSET), 0);
}

// Known weekdays.
TEST_F(CALENDAR, known_weekdays){
    auto cal = Calendar::getInstance(0);
    cal->set(2000, 0, 1); // Saturday
    ASSERT_EQ(cal->get(Calendar::DAY_OF_WEEK), Calendar::SATURDAY);
    cal->set(2024, 0, 1); // Monday
    ASSERT_EQ(cal->get(Calendar::DAY_OF_WEEK), Calendar::MONDAY);
    cal->set(2024, 1, 29); // Thursday
    ASSERT_EQ(cal->get(Calendar::DAY_OF_WEEK), Calendar::THURSDAY);
    cal->set(2025, 0, 1); // Wednesday
    ASSERT_EQ(cal->get(Calendar::DAY_OF_WEEK), Calendar::WEDNESDAY);
}

// WEEK_OF_YEAR of Jan 1 is week 1 (firstDayOfWeek=Sunday, minDays=1 default).
TEST_F(CALENDAR, week_of_year_jan1){
    auto cal = Calendar::getInstance(0);
    cal->set(2024, 0, 1);
    ASSERT_EQ(cal->get(Calendar::WEEK_OF_YEAR), 1);
}

// ---- computeTime (fields -> millis), selectFields combinations ----

// DAY_OF_YEAR path must be honored, not MONTH+DAY_OF_MONTH.
TEST_F(CALENDAR, day_of_year_path){
    auto cal = Calendar::getInstance(0);
    cal->clear();
    cal->set(Calendar::YEAR, 2024);
    cal->set(Calendar::DAY_OF_YEAR, 1); // 2024-01-01 Monday
    ASSERT_EQ(cal->get(Calendar::MONTH), Calendar::JANUARY);
    ASSERT_EQ(cal->get(Calendar::DAY_OF_MONTH), 1);
    ASSERT_EQ(cal->get(Calendar::DAY_OF_WEEK), Calendar::MONDAY);
    cal->set(Calendar::DAY_OF_YEAR, 60); // 2024-02-29 (leap)
    ASSERT_EQ(cal->get(Calendar::MONTH), Calendar::FEBRUARY);
    ASSERT_EQ(cal->get(Calendar::DAY_OF_MONTH), 29);
}

// DAY_OF_WEEK_IN_MONTH + DAY_OF_WEEK -> the Nth weekday of the month.
TEST_F(CALENDAR, day_of_week_in_month_path){
    auto cal = Calendar::getInstance(0);
    cal->clear();
    cal->set(Calendar::YEAR, 2024);
    cal->set(Calendar::MONTH, Calendar::JANUARY);
    cal->set(Calendar::DAY_OF_WEEK, Calendar::MONDAY);
    cal->set(Calendar::DAY_OF_WEEK_IN_MONTH, 2); // 2nd Monday of Jan 2024 = Jan 8
    ASSERT_EQ(cal->get(Calendar::DAY_OF_MONTH), 8);
}

// HOUR + AM_PM resolves to the 24-hour time of day.
TEST_F(CALENDAR, hour_am_pm_path){
    auto cal = Calendar::getInstance(0);
    cal->clear();
    cal->set(Calendar::YEAR, 2024);
    cal->set(Calendar::MONTH, Calendar::JANUARY);
    cal->set(Calendar::DAY_OF_MONTH, 1);
    cal->set(Calendar::HOUR, 10);
    cal->set(Calendar::AM_PM, Calendar::PM);
    ASSERT_EQ(cal->get(Calendar::HOUR_OF_DAY), 22);
}

// millis -> fields -> millis round trip at a non-trivial instant.
TEST_F(CALENDAR, round_trip){
    auto cal = Calendar::getInstance(0);
    cal->setTime(123456789000LL);
    ASSERT_EQ(cal->getTime(), 123456789000LL);
}

// ---- add() ----

// add(DAY_OF_WEEK, n) advances n days (1-day unit, Android semantics).
TEST_F(CALENDAR, add_day_of_week){
    auto cal = Calendar::getInstance(0);
    cal->set(2024, 0, 1, 0, 0, 0); // Monday
    cal->add(Calendar::DAY_OF_WEEK, 1);
    ASSERT_EQ(cal->get(Calendar::DAY_OF_MONTH), 2);
    ASSERT_EQ(cal->get(Calendar::DAY_OF_WEEK), Calendar::TUESDAY);
}

// Add Rule 2: Jan 31 + 1 month -> Feb 29 (leap), not Mar 2.
TEST_F(CALENDAR, add_month_rule2){
    auto cal = Calendar::getInstance(0);
    cal->set(2024, 0, 31, 0, 0, 0);
    cal->add(Calendar::MONTH, 1);
    ASSERT_EQ(cal->get(Calendar::MONTH), Calendar::FEBRUARY);
    ASSERT_EQ(cal->get(Calendar::DAY_OF_MONTH), 29);
    cal->set(2024, 2, 31, 0, 0, 0); // Mar 31
    cal->add(Calendar::MONTH, 1);
    ASSERT_EQ(cal->get(Calendar::MONTH), Calendar::APRIL);
    ASSERT_EQ(cal->get(Calendar::DAY_OF_MONTH), 30);
}

// Add Rule 2 with negative amount: Mar 31 - 1 month -> Feb 29.
TEST_F(CALENDAR, add_month_negative){
    auto cal = Calendar::getInstance(0);
    cal->set(2024, 2, 31, 0, 0, 0);
    cal->add(Calendar::MONTH, -1);
    ASSERT_EQ(cal->get(Calendar::MONTH), Calendar::FEBRUARY);
    ASSERT_EQ(cal->get(Calendar::DAY_OF_MONTH), 29);
}

// Add Rule 2 for YEAR: Feb 29 + 1 year -> Feb 28 (non-leap target).
TEST_F(CALENDAR, add_year_rule2){
    auto cal = Calendar::getInstance(0);
    cal->set(2024, 1, 29, 0, 0, 0);
    cal->add(Calendar::YEAR, 1);
    ASSERT_EQ(cal->get(Calendar::YEAR), 2025);
    ASSERT_EQ(cal->get(Calendar::MONTH), Calendar::FEBRUARY);
    ASSERT_EQ(cal->get(Calendar::DAY_OF_MONTH), 28);
}

// add across day boundaries (leap day).
TEST_F(CALENDAR, add_day_leap){
    auto cal = Calendar::getInstance(0);
    cal->set(2024, 1, 28, 0, 0, 0);
    cal->add(Calendar::DAY_OF_MONTH, 1); // -> Feb 29
    ASSERT_EQ(cal->get(Calendar::DAY_OF_MONTH), 29);
    cal->add(Calendar::DAY_OF_MONTH, 1); // -> Mar 1
    ASSERT_EQ(cal->get(Calendar::MONTH), Calendar::MARCH);
    ASSERT_EQ(cal->get(Calendar::DAY_OF_MONTH), 1);
}

// add on time fields shifts the millis by the right amount.
TEST_F(CALENDAR, add_time_fields){
    auto cal = Calendar::getInstance(0);
    cal->set(2024, 0, 1, 10, 30, 45);
    int64_t before = cal->getTime();
    cal->add(Calendar::HOUR_OF_DAY, 1);
    ASSERT_EQ(cal->getTime() - before, 3600 * 1000LL);
    cal->add(Calendar::MINUTE, 1);
    ASSERT_EQ(cal->get(Calendar::MINUTE), 31);
    cal->add(Calendar::SECOND, 1);
    ASSERT_EQ(cal->get(Calendar::SECOND), 46);
    cal->add(Calendar::MILLISECOND, 500);
    ASSERT_EQ(cal->get(Calendar::MILLISECOND), 500);
}

// add(WEEK_OF_YEAR, n) advances n weeks.
TEST_F(CALENDAR, add_week){
    auto cal = Calendar::getInstance(0);
    cal->set(2024, 0, 1, 0, 0, 0);
    cal->add(Calendar::WEEK_OF_YEAR, 2);
    ASSERT_EQ(cal->get(Calendar::DAY_OF_MONTH), 15);
}

// ---- roll() ----

// roll(MONTH) keeps YEAR; Add Rule 2 clamps day.
TEST_F(CALENDAR, roll_month){
    auto cal = Calendar::getInstance(0);
    cal->set(2024, 0, 15, 0, 0, 0);
    cal->roll(Calendar::MONTH, 1);
    ASSERT_EQ(cal->get(Calendar::YEAR), 2024);
    ASSERT_EQ(cal->get(Calendar::MONTH), Calendar::FEBRUARY);
    ASSERT_EQ(cal->get(Calendar::DAY_OF_MONTH), 15);
    cal->set(2024, 0, 31, 0, 0, 0);
    cal->roll(Calendar::MONTH, 1); // Jan 31 -> Feb 29 (clamped), year unchanged
    ASSERT_EQ(cal->get(Calendar::YEAR), 2024);
    ASSERT_EQ(cal->get(Calendar::MONTH), Calendar::FEBRUARY);
    ASSERT_EQ(cal->get(Calendar::DAY_OF_MONTH), 29);
}

// roll(HOUR_OF_DAY) wraps within the day, date unchanged.
TEST_F(CALENDAR, roll_hour){
    auto cal = Calendar::getInstance(0);
    cal->set(2024, 0, 1, 10, 0, 0);
    cal->roll(Calendar::HOUR_OF_DAY, 5);
    ASSERT_EQ(cal->get(Calendar::HOUR_OF_DAY), 15);
    ASSERT_EQ(cal->get(Calendar::DAY_OF_MONTH), 1);
    cal->roll(Calendar::HOUR_OF_DAY, 12); // wraps 15 -> 3 next? wraps within 24
    ASSERT_EQ(cal->get(Calendar::DAY_OF_MONTH), 1); // date still unchanged
}

// roll(DAY_OF_MONTH) wraps within the month.
TEST_F(CALENDAR, roll_day_of_month){
    auto cal = Calendar::getInstance(0);
    cal->set(2024, 0, 15, 0, 0, 0);
    cal->roll(Calendar::DAY_OF_MONTH, 1);
    ASSERT_EQ(cal->get(Calendar::DAY_OF_MONTH), 16);
    ASSERT_EQ(cal->get(Calendar::MONTH), Calendar::JANUARY);
}

// ---- limits ----

TEST_F(CALENDAR, actual_min_max){
    auto cal = Calendar::getInstance(0);
    cal->set(2024, 1, 15); // Feb 2024 (leap)
    ASSERT_EQ(cal->getActualMaximum(Calendar::DAY_OF_MONTH), 29);
    ASSERT_EQ(cal->getActualMinimum(Calendar::DAY_OF_MONTH), 1);
    cal->set(2023, 1, 15); // Feb 2023
    ASSERT_EQ(cal->getActualMaximum(Calendar::DAY_OF_MONTH), 28);
    ASSERT_EQ(cal->getActualMaximum(Calendar::MONTH), Calendar::DECEMBER);
    ASSERT_EQ(cal->getActualMaximum(Calendar::HOUR_OF_DAY), 23);
    ASSERT_EQ(cal->getActualMaximum(Calendar::DAY_OF_WEEK), Calendar::SATURDAY);
}

// 2100 is NOT a leap year (divisible by 100 but not 400).
TEST_F(CALENDAR, non_leap_century){
    auto cal = Calendar::getInstance(0);
    cal->set(2100, 1, 1);
    ASSERT_EQ(cal->getActualMaximum(Calendar::DAY_OF_MONTH), 28);
    ASSERT_FALSE(Calendar::isLeapYear(2100));
    ASSERT_FALSE(Calendar::isLeapYear(1900));
    ASSERT_TRUE(Calendar::isLeapYear(2000));
    ASSERT_TRUE(Calendar::isLeapYear(2024));
    ASSERT_EQ(Calendar::getDaysInMonth(2024, Calendar::FEBRUARY), 29);
    ASSERT_EQ(Calendar::getDaysInMonth(2023, Calendar::FEBRUARY), 28);
}

// ---- BC / extended range ----

// 1 BC (extended year 0) is before the epoch.
TEST_F(CALENDAR, bc_date){
    auto cal = Calendar::getInstance(0);
    cal->clear();
    cal->set(Calendar::ERA, Calendar::BC);
    cal->set(Calendar::YEAR, 1);
    cal->set(Calendar::MONTH, Calendar::JANUARY);
    cal->set(Calendar::DAY_OF_MONTH, 1);
    ASSERT_EQ(cal->get(Calendar::ERA), Calendar::BC);
    ASSERT_LT(cal->getTime(), 0);
}

// Dates past the 2038 32-bit time_t boundary work (pure-integer, no libc).
TEST_F(CALENDAR, year_past_2038){
    auto cal = Calendar::getInstance(0);
    cal->set(2040, 0, 1, 0, 0, 0);
    ASSERT_GT(cal->getTime(), 0);
    ASSERT_EQ(cal->get(Calendar::YEAR), 2040);
    cal->set(2099, 11, 31, 23, 59, 59);
    ASSERT_EQ(cal->get(Calendar::YEAR), 2099);
}

// ---- comparison / misc ----

TEST_F(CALENDAR, after_before){
    auto a = Calendar::getInstance(0);
    auto b = Calendar::getInstance(0);
    a->set(2024, 0, 1, 0, 0, 0);
    b->set(2024, 0, 2, 0, 0, 0);
    ASSERT_TRUE(b->after(*a));
    ASSERT_TRUE(a->before(*b));
    ASSERT_FALSE(a->after(*b));
}

TEST_F(CALENDAR, first_day_of_week){
    auto cal = Calendar::getInstance(0);
    cal->setFirstDayOfWeek(Calendar::MONDAY);
    ASSERT_EQ(cal->getFirstDayOfWeek(), Calendar::MONDAY);
    cal->setMinimalDaysInFirstWeek(4);
    ASSERT_EQ(cal->getMinimalDaysInFirstWeek(), 4);
}

// isSet reflects the nextStamp fix (first set() must mark the field set).
TEST_F(CALENDAR, is_set_semantics){
    auto cal = Calendar::getInstance(0);
    cal->clear();
    ASSERT_FALSE(cal->isSet(Calendar::YEAR));
    cal->set(Calendar::YEAR, 2024);
    ASSERT_TRUE(cal->isSet(Calendar::YEAR));
    cal->clear(Calendar::YEAR);
    ASSERT_FALSE(cal->isSet(Calendar::YEAR));
}

// ---- Buddhist calendar (BE = Gregorian + 543) ----

TEST_F(CALENDAR, buddhist_offset){
    BuddhistCalendar bc;
    bc.setTimeZone(0);
    bc.set(2541, 0, 1, 0, 0, 0); // BE 2541-01-01 == AD 1998-01-01
    auto g = Calendar::getInstance(0);
    g->set(1998, 0, 1, 0, 0, 0);
    ASSERT_EQ(bc.getTime(), g->getTime());
    ASSERT_EQ(bc.get(Calendar::YEAR), 2541);
    ASSERT_EQ(bc.get(Calendar::MONTH), Calendar::JANUARY);
    ASSERT_EQ(bc.get(Calendar::ERA), 0); // BE
    // Same weekday as the equivalent Gregorian date (1998-01-01 = Thursday).
    ASSERT_EQ(bc.get(Calendar::DAY_OF_WEEK), Calendar::THURSDAY);
}

// ---- Taiwan / Minguo calendar (Minguo = Gregorian - 1911) ----

TEST_F(CALENDAR, taiwan_minguo){
    TaiwanCalendar tc;
    tc.setTimeZone(0);
    tc.set(113, 0, 1, 0, 0, 0); // Minguo 113-01-01 == AD 2024-01-01
    auto g = Calendar::getInstance(0);
    g->set(2024, 0, 1, 0, 0, 0);
    ASSERT_EQ(tc.getTime(), g->getTime());
    ASSERT_EQ(tc.get(Calendar::YEAR), 113);
    ASSERT_EQ(tc.get(Calendar::ERA), 1); // MINGUO
    ASSERT_EQ(tc.get(Calendar::DAY_OF_WEEK), Calendar::MONDAY);
}

// ---- Coptic / Ethiopic (shared CECalendar, 13-month) ----

TEST_F(CALENDAR, coptic_month_lengths){
    CopticCalendar cc;
    cc.setTimeZone(0);
    cc.set(1739, 12, 1, 0, 0, 0); // 13th month, leap year (1739 % 4 == 3)
    ASSERT_EQ(cc.getActualMaximum(Calendar::DAY_OF_MONTH), 6);
    cc.set(1737, 12, 1, 0, 0, 0); // 13th month, non-leap
    ASSERT_EQ(cc.getActualMaximum(Calendar::DAY_OF_MONTH), 5);
    cc.set(1737, 0, 1, 0, 0, 0);  // regular month
    ASSERT_EQ(cc.getActualMaximum(Calendar::DAY_OF_MONTH), 30);
}

TEST_F(CALENDAR, coptic_roundtrip){
    CopticCalendar cc;
    cc.setTimeZone(0);
    cc.set(1737, 5, 15, 12, 30, 45);
    ASSERT_EQ(cc.get(Calendar::YEAR), 1737);
    ASSERT_EQ(cc.get(Calendar::MONTH), 5);
    ASSERT_EQ(cc.get(Calendar::DAY_OF_MONTH), 15);
    ASSERT_EQ(cc.get(Calendar::HOUR_OF_DAY), 12);
    ASSERT_EQ(cc.get(Calendar::MINUTE), 30);
}

TEST_F(CALENDAR, ethiopic_roundtrip){
    EthiopicCalendar ec;
    ec.setTimeZone(0);
    ec.set(2015, 5, 15, 9, 0, 0);
    ASSERT_EQ(ec.get(Calendar::YEAR), 2015);
    ASSERT_EQ(ec.get(Calendar::MONTH), 5);
    ASSERT_EQ(ec.get(Calendar::DAY_OF_MONTH), 15);
}

// Coptic and Ethiopic differ only in epoch -> same nominal date gives different
// instants (epoch offset 1825029 vs 1723856).
TEST_F(CALENDAR, coptic_ethiopic_distinct_epoch){
    CopticCalendar cc; cc.setTimeZone(0); cc.set(1, 0, 1, 0, 0, 0);
    EthiopicCalendar ec; ec.setTimeZone(0); ec.set(1, 0, 1, 0, 0, 0);
    ASSERT_NE(cc.getTime(), ec.getTime());
}

// ---- Hebrew (lunisolar; molad + dehiyyot) ----

TEST_F(CALENDAR, hebrew_rosh_hashanah){
    HebrewCalendar hc; hc.setTimeZone(0); hc.set(5784, 0, 1, 0, 0, 0); // Tishri 1, 5784
    auto g = Calendar::getInstance(0); g->set(2023, 8, 16, 0, 0, 0);    // == 2023-09-16 (Saturday)
    ASSERT_EQ(hc.getTime(), g->getTime());
    ASSERT_EQ(hc.get(Calendar::YEAR), 5784);
    ASSERT_EQ(hc.get(Calendar::DAY_OF_WEEK), Calendar::SATURDAY);
}

TEST_F(CALENDAR, hebrew_roundtrip){
    HebrewCalendar hc; hc.setTimeZone(0); hc.set(5784, 5, 15, 10, 30, 0);
    ASSERT_EQ(hc.get(Calendar::YEAR), 5784);
    ASSERT_EQ(hc.get(Calendar::MONTH), 5);
    ASSERT_EQ(hc.get(Calendar::DAY_OF_MONTH), 15);
    ASSERT_EQ(hc.get(Calendar::HOUR_OF_DAY), 10);
}

// ---- Persian (Solar Hijri / Birashk arithmetic) ----
// Note: CDROID uses the Birashk 2820-cycle arithmetic algorithm, NOT the
// astronomical Iranian calendar. They diverge by +/-1 day in some years
// (e.g. 1403: Birashk Nowruz = 2024-03-19, astronomical = 2024-03-20).

TEST_F(CALENDAR, persian_nowruz){
    PersianCalendar pc; pc.setTimeZone(0); pc.set(1403, 0, 1, 0, 0, 0); // Farvardin 1, 1403
    auto g = Calendar::getInstance(0); g->set(2024, 2, 19, 0, 0, 0);    // Birashk: 2024-03-19
    ASSERT_EQ(pc.getTime(), g->getTime());
    ASSERT_EQ(pc.get(Calendar::YEAR), 1403);
    ASSERT_EQ(pc.get(Calendar::DAY_OF_WEEK), Calendar::TUESDAY);
}

TEST_F(CALENDAR, persian_roundtrip){
    PersianCalendar pc; pc.setTimeZone(0); pc.set(1403, 5, 15, 14, 30, 0);
    ASSERT_EQ(pc.get(Calendar::YEAR), 1403);
    ASSERT_EQ(pc.get(Calendar::MONTH), 5);
    ASSERT_EQ(pc.get(Calendar::DAY_OF_MONTH), 15);
    ASSERT_EQ(pc.get(Calendar::HOUR_OF_DAY), 14);
}

// ---- Islamic (tabular) ----

TEST_F(CALENDAR, islamic_roundtrip){
    IslamicCalendar ic;
    ic.setTimeZone(0);
    ic.set(1446, 0, 1, 0, 0, 0); // Muharram 1, 1446
    ASSERT_EQ(ic.get(Calendar::YEAR), 1446);
    ASSERT_EQ(ic.get(Calendar::MONTH), 0);
    ASSERT_EQ(ic.get(Calendar::DAY_OF_MONTH), 1);
}

TEST_F(CALENDAR, islamic_month_lengths){
    IslamicCalendar ic;
    ic.setTimeZone(0);
    ic.set(1446, 0, 1, 0, 0, 0);
    ASSERT_EQ(ic.getActualMaximum(Calendar::DAY_OF_MONTH), 30); // odd months 30
    ic.set(1446, 1, 1, 0, 0, 0);
    ASSERT_EQ(ic.getActualMaximum(Calendar::DAY_OF_MONTH), 29); // even months 29
}

// ---- Indian (Saka / Indian National Calendar) ----
// Saka year = Gregorian - 78; Chaitra 1 falls on Mar 22 (or Mar 21 in a leap year).

TEST_F(CALENDAR, indian_chaitra){
    IndianCalendar ic; ic.setTimeZone(0); ic.set(1946, 0, 1, 0, 0, 0); // Chaitra 1, Saka 1946
    auto g = Calendar::getInstance(0); g->set(2024, 2, 21, 0, 0, 0);    // 2024-03-21 (leap-year shift)
    ASSERT_EQ(ic.getTime(), g->getTime());
    ASSERT_EQ(ic.get(Calendar::YEAR), 1946);
}

TEST_F(CALENDAR, indian_roundtrip){
    IndianCalendar ic; ic.setTimeZone(0); ic.set(1946, 5, 15, 10, 0, 0);
    ASSERT_EQ(ic.get(Calendar::YEAR), 1946);
    ASSERT_EQ(ic.get(Calendar::MONTH), 5);
    ASSERT_EQ(ic.get(Calendar::DAY_OF_MONTH), 15);
}

// ---- Japanese (imperial eras: Meiji=232..Reiwa=236) ----

TEST_F(CALENDAR, japanese_reiwa1){
    JapaneseCalendar jc; jc.setTimeZone(0);
    jc.set(Calendar::ERA, 236);       // Reiwa
    jc.set(Calendar::YEAR, 1);
    jc.set(Calendar::MONTH, 4);       // May
    jc.set(Calendar::DAY_OF_MONTH, 1);
    jc.set(Calendar::HOUR_OF_DAY, 0); jc.set(Calendar::MINUTE, 0);
    jc.set(Calendar::SECOND, 0); jc.set(Calendar::MILLISECOND, 0);
    auto g = Calendar::getInstance(0); g->set(2019, 4, 1, 0, 0, 0); // 2019-05-01
    ASSERT_EQ(jc.getTime(), g->getTime());
}

TEST_F(CALENDAR, japanese_showa1){
    JapaneseCalendar jc; jc.setTimeZone(0);
    jc.set(Calendar::ERA, 234);       // Showa
    jc.set(Calendar::YEAR, 1);
    jc.set(Calendar::MONTH, 11);      // Dec
    jc.set(Calendar::DAY_OF_MONTH, 25);
    jc.set(Calendar::HOUR_OF_DAY, 0); jc.set(Calendar::MINUTE, 0);
    jc.set(Calendar::SECOND, 0); jc.set(Calendar::MILLISECOND, 0);
    auto g = Calendar::getInstance(0); g->set(1926, 11, 25, 0, 0, 0); // 1926-12-25
    ASSERT_EQ(jc.getTime(), g->getTime());
}

TEST_F(CALENDAR, japanese_era_boundary){
    JapaneseCalendar jc; jc.setTimeZone(0);
    auto g = Calendar::getInstance(0);
    g->set(2019, 4, 1, 0, 0, 0);      // Reiwa day 1
    jc.setTime(g->getTime());
    ASSERT_EQ(jc.get(Calendar::ERA), 236);   // Reiwa
    ASSERT_EQ(jc.get(Calendar::YEAR), 1);
    g->set(2019, 3, 30, 0, 0, 0);     // day before -> still Heisei
    jc.setTime(g->getTime());
    ASSERT_EQ(jc.get(Calendar::ERA), 235);   // Heisei
    ASSERT_EQ(jc.get(Calendar::YEAR), 31);   // 2019 - 1989 + 1
}
