/*********************************************************************************
 * androidx.preference port demo: a PreferenceFragment hosted in a
 * FragmentActivity, hierarchy built in code (CheckBox / Switch / EditText /
 * List / MultiSelect), persisted through SharedPreferences (the default
 * "<package>_preferences" store under ~/.cdroid/prefs/). Re-run to observe
 * values surviving restart; toggle items and watch dependency disabling.
 *********************************************************************************/
#include <cdroid.h>
#include <preference/preferencefragment.h>
#include <preference/preferencescreen.h>
#include <preference/preferencecategory.h>
#include <preference/checkboxpreference.h>
#include <preference/switchpreference.h>
#include <preference/edittextpreference.h>
#include <preference/listpreference.h>
#include <preference/multiselectlistpreference.h>
#include <fragment/fragmentactivity.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>
#include <widget/textview.h>
#include <porting/cdlog.h>

using cdroid::PreferenceFragment;
using cdroid::fragment::FragmentActivity;

class SettingsFragment : public PreferenceFragment {
public:
    void onCreatePreferences(cdroid::Bundle* /*savedInstanceState*/,
            const std::string& /*rootKey*/) override {
        cdroid::Context& ctx = *requireContext();
        cdroid::PreferenceManager* pm = getPreferenceManager();

        cdroid::PreferenceScreen* screen = pm->createPreferenceScreen(ctx);
        screen->setKey("root");

        auto* general = new cdroid::PreferenceCategory(ctx);
        general->setTitle("General");
        screen->addPreference(general);

        auto* wifi = new cdroid::CheckBoxPreference(ctx);
        wifi->setKey("wifi_enabled");
        wifi->setTitle("Wi-Fi");
        wifi->setSummary("Enable wireless networking");
        wifi->setDefaultValue(cdroid::any(true));
        general->addPreference(wifi);

        auto* dark = new cdroid::SwitchPreference(ctx);
        dark->setKey("dark_mode");
        dark->setTitle("Dark theme");
        dark->setSummaryOn("Dark theme is on");
        dark->setSummaryOff("Dark theme is off");
        general->addPreference(dark);

        auto* account = new cdroid::PreferenceCategory(ctx);
        account->setTitle("Account");
        screen->addPreference(account);

        auto* name = new cdroid::EditTextPreference(ctx);
        name->setKey("username");
        name->setTitle("User name");
        name->setSummary("Name shown on the home screen");
        account->addPreference(name);

        auto* sync = new cdroid::ListPreference(ctx);
        sync->setKey("sync_interval");
        sync->setTitle("Sync interval");
        sync->setEntries({"Never", "Hourly", "Daily"});
        sync->setEntryValues({"0", "3600", "86400"});
        sync->setDefaultValue(cdroid::any(std::string("3600")));
        sync->setSummaryProvider(cdroid::ListPreference::SimpleSummaryProvider::getInstance());
        account->addPreference(sync);

        auto* channels = new cdroid::MultiSelectListPreference(ctx);
        channels->setKey("notify_channels");
        channels->setTitle("Notifications");
        channels->setEntries({"Email", "SMS", "Push"});
        channels->setEntryValues({"email", "sms", "push"});
        account->addPreference(channels);

        setPreferenceScreen(screen);
    }
};

class PreferenceDemoWindow : public FragmentActivity {
public:
    PreferenceDemoWindow() : FragmentActivity(0, 0, -1, -1) {
        auto* fragment = new SettingsFragment();
        getSupportFragmentManager()->beginTransaction()
            ->replace(getFragmentContainerId(), fragment)
            .commit();
    }
};

int main(int argc, const char* argv[]) {
    cdroid::App app(argc, argv);
    auto* w = new PreferenceDemoWindow();
    LOGD("preference demo window created");
    return app.exec();
}
