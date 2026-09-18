/* Sample for android.app.AlarmManager (app/alarmmanager.{h,cc}).
 *
 * Exercises the OnAlarmListener variants, the PendingIntent variants
 * (in-process send, see app/pendingintent.h), setRepeating and setAlarmClock.
 * On a kernel with the alarmtimer framework (CONFIG_ALARMTIMER + wake RTC)
 * the *_WAKEUP entries also fire out of suspend; otherwise they degrade to
 * CLOCK_BOOTTIME and fire after the next resume (the banner shows which).
 */
#include <cdroid.h>
#include <cdlog.h>
#include <app/alarmmanager.h>
#include <app/pendingintent.h>
#include <core/intent.h>
#include <core/systemclock.h>
#include <widget/linearlayout.h>
#include <widget/scrollview.h>
#include <widget/textview.h>
#include <widget/button.h>

using namespace cdroid;

class AlarmSample : public Window {
private:
    TextView* mLog;
    std::string mLogText;
    int mSeq = 0;
    PendingIntent* mRepeatPi = nullptr;
    PendingIntent* mClockPi = nullptr;
public:
    AlarmSample(int w, int h) : Window(0, 0, w, h) {
        LinearLayout* box = new LinearLayout(mContext);
        box->setOrientation(LinearLayout::VERTICAL);

        mLog = new TextView(mContext);
        mLog->setTextSize(18);
        mLog->setPadding(20, 20, 20, 20);

        /* Which clock each timer fd got (and whether it can break suspend)
         * is logged by the manager at first use; see the AlarmManager lines. */
        log("AlarmManager sample ready");

        addButton(box, "set ELAPSED_REALTIME +5s (listener)", [this](View&) {
            AlarmManager::getInstance().set(AlarmManager::ELAPSED_REALTIME,
                    SystemClock::elapsedRealtime() + 5000, "elapsed",
                    makeListener("set/ELAPSED"), nullptr);
        });
        addButton(box, "set ELAPSED_REALTIME_WAKEUP +5s (listener)", [this](View&) {
            AlarmManager::getInstance().set(AlarmManager::ELAPSED_REALTIME_WAKEUP,
                    SystemClock::elapsedRealtime() + 5000, "elapsed-wakeup",
                    makeListener("set/ELAPSED_WAKEUP"), nullptr);
        });
        addButton(box, "set RTC_WAKEUP +5s (listener)", [this](View&) {
            AlarmManager::getInstance().set(AlarmManager::RTC_WAKEUP,
                    SystemClock::currentTimeMillis() + 5000, "rtc",
                    makeListener("set/RTC_WAKEUP"), nullptr);
        });
        addButton(box, "setExact ELAPSED +3s (listener)", [this](View&) {
            AlarmManager::getInstance().setExact(AlarmManager::ELAPSED_REALTIME,
                    SystemClock::elapsedRealtime() + 3000, "exact",
                    makeListener("setExact"), nullptr);
        });
        addButton(box, "setRepeating ELAPSED every 2s (PendingIntent)", [this](View&) {
            Intent intent;
            intent.setAction("cdroid.sample.REPEAT");
            delete mRepeatPi;
            mRepeatPi = PendingIntent::getBroadcast(mContext, 0, &intent, 0,
                    [this](Intent&) { log("fired: repeating PendingIntent"); });
            AlarmManager::getInstance().setRepeating(AlarmManager::ELAPSED_REALTIME,
                    SystemClock::elapsedRealtime() + 2000, 2000, mRepeatPi);
        });
        addButton(box, "cancel repeating", [this](View&) {
            if (mRepeatPi) AlarmManager::getInstance().cancel(mRepeatPi);
        });
        addButton(box, "setAlarmClock RTC +8s", [this](View&) {
            Intent intent;
            intent.setAction("cdroid.sample.ALARM_CLOCK");
            delete mClockPi;
            mClockPi = PendingIntent::getBroadcast(mContext, 0, &intent, 0,
                    [this](Intent&) { log("fired: alarm clock"); });
            AlarmManager::AlarmClockInfo info(SystemClock::currentTimeMillis() + 8000, mClockPi);
            AlarmManager::getInstance().setAlarmClock(&info, mClockPi);
            auto* next = AlarmManager::getInstance().getNextAlarmClock();
            log(next ? "next alarm clock armed" : "getNextAlarmClock: none");
        });

        ScrollView* scroll = new ScrollView(mContext);
        scroll->addView(mLog);
        box->addView(scroll, new LinearLayout::LayoutParams(-1, -1, 1.f));
        addView(box);
    }

private:
    void addButton(LinearLayout* box, const std::string& text, View::OnClickListener click) {
        Button* b = new Button(mContext);
        b->setText(text);
        b->setOnClickListener(click);
        box->addView(b, new LinearLayout::LayoutParams(-1, -2));
    }

    AlarmManager::OnAlarmListener makeListener(const std::string& what) {
        std::string label = what + " #" + std::to_string(++mSeq);
        return [this, label]() { log("fired: " + label); };
    }

    void log(const std::string& line) {
        LOGI("AlarmSample: %s", line.c_str());
        mLogText += line + "\n";
        mLog->setText(mLogText);
    }
};

int main(int argc, const char* argv[]) {
    App app(argc, argv);
    Window* w = new AlarmSample(720, 720);
    w->setBackgroundColor(0xFF101418);
    return app.exec();
}
