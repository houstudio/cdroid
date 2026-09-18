#ifndef __DESKCLOCK_SETTINGSACTIVITY_H__
#define __DESKCLOCK_SETTINGSACTIVITY_H__
/*********************************************************************************
 * Port of com.android.deskclock.settings.SettingsActivity — a FragmentActivity
 * hosting PrefsFragment. The custom SimpleMenuPreference/AlarmVolumePreference
 * widgets are not ported (their rows degrade to ListPreference dialogs / are
 * dropped); keys mirror the SettingsDAO constants the DataModel reads.
 *********************************************************************************/
#include <fragment/fragmentactivity.h>

#include <actionbarmenu.h>
#include <dropshadowcontroller.h>

namespace cdroid {
namespace deskclock {

class SettingsActivity : public FragmentActivity {
private:
    actionbarmenu::OptionsMenuManager mOptionsMenuManager;
    DropShadowController* mDropShadowController = nullptr;

public:
    SettingsActivity();
    ~SettingsActivity() override;

    void onCreate(Bundle* savedInstanceState) override;
    void onResume() override;
    void onPause() override;
    bool onCreateOptionsMenu(Menu& menu) override;
    bool onPrepareOptionsMenu(Menu& menu) override;
    bool onOptionsItemSelected(MenuItem& item) override;
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_SETTINGSACTIVITY_H__
