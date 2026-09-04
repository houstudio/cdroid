#ifndef __DESKCLOCK_SCREENSAVERSETTINGSACTIVITY_H__
#define __DESKCLOCK_SCREENSAVERSETTINGSACTIVITY_H__
/*********************************************************************************
 * Port of com.android.deskclock.settings.ScreensaverSettingsActivity — the one
 * DeskClock screen whose layout (screensaver_settings.xml) hosts its fragment
 * through the <fragment> XML tag: setContentView-time inflation creates
 * PrefsFragment in place (FragmentManager::onCreateView via the LayoutInflater
 * private factory), unlike SettingsActivity's code-side replace(R.id.main).
 *********************************************************************************/
#include <fragment/fragmentactivity.h>

namespace cdroid {
namespace deskclock {

class ScreensaverSettingsActivity : public FragmentActivity {
public:
    ScreensaverSettingsActivity();

    void onCreate(Bundle* savedInstanceState) override;
    bool onOptionsItemSelected(MenuItem& item) override;
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_SCREENSAVERSETTINGSACTIVITY_H__
