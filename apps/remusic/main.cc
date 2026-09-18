/*********************************************************************************
 * CDROID port of com.wm.remusic — a 2016 NetEase-Cloud-Music-styled local music
 * player (originally Baidu-ting-backed; online path trimmed offline here).
 * UI-fidelity port: original layouts/drawables/strings, in-process playback
 * service (queue/shuffle/repeat/history persisted via SharedPreferences).
 *********************************************************************************/
#include <cdroid.h>
#include <csignal>
#include <execinfo.h>
#include <unistd.h>

// apport eats core dumps; print the crash backtrace straight to stderr.
static void segvTrace(int sig) {
    void* frames[40];
    const int n = backtrace(frames, 40);
    backtrace_symbols_fd(frames, n, STDERR_FILENO);
    _exit(128 + sig);
}

#include <R.h>
#include <core/cxxopts.h>
#include <core/intent.h>

#include "src/musicplayer.h"
#include "src/playliststore.h"
#include "src/themestore.h"

// Registered at static init, before App's ctor parses argv (deskclock pattern).
// Dev/test hook: skip the library and land on the now-playing screen directly.
static const bool sAppOptionsRegistered = cdroid::App::addAppOptions("remusic",
        [](cxxopts::OptionAdder& add){
            add("open-playing","launch straight into the now-playing screen",
                cxxopts::value<bool>()->implicit_value("1"));
        });

int main(int argc, const char* argv[]) {
    signal(SIGSEGV, segvTrace);
    signal(SIGABRT, segvTrace);
    cdroid::App app(argc, argv);

    // MainApplication.onCreate equivalent: wire the playback singleton.
    remusic::MusicPlayer::init(app);
    remusic::PlaylistStore::get().init(&app);
    remusic::ThemeStore::get().init(&app);

    // Launch through the ActivityFactory registry (REGISTER_ACTIVITY keys),
    // the same path the manifest launcher intent resolves to.
    cdroid::Intent intent;
    intent.setClassName("cdroid.remusic",
            app.hasSwitch("open-playing") ? "PlayingActivity" : "MainActivity")
          .setAction(cdroid::Intent::ACTION_MAIN);
    app.startActivity(intent);

    return app.exec();
}
