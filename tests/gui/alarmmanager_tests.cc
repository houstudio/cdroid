// Tests for the android.app.AlarmManager port (app/alarmmanager.{h,cc}).
//
// CDROID adaptation notes:
//  - Upstream CTS drives AlarmManager against system_server and waits on a
//    looping thread; here deliveries are posted to the main looper by the
//    internal alarm thread, so tests pump with pumpFor().
//  - Timings are generous multiples of the looper granularity but short
//    enough to keep the suite fast (the whole file adds ~3s).
//  - setTime() is intentionally not exercised: on a root dev box it would
//    really jump the wall clock of the build machine.
#include <gtest/gtest.h>
#include <app/alarmmanager.h>
#include <app/pendingintent.h>
#include <climits>
#include <core/intent.h>
#include <core/looper.h>
#include <core/systemclock.h>
#include "guienvironment.h"

using namespace cdroid;

namespace {
constexpr long SOON = 60;   /* ms ahead: when alarms are set to fire */
constexpr long PUMP = 300;  /* ms pumped while waiting for delivery */
}

TEST(AlarmManagerTest, testConstants) {
    EXPECT_EQ(0, (int)AlarmManager::RTC_WAKEUP);
    EXPECT_EQ(1, (int)AlarmManager::RTC);
    EXPECT_EQ(2, (int)AlarmManager::ELAPSED_REALTIME_WAKEUP);
    EXPECT_EQ(3, (int)AlarmManager::ELAPSED_REALTIME);
    EXPECT_EQ(0L, (long)AlarmManager::WINDOW_EXACT);
    EXPECT_EQ(-1L, (long)AlarmManager::WINDOW_HEURISTIC);
    EXPECT_EQ(24L * 3600 * 1000, (long)AlarmManager::INTERVAL_DAY);
    EXPECT_STREQ("android.app.action.NEXT_ALARM_CLOCK_CHANGED",
                 AlarmManager::ACTION_NEXT_ALARM_CLOCK_CHANGED);
}

TEST(AlarmManagerTest, testSetListenerFiresOnce) {
    AlarmManager& am = AlarmManager::getInstance();
    int fired = 0;
    AlarmManager::OnAlarmListener listener = [&fired]() { fired++; };
    am.set(AlarmManager::ELAPSED_REALTIME, SystemClock::elapsedRealtime() + SOON,
           "tag", listener, nullptr);
    pumpFor(PUMP);
    EXPECT_EQ(1, fired);
    pumpFor(SOON);
    EXPECT_EQ(1, fired);  /* one-shot: no repeat */
    am.cancel(listener);
}

TEST(AlarmManagerTest, testSetExactListenerFires) {
    AlarmManager& am = AlarmManager::getInstance();
    int fired = 0;
    AlarmManager::OnAlarmListener listener = [&fired]() { fired++; };
    am.setExact(AlarmManager::ELAPSED_REALTIME, SystemClock::elapsedRealtime() + SOON,
                "exact", listener, nullptr);
    pumpFor(PUMP);
    EXPECT_EQ(1, fired);
}

TEST(AlarmManagerTest, testSetWindowListenerFiresAtWindowStart) {
    AlarmManager& am = AlarmManager::getInstance();
    int fired = 0;
    AlarmManager::OnAlarmListener listener = [&fired]() { fired++; };
    am.setWindow(AlarmManager::ELAPSED_REALTIME,
                 SystemClock::elapsedRealtime() + SOON, 1000, "win", listener, nullptr);
    pumpFor(PUMP);
    EXPECT_EQ(1, fired);  /* no batcher: windowed = delivered at window start */
}

TEST(AlarmManagerTest, testRtcWakeupConversion) {
    /* RTC-domain trigger converts through the wall/elapsed offset */
    AlarmManager& am = AlarmManager::getInstance();
    int fired = 0;
    AlarmManager::OnAlarmListener listener = [&fired]() { fired++; };
    am.set(AlarmManager::RTC_WAKEUP, SystemClock::currentTimeMillis() + SOON,
           "rtc", listener, nullptr);
    pumpFor(PUMP);
    EXPECT_EQ(1, fired);
}

TEST(AlarmManagerTest, testCancelListenerBeforeFire) {
    AlarmManager& am = AlarmManager::getInstance();
    int fired = 0;
    AlarmManager::OnAlarmListener listener = [&fired]() { fired++; };
    am.set(AlarmManager::ELAPSED_REALTIME, SystemClock::elapsedRealtime() + 150,
           "tag", listener, nullptr);
    am.cancel(listener);
    pumpFor(PUMP);
    EXPECT_EQ(0, fired);
}

TEST(AlarmManagerTest, testSameListenerReplacesPreviousAlarm) {
    /* Upstream: an OnAlarmListener can back only one alarm (sWrappers reuse);
     * re-setting it replaces the earlier deadline. */
    AlarmManager& am = AlarmManager::getInstance();
    int fired = 0;
    AlarmManager::OnAlarmListener listener = [&fired]() { fired++; };
    am.set(AlarmManager::ELAPSED_REALTIME, SystemClock::elapsedRealtime() + 800,
           "slow", listener, nullptr);
    am.set(AlarmManager::ELAPSED_REALTIME, SystemClock::elapsedRealtime() + SOON,
           "fast", listener, nullptr);
    pumpFor(PUMP);
    EXPECT_EQ(1, fired);  /* fast one fired; slow one was replaced away */
    pumpFor(600);
    EXPECT_EQ(1, fired);
}

TEST(AlarmManagerTest, testCancelNullThrows) {
    AlarmManager& am = AlarmManager::getInstance();
    EXPECT_THROW(am.cancel((PendingIntent*)nullptr), std::invalid_argument);
    AlarmManager::OnAlarmListener empty;
    EXPECT_THROW(am.cancel(empty), std::invalid_argument);
}

TEST(AlarmManagerTest, testBadTypeThrows) {
    AlarmManager& am = AlarmManager::getInstance();
    AlarmManager::OnAlarmListener listener = []() {};
    EXPECT_THROW(am.set(42, 0, "bad", listener, nullptr), std::invalid_argument);
}

TEST(AlarmManagerTest, testNegativeTriggerClampsToNow) {
    /* AlarmManager.java:915-924: negative trigger -> fires immediately */
    AlarmManager& am = AlarmManager::getInstance();
    int fired = 0;
    AlarmManager::OnAlarmListener listener = [&fired]() { fired++; };
    am.set(AlarmManager::ELAPSED_REALTIME, -5, "past", listener, nullptr);
    pumpFor(SOON);
    EXPECT_EQ(1, fired);
}

TEST(AlarmManagerTest, testPendingIntentDelivery) {
    AlarmManager& am = AlarmManager::getInstance();
    int sent = 0;
    Intent intent;
    intent.setAction("cdroid.test.ALARM");
    PendingIntent* pi = PendingIntent::getBroadcast(nullptr, 0, &intent, 0,
            [&sent](Intent& i) {
                sent++;
                EXPECT_EQ("cdroid.test.ALARM", i.getAction());
            });
    am.set(AlarmManager::ELAPSED_REALTIME, SystemClock::elapsedRealtime() + SOON, pi);
    pumpFor(PUMP);
    EXPECT_EQ(1, sent);
    am.cancel(pi);
    delete pi;
}

TEST(AlarmManagerTest, testSetRepeatingThenCancel) {
    AlarmManager& am = AlarmManager::getInstance();
    int fired = 0;
    Intent intent;
    PendingIntent* pi = PendingIntent::getBroadcast(nullptr, 0, &intent, 0,
            [&fired](Intent&) { fired++; });
    am.setRepeating(AlarmManager::ELAPSED_REALTIME,
                    SystemClock::elapsedRealtime() + SOON, 100, pi);
    pumpFor(360);          /* SOON=60 + 3x100 grid points inside 360ms */
    EXPECT_GE(fired, 2);
    int count = fired;
    am.cancel(pi);
    pumpFor(250);
    EXPECT_EQ(count, fired);  /* no delivery after cancel */
    delete pi;
}

TEST(AlarmManagerTest, testSetAlarmClock) {
    AlarmManager& am = AlarmManager::getInstance();
    int fired = 0;
    Intent intent;
    PendingIntent* pi = PendingIntent::getBroadcast(nullptr, 0, &intent, 0,
            [&fired](Intent&) { fired++; });
    const int64_t trigger = SystemClock::currentTimeMillis() + SOON;
    AlarmManager::AlarmClockInfo info(trigger, nullptr);
    am.setAlarmClock(&info, pi);

    auto* next = am.getNextAlarmClock();
    ASSERT_NE(nullptr, next);
    EXPECT_EQ(trigger, next->getTriggerTime());

    pumpFor(PUMP);
    EXPECT_EQ(1, fired);
    EXPECT_EQ(nullptr, am.getNextAlarmClock());  /* cleared once fired */
    delete pi;
}

TEST(AlarmManagerTest, testGetNextWakeFromIdleTime) {
    AlarmManager& am = AlarmManager::getInstance();
    AlarmManager::OnAlarmListener listener = []() {};
    const int64_t at = SystemClock::elapsedRealtime() + 300;
    am.set(AlarmManager::ELAPSED_REALTIME_WAKEUP, at, "wake", listener, nullptr);
    const int64_t next = am.getNextWakeFromIdleTime();
    EXPECT_GT(next, SystemClock::elapsedRealtime());
    EXPECT_LE(next, at + 5);
    am.cancel(listener);
    EXPECT_EQ(LLONG_MAX, am.getNextWakeFromIdleTime());
}

TEST(AlarmManagerTest, testCanScheduleExactAlarmsAndAllowWhileIdle) {
    /* no permission subsystem: exact alarms are always allowed */
    EXPECT_TRUE(AlarmManager::getInstance().canScheduleExactAlarms());
    /* without doze the AllowWhileIdle variants behave like set/setExact */
    AlarmManager& am = AlarmManager::getInstance();
    int fired = 0;
    Intent intent;
    PendingIntent* pi = PendingIntent::getBroadcast(nullptr, 0, &intent, 0,
            [&fired](Intent&) { fired++; });
    am.setAndAllowWhileIdle(AlarmManager::ELAPSED_REALTIME,
                            SystemClock::elapsedRealtime() + SOON, pi);
    am.setExactAndAllowWhileIdle(AlarmManager::ELAPSED_REALTIME,
                                 SystemClock::elapsedRealtime() + SOON, pi);
    pumpFor(PUMP);
    /* second set replaces the first (same PendingIntent backs one alarm) */
    EXPECT_EQ(1, fired);
    delete pi;
}

TEST(AlarmManagerTest, testSetTimeZone) {
    /* empty string is a silent no-op (TextUtils.isEmpty branch) */
    AlarmManager::getInstance().setTimeZone("");
    SUCCEED();
}
