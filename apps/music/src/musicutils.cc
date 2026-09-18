#include <musicutils.h>

#include <cstdio>
#include <cstdint>

#include <R.h>
#include <core/app.h>
#include <core/intent.h>
#include <core/context.h>
#include <content/sharedpreferences.h>
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <porting/cdlog.h>
#include <widget/cdwindow.h>
#include <widget/tabwidget.h>
#include <widget/textview.h>
#include <widget/toast.h>

#include <mediaplaybackservice.h>
#include <musicdb.h>

// R.h declares ::music::R (generated from the manifest package cdroid.music).
using namespace ::music;

namespace cdroid {
namespace music {

// aapt2 generates one R.h for the app; the plurals used by the labels
// live under ::music::R::plurals (Nsongs/Nalbums in strings.xml).
static void processTabClick(Window& a, View* v, int current);

std::string makeAlbumsLabel(Context& c, int numalbums, int numsongs, bool isUnknown) {
    // "N songs" for unknown artist/album, "N albums\n" for known ones.
    std::string out;
    Resources& r = c.getResources();
    if (isUnknown) {
        if (numsongs == 1) {
            out = c.getString(R::string::onesong);
        } else {
            out = r.getQuantityString(R::plurals::Nsongs, numsongs,
                    {std::to_string(numsongs)});
        }
    } else {
        out = r.getQuantityString(R::plurals::Nalbums, numalbums,
                {std::to_string(numalbums)});
        out += c.getString(R::string::albumsongseparator);
    }
    return out;
}

std::string makeTimeString(Context& c, long secs) {
    const int id = secs < 3600 ? R::string::durationformatshort
                               : R::string::durationformatlong;
    const std::string fmt = c.getString(id);
    // timeArgs order matches the original: h, h+m, m%60, s, s%60
    long args[5] = {secs / 3600, secs / 60, (secs / 60) % 60, secs, secs % 60};
    char buf[64];
    // glibc printf handles the %n$ positional specifiers in the XML format
    snprintf(buf, sizeof(buf), fmt.c_str(), args[0], args[1], args[2], args[3], args[4]);
    return buf;
}

int getIntPref(Window& a, const std::string& name, int def) {
    auto prefs = a.getContext()->getSharedPreferences(a.getContext()->getPackageName(),
            Context::MODE_PRIVATE);
    return prefs ? prefs->getInt(name, def) : def;
}

void setIntPref(Window& a, const std::string& name, int value) {
    auto prefs = a.getContext()->getSharedPreferences(a.getContext()->getPackageName(),
            Context::MODE_PRIVATE);
    if (!prefs) return;
    SharedPreferences::Editor& ed = prefs->edit();
    ed.putInt(name, value);
    ed.commit();
}

static TabWidget* buttonBar(Window& a) {
    return dynamic_cast<TabWidget*>(a.findViewById(R::id::buttonbar));
}

bool updateButtonBar(Window& a, int highlight) {
    TabWidget* ll = buttonBar(a);
    if (ll == nullptr) return false;
    const bool withtabs = a.getIntent().getBooleanExtra("withtabs", false);

    if (highlight == 0 || !withtabs) {
        ll->setVisibility(View::GONE);
        return withtabs;
    }
    ll->setVisibility(View::VISIBLE);

    for (int i = ll->getChildCount() - 1; i >= 0; i--) {
        View* v = ll->getChildAt(i);
        if (v == nullptr) continue;
        const bool isActive = (v->getId() == highlight);
        if (isActive) ll->setCurrentTab(i);
        v->setTag((void*)(intptr_t)i);
        v->setOnClickListener([&a, v, ll](View&) {
            View* cur = ll->getChildAt((int)(intptr_t)v->getTag());
            processTabClick(a, v, cur ? cur->getId() : 0);
        });
    }
    return withtabs;
}

static void processTabClick(Window& a, View* v, int current) {
    const int id = v->getId();
    if (id == current) return;
    TabWidget* ll = buttonBar(a);
    activateTab(a, id);
    if (id != R::id::nowplayingtab && ll != nullptr) {
        ll->setCurrentTab((int)(intptr_t)v->getTag());
        setIntPref(a, "activetab", id);
    }
}

void activateTab(Window& a, int id) {
    std::string cls;
    if (id == R::id::artisttab) {
        cls = "ArtistAlbumBrowserActivity";
    } else if (id == R::id::albumtab) {
        cls = "AlbumBrowserActivity";
    } else if (id == R::id::songtab) {
        cls = "TrackBrowserActivity";
    } else if (id == R::id::playlisttab) {
        cls = "PlaylistBrowserActivity";
    } else if (id == R::id::nowplayingtab) {
        Intent intent;
        intent.setClassName("cdroid.music", "MediaPlaybackActivity");
        App::getInstance().startActivity(intent);
        return;
    } else {
        return;
    }
    Intent intent;
    intent.setClassName("cdroid.music", cls)
          .putExtra("withtabs", true)
          .addFlags(Intent::FLAG_ACTIVITY_CLEAR_TOP);
    App::getInstance().startActivity(intent);
    a.close();
}

void updateNowPlaying(Window& a) {
    View* nowPlayingView = a.findViewById(R::id::nowplaying);
    if (nowPlayingView == nullptr) return;
    MediaPlaybackService& s = MediaPlaybackService::getInstance();
    if (s.isInitialized() && s.getAudioId() != -1) {
        TextView* title = dynamic_cast<TextView*>(
                nowPlayingView->findViewById(R::id::title));
        TextView* artist = dynamic_cast<TextView*>(
                nowPlayingView->findViewById(R::id::artist));
        if (title) title->setText(s.getTrackName());
        if (artist) artist->setText(s.getArtistName());
        nowPlayingView->setVisibility(View::VISIBLE);
        nowPlayingView->setOnClickListener([](View&) {
            Intent intent;
            intent.setClassName("cdroid.music", "MediaPlaybackActivity");
            App::getInstance().startActivity(intent);
        });
        return;
    }
    nowPlayingView->setVisibility(View::GONE);
}

void addBackButton(Window& a) {
    // Window is a FrameLayout, so a gravity-positioned child floats over the
    // inflated content without touching the original layouts.
    auto* back = new TextView(a.getContext());
    back->setText("←");
    back->setTextSize(26);
    back->setTextColor(0xffffffff);
    back->setBackgroundColor(0x88000000);
    back->setPadding(26, 6, 26, 6);
    back->setClickable(true);
    back->setOnClickListener([&a](View&) { a.close(); });
    auto* lp = new FrameLayout::LayoutParams(
            ViewGroup::LayoutParams::WRAP_CONTENT, ViewGroup::LayoutParams::WRAP_CONTENT,
            Gravity::LEFT | Gravity::TOP);
    lp->leftMargin = 10;
    lp->topMargin = 10;
    a.addView(back, lp);
}

void playAll(Window& a, const std::vector<long>& list, int position, bool forceShuffle) {
    if (list.empty()) {
        Toast::makeText(a.getContext(), a.getContext()->getString(
                R::string::emptyplaylist), Toast::LENGTH_SHORT)->show();
        return;
    }
    MediaPlaybackService& s = MediaPlaybackService::getInstance();
    if (forceShuffle) s.setShuffleMode(MediaPlaybackService::SHUFFLE_NORMAL);
    const long curid = s.getAudioId();
    const int curpos = s.getQueuePosition();
    if (position != -1 && curpos == position && curid == list[position]) {
        // selected file is already the current one — just resume if needed
        if (s.getQueue() == list) {
            s.play();
        }
    } else {
        if (position < 0) position = 0;
        s.open(list, forceShuffle ? -1 : position);
        s.play();
    }
    Intent intent;
    intent.setClassName("cdroid.music", "MediaPlaybackActivity")
          .setAction("com.android.music.PLAYBACK_VIEWER");
    App::getInstance().startActivity(intent);
}

void shuffleAll(Window& a) {
    playAll(a, MusicDB::get().allSongIds(), 0, true);
}

void togglePartyShuffle() {
    MediaPlaybackService& s = MediaPlaybackService::getInstance();
    s.setShuffleMode(s.getShuffleMode() == MediaPlaybackService::SHUFFLE_AUTO
            ? MediaPlaybackService::SHUFFLE_NONE
            : MediaPlaybackService::SHUFFLE_AUTO);
}

void setPartyShuffleMenuIcon(Menu& menu) {
    MenuItem* item = menu.findItem(Defs::PARTY_SHUFFLE);
    if (item == nullptr) return;
    const int shuffle = MediaPlaybackService::getInstance().getShuffleMode();
    if (shuffle == MediaPlaybackService::SHUFFLE_AUTO) {
        item->setIcon(R::drawable::ic_menu_party_shuffle);
        item->setTitle(App::getInstance().getString(R::string::party_shuffle_off));
    } else {
        item->setIcon(R::drawable::ic_menu_party_shuffle);
        item->setTitle(App::getInstance().getString(R::string::party_shuffle));
    }
}

} // namespace music
} // namespace cdroid
