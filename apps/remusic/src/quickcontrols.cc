#include "quickcontrols.h"

#include <R.h>
#include <core/intent.h>
#include <core/uri.h>
#include <widget/imageview.h>
#include <widget/progressbar.h>
#include <widget/textview.h>

#include "musicplayer.h"
#include "themestore.h"

using namespace cdroid;
using namespace remusic;

QuickControls& QuickControls::get() {
    static QuickControls instance;
    return instance;
}

void QuickControls::attachTo(Window& host) {
    if (mHost == &host) { updateInfo(); return; }
    mHost = &host;

    auto* container = (ViewGroup*) host.findViewById(R::id::bottom_container);
    if (container == nullptr) return;
    container->removeAllViews();
    auto* bar = LayoutInflater::from(host.getContext())->inflate(R::layout::bottom_nav, container, true);
    if (bar == nullptr) return;
    // bottom_nav's root is layout_height=match_parent with a translucent
    // background: inside the 50dp bottom_container it would still swallow
    // every touch on the whole screen (the frame does not clip hit-testing).
    // Pin it to the container's own height instead.
    if (auto* clp = container->getLayoutParams()) {
        auto* lp = bar->getLayoutParams();
        if (lp) lp->height = clp->height;
    }

    auto* title = (TextView*) host.findViewById(R::id::playbar_info);
    auto* artist = (TextView*) host.findViewById(R::id::playbar_singer);
    auto* control = (ImageView*) host.findViewById(R::id::control);
    auto* next = (ImageView*) host.findViewById(R::id::play_next);

    control->setOnClickListener([](View&) { MusicPlayer::playOrPause(); });
    next->setOnClickListener([](View&) { MusicPlayer::next(); });
    // LaunchNowPlayingReceiver: tapping anywhere on the bar (children included
    // — a child without its own listener never forwards a click to the parent)
    // opens the now-playing screen.
    auto openPlaying = [](View& v) {
        Intent intent;
        intent.setClassName("cdroid.remusic", "PlayingActivity")
              .setAction("com.wm.remusic.LAUNCH_NOW_PLAYING_ACTION");
        App::getInstance().startActivity(intent);
    };
    bar->setOnClickListener(openPlaying);
    if (auto* img = host.findViewById(R::id::playbar_img)) img->setOnClickListener(openPlaying);
    if (title) title->setOnClickListener(openPlaying);
    if (artist) artist->setOnClickListener(openPlaying);

    // PlaybackStatus receiver: refresh on meta/playstate changes; open
    // failures surface as a warning in the bar (TRACK_ERROR).
    MediaPlaybackService::getInstance().addListener(this, [this, title, artist, control](const std::string& what) {
        if (what == MediaServiceActions::TRACK_ERROR) {
            if (title != nullptr)
                title->setText("⚠ 无法播放:" + MusicPlayer::getTrackName()
                        + "(换台/换曲试试)");
            return;
        }
        if (what != MediaServiceActions::META_CHANGED
                && what != MediaServiceActions::PLAYSTATE_CHANGED) return;
        updateInfo();
    });
    updateInfo();
}

void QuickControls::updateInfo() {
    if (mHost == nullptr) return;
    if (auto* title = (TextView*) mHost->findViewById(R::id::playbar_info))
        title->setText(MusicPlayer::getTrackName().empty() ? "remusic" : MusicPlayer::getTrackName());
    if (auto* artist = (TextView*) mHost->findViewById(R::id::playbar_singer))
        artist->setText(MusicPlayer::getArtistName());
    if (auto* img = (ImageView*) mHost->findViewById(R::id::playbar_img)) {
        const MusicInfo* info = MusicPlayer::currentTrackInfo();
        const std::string art = info ? info->albumData : std::string();
        if (!art.empty()) {
            img->setImageURIAsync("file://" + art);
        } else {
            img->setImageResource(R::drawable::placeholder_disk_210);
        }
    }
    if (auto* bar = mHost->findViewById(R::id::nav_play))
        bar->setBackgroundColor(ThemeStore::get().accent());
    if (auto* control = (ImageView*) mHost->findViewById(R::id::control)) {
        // The original swaps drawable ids between play/pause states (pause art
        // ships only in some builds — reuse play for now).
        control->setImageResource(R::drawable::playbar_btn_play);
    }
}
