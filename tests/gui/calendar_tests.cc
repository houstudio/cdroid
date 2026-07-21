#include <gtest/gtest.h>
#include <cdroid.h>
#include <core/calendar.h>
#include <core/buddhistcalendar.h>
#include <core/taiwancalendar.h>

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
