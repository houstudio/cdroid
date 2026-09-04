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

    return app.exec();
}
