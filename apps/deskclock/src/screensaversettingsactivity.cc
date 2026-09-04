#include <screensaversettingsactivity.h>

#include <R.h>

#include <any>

#include <core/activityfactory.h>
#include <fragment/fragmentfactory.h>
#include <preference/listpreference.h>
#include <preference/preference.h>
#include <preference/preferencefragment.h>
#include <view/layoutinflater.h>
#include <widget/cdwindow.h>
#include <widget/R.h>

#include <settingsdao.h>
#include <utils.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {

//
// PrefsFragment (upstream: ScreensaverSettingsActivity.PrefsFragment)
//

namespace {
class PrefsFragment : public PreferenceFragment {
public:
    void onCreatePreferences(Bundle* savedInstanceState, const std::string& rootKey) override {
        (void) savedInstanceState;
        (void) rootKey;

        // Persist into the same store the DataModel was initialized with
        // (upstream uses device-protected default shared preferences).
        getPreferenceManager()->setSharedPreferencesName("DeskClock");
        addPreferencesFromResource(::deskclock::R::xml::screensaver_settings);
    }

    void onResume() override {
        PreferenceFragment::onResume();
        refresh();
    }

private:
    // Upstream onPreferenceChange: keep the clock-style summary in sync with the value.
    bool onPreferenceChange(Preference& pref, const any& newValue) {
        if (data::SCREENSAVER_KEY_CLOCK_STYLE == pref.getKey()) {
            ListPreference* clockStylePref = (ListPreference*) &pref;
            const int index = clockStylePref->findIndexOfValue(any_cast<std::string>(newValue));
            clockStylePref->setSummary(clockStylePref->getEntries()[index]);
        }
        return true;
    }

    void refresh() {
        ListPreference* clockStylePref =
                (ListPreference*) findPreference(data::SCREENSAVER_KEY_CLOCK_STYLE);
        if (clockStylePref != nullptr) {
            clockStylePref->setSummary(clockStylePref->getEntry());
            clockStylePref->setOnPreferenceChangeListener(
                    [this](Preference& pref, const any& newValue) {
                        return onPreferenceChange(pref, newValue);
                    });
        }
    }
};
} // namespace

//
// ScreensaverSettingsActivity
//

ScreensaverSettingsActivity::ScreensaverSettingsActivity() : FragmentActivity(0, 0, -1, -1) {
}

void ScreensaverSettingsActivity::onCreate(Bundle* savedInstanceState) {
    FragmentActivity::onCreate(savedInstanceState);

    // The <fragment android:name=...> tag in this layout is inflated in place by the
    // FragmentManager's layout factory — PrefsFragment is created during this inflate.
    View* content = LayoutInflater::from(getContext())->inflate(
            ::deskclock::R::layout::screensaver_settings, nullptr, false);
    // Opaque base layer: without one, SRC_OVER smears whatever was under the window.
    Utils::setDefaultBackground(content);
    addView(content);
}

bool ScreensaverSettingsActivity::onOptionsItemSelected(MenuItem& item) {
    if (item.getItemId() == cdroid::R::id::home) {
        close();
        return true;
    }
    return FragmentActivity::onOptionsItemSelected(item);
}

// Registered under the name the layout's <fragment android:name> carries
// (screensaver_settings.xml); must match that string exactly.
static const int _cdroid_frag_reg_screensaver_prefs =
    (::cdroid::FragmentFactory::registerFragment(
         "DeskClocksettings.ScreensaverSettingsActivity$PrefsFragment",
         []() -> ::cdroid::Fragment* { return new PrefsFragment(); }), 0);

// Registered under the bare class name the Intent ComponentName carries
// (REGISTER_ACTIVITY would stringify the qualified name).
static const int _cdroid_act_reg_screensaversettings =
    (::cdroid::ActivityFactory::registerActivity("ScreensaverSettingsActivity",
        []() -> ::cdroid::Window* {
            ::cdroid::Window* w = new ::cdroid::deskclock::ScreensaverSettingsActivity();
            w->setActivityName("ScreensaverSettingsActivity");
            return w;
        }), 0);

} // namespace deskclock
} // namespace cdroid
