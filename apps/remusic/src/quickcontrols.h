// Port of com.wm.remusic.fragment.QuickControlsFragment — the bottom
// mini-player bar (bottom_nav.xml) the original embeds through BaseActivity.
// CDROID equivalent: a plain controller that inflates bottom_nav.xml into the
// hosting activity's bottom_container and wires itself to the playback
// service listener bus.
#ifndef __REMUSIC_QUICKCONTROLS_H__
#define __REMUSIC_QUICKCONTROLS_H__

#include <string>

#include <cdroid.h>

namespace remusic {

// TimingFragment, faithful port: the fixed sleep-timer presets as a bottom
// sheet appended to `host` (any activity). Shared by the playing screen's
// overflow (⋮) and the drawer's 定时关闭 entry.
void showTimingSheet(cdroid::ViewGroup& host);

class QuickControls {
public:
    static QuickControls& get();
    void attachTo(cdroid::Window& host);   // idempotent per host

private:
    void updateInfo();
    cdroid::Window* mHost = nullptr;
};

} // namespace remusic
#endif
