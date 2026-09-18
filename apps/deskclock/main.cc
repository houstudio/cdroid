/*********************************************************************************
 * CDROID port of the AOSP DeskClock app (packages/apps/DeskClock, kotlin flavor) —
 * a UI-fidelity facade: original layouts/drawables/strings, in-process data
 * model (SharedPreferences-backed DAOs), Handler-scheduled alarm/timer expiry
 * instead of AlarmManager + notifications. Entry point mirrors the launcher
 * intent: singleTask DeskClock.
 *********************************************************************************/
#include <cdroid.h>

#include <R.h>
#include <core/cxxopts.h>
#include <core/intent.h>

#include <alarmstatemanager.h>
#include <datamodel.h>
#include <uidata.h>

#include <csignal>
#include <execinfo.h>
#include <unistd.h>

// TEMP INSTRUMENTATION: apport eats the core dumps; print the crash backtrace
// straight to stderr instead.
static void segvTrace(int sig) {
    void* frames[40];
    int n = backtrace(frames, 40);
    backtrace_symbols_fd(frames, n, STDERR_FILENO);
    _exit(128 + sig);
}

// Registered at static init, before App's ctor parses argv (see imageview sample).
// Dev hook: launch the screensaver settings screen directly — the one DeskClock
// layout that hosts its fragment through the <fragment> XML tag.
static const bool sAppOptionsRegistered = cdroid::App::addAppOptions("DeskClock",
        [](cxxopts::OptionAdder& add){
            add("screensaver-settings","open the screensaver settings screen",
                cxxopts::value<bool>()->implicit_value("1"));
        });

int main(int argc, const char* argv[]) {
    signal(SIGSEGV, segvTrace);   // TEMP INSTRUMENTATION
    cdroid::App app(argc, argv);

    // DeskClockApplication.onCreate: initialize the data and ui-data models.
    auto prefs = app.getSharedPreferences("DeskClock", 0 /* MODE_PRIVATE */);
    cdroid::deskclock::data::DataModel::getDataModel().init(app, *prefs);
    cdroid::deskclock::uidata::UiDataModel::getUiDataModel().init(app, *prefs);

    // Launch through the ActivityFactory registry (REGISTER_ACTIVITY keys),
    // the same path the manifest launcher intent resolves to.
    cdroid::Intent intent;
    intent.setClassName("cdroid.deskclock",
            app.hasSwitch("screensaver-settings") ? "ScreensaverSettingsActivity" : "DeskClock")
          .setAction(cdroid::Intent::ACTION_MAIN);
    app.startActivity(intent);

    // BOOT_COMPLETED stand-in (upstream: TimerReceiver/AlarmNotifications pick
    // this up as a broadcast after the process starts). CDROID has no system
    // AlarmManager, so the expiry schedule dies with the process — without this
    // reconciliation a timer that came due while the process was down would sit
    // RUNNING with a negative remaining time and no expiry posted, counting
    // negative forever. updateTimersAfterReboot re-bases the timers and re-arms
    // the next expiry (past-due ones fire immediately; far-past-due ones miss()).
    static cdroid::Handler sBootHandler(cdroid::Looper::getMainLooper());
    sBootHandler.postDelayed([]() {
        auto& dm = cdroid::deskclock::data::DataModel::getDataModel();
        dm.updateAfterReboot();
        // AlarmInitReceiver's alarm half: reconcile instance states with the
        // wall clock that moved while the process was down, then (re)arm the
        // next state transition — the in-process schedule died with the
        // process, so a seeded/restored FIRE-able instance never fires
        // without this.
        cdroid::deskclock::alarms::AlarmStateManager::fixAlarmInstances(
                cdroid::App::getInstance());
        // Timers that expired in a previous session (or just now via the re-armed
        // schedule) surface through the takeover activity — the in-process
        // stand-in for upstream's persistent heads-up notification, which keeps
        // reappearing until the expired timers are handled.
        if (!dm.getExpiredTimers().empty()) {
            cdroid::Intent takeover;
            takeover.setClassName("cdroid.deskclock", "ExpiredTimersActivity")
                    .setAction(cdroid::Intent::ACTION_MAIN)
                    .setFlags(cdroid::Intent::FLAG_ACTIVITY_NEW_TASK);
            cdroid::App::getInstance().startActivity(takeover);
        }
    }, 2000);  // let the launcher window come up underneath first

    return app.exec();
}
