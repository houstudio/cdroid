/*********************************************************************************
 * Subset port of com.android.music.MusicUtils.
 *
 * Kept: the Defs menu ids, the label/time formatters, the tab-strip (buttonbar)
 * wiring and the playAll entry point. Replaced: every ContentResolver/MediaStore
 * query (→ MusicDB), service binding (→ MediaPlaybackService singleton) and
 * the album-art bitmap cache (→ the static default artwork only).
 *********************************************************************************/
#ifndef CDROID_MUSIC_MUSICUTILS_H
#define CDROID_MUSIC_MUSICUTILS_H

#include <string>
#include <vector>

#include <R.h>

namespace cdroid {
class Context;
class Window;
class Menu;
class MenuItem;

namespace music {

// MusicUtils.Defs — menu item ids used by every screen.
struct Defs {
    enum {
        OPEN_URL = 0,
        ADD_TO_PLAYLIST = 1,
        USE_AS_RINGTONE = 2,
        PLAYLIST_SELECTED = 3,
        NEW_PLAYLIST = 4,
        PLAY_SELECTION = 5,
        GOTO_START = 6,
        GOTO_PLAYBACK = 7,
        PARTY_SHUFFLE = 8,
        SHUFFLE_ALL = 9,
        DELETE_ITEM = 10,
        SCAN_DONE = 11,
        QUEUE = 12,
        EFFECTS_PANEL = 13,
        CHILD_MENU_BASE = 14,
        SEARCH = CHILD_MENU_BASE,
        // TrackBrowserActivity locals (CHILD_MENU_BASE + n, as in the original)
        SAVE_AS_PLAYLIST = CHILD_MENU_BASE + 3,
        CLEAR_PLAYLIST = CHILD_MENU_BASE + 4,
        REMOVE = CHILD_MENU_BASE + 5,
        USE_AS_RINGTONE_LOCAL = CHILD_MENU_BASE + 6,
    };
};

std::string makeAlbumsLabel(Context& c, int numalbums, int numsongs, bool isUnknown);

// "%2$d:%5$02d"-style strings from strings.xml (positional printf args).
std::string makeTimeString(Context& c, long secs);

int getIntPref(Window& a, const std::string& name, int def);
void setIntPref(Window& a, const std::string& name, int value);

// Wires the TabWidget tab strip (R.id.buttonbar) inside a browser window:
// visibility per the "withtabs" intent extra, current-tab highlight, click
// → activateTab. Returns whether the tab strip is shown.
bool updateButtonBar(Window& a, int highlight);

// Tab switch: starts the target browser window ("withtabs" extra) and finishes
// the current one, like the original activity-swap tabs.
void activateTab(Window& a, int id);

// Bottom "now playing" bar on the browser screens: shows the current track
// and opens MediaPlaybackActivity on tap; hidden when nothing is loaded.
void updateNowPlaying(Window& a);

// On-screen back affordance for drilled-in screens (no keyboard/BACK key on
// the embedded targets — the repo's appliance apps all carry a page-local
// back button). Floats a "←" over the window's top-left corner; closes the
// window (i.e. returns to the screen beneath, the ESC/back-key equivalent).
void addBackButton(Window& a);

// MusicUtils.playAll: opens the list in the (fake) service, plays, and jumps
// to the playback screen.
void playAll(Window& a, const std::vector<long>& list, int position, bool forceShuffle);
void shuffleAll(Window& a);
void togglePartyShuffle();
void setPartyShuffleMenuIcon(Menu& menu);

} // namespace music
} // namespace cdroid

#endif // CDROID_MUSIC_MUSICUTILS_H
