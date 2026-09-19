/*********************************************************************************
 * Port of com.android.music.MusicBrowserActivity — the launcher trampoline.
 * Itself content-less: reads the last active tab (SharedPreferences) and
 * launches the matching browser activity, exactly like the original
 * initApp()/MusicUtils.activateTab() path. Permission request dropped (no
 * runtime permissions in CDROID; the mock library needs none).
 *********************************************************************************/
#include <R.h>
#include <core/activityfactory.h>
#include <core/app.h>
#include <core/context.h>
#include <core/intent.h>
#include <widget/cdwindow.h>

#include <musicutils.h>

using namespace ::music;

namespace cdroid {
namespace music {

class MusicBrowserActivity : public Window {
public:
    MusicBrowserActivity() : Window(0, 0, -1, -1) {}

protected:
    void onCreate(Bundle*) override {
        // opaque backdrop: the window surface is transparent by default and the
        // desktop would bleed through the dirty regions (classic Music is black)
        setBackgroundColor(0xff000000);
        int activeTab = getIntPref(*this, "activetab", R::id::albumtab);
        if (activeTab != R::id::artisttab && activeTab != R::id::albumtab
                && activeTab != R::id::songtab && activeTab != R::id::playlisttab) {
            activeTab = R::id::albumtab;
        }
        activateTab(*this, activeTab);
    }
};

REGISTER_ACTIVITY(MusicBrowserActivity);

} // namespace music
} // namespace cdroid
