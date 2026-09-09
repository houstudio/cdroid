#include <settingsactivity.h>

#include <R.h>

#include <utils.h>

#include <any>

#include <core/activityfactory.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>
#include <preference/listpreference.h>
#include <preference/preferencefragment.h>
#include <preference/twostatepreference.h>
#include <view/layoutinflater.h>
#include <widget/cdwindow.h>

#include <datamodel.h>
#include <ringtonepickeractivity.h>
#include <searchmenuitemcontroller.h>
#include <settingsdao.h>
#include <uidata.h>

using namespace ::deskclock;

namespace cdroid {
namespace deskclock {

//
// PrefsFragment
//

namespace {
class PrefsFragment : public PreferenceFragment {
public:
    void onCreatePreferences(Bundle* savedInstanceState, const std::string& rootKey) override {
        (void) rootKey;

        // Persist into the same store the DataModel was initialized with
        // (upstream uses device-protected default shared preferences).
        getPreferenceManager()->setSharedPreferencesName("DeskClock");
        addPreferencesFromResource(R::xml::settings);

        loadTimeZoneList();
    }

    void onResume() override {
        PreferenceFragment::onResume();
        refresh();
        // By default, do not recreate the DeskClock activity.
        if (getActivity() != nullptr) getActivity()->setResult(0 /* RESULT_CANCELED */);
    }

private:
    void loadTimeZoneList() {
        // Reconstruct the timezone list.
        auto now = data::DataModel::getDataModel().getCalendar();
        data::TimeZones timezones =
                data::SettingsDAO::getTimeZones(*getContext(), now->getTimeInMillis());
        ListPreference* homeTimezonePref =
                (ListPreference*) findPreference(data::KEY_HOME_TZ);
        if (homeTimezonePref != nullptr) {
            homeTimezonePref->setEntryValues(timezones.timeZoneIds);
            homeTimezonePref->setEntries(timezones.timeZoneNames);
            homeTimezonePref->setSummary(homeTimezonePref->getEntry());
            homeTimezonePref->setOnPreferenceChangeListener(
                    [this](Preference& pref, const any& newValue) {
                        return onPreferenceChange(pref, newValue);
                    });
        }
    }

    bool onPreferenceChange(Preference& pref, const any& newValue) {
        const std::string key = pref.getKey();
        if (key == data::KEY_ALARM_CRESCENDO || key == data::KEY_HOME_TZ
                || key == data::KEY_ALARM_SNOOZE || key == data::KEY_TIMER_CRESCENDO) {
            ListPreference* listPref = (ListPreference*) &pref;
            const std::string value = any_cast<std::string>(newValue);
            const int index = listPref->findIndexOfValue(value);
            const std::vector<std::string> entries = listPref->getEntries();
            if (index >= 0 && index < (int) entries.size()) {
                listPref->setSummary(entries[index]);
            }
        } else if (key == data::KEY_CLOCK_STYLE || key == data::KEY_WEEK_START
                || key == data::KEY_VOLUME_BUTTONS) {
            ListPreference* listPref = (ListPreference*) &pref;
            const std::string value = any_cast<std::string>(newValue);
            const int index = listPref->findIndexOfValue(value);
            const std::vector<std::string> entries = listPref->getEntries();
            if (index >= 0 && index < (int) entries.size()) {
                pref.setSummary(entries[index]);
            }
        } else if (key == data::KEY_CLOCK_DISPLAY_SECONDS) {
            data::DataModel::getDataModel().setDisplayClockSeconds(any_cast<bool>(newValue));
        } else if (key == data::KEY_AUTO_SILENCE) {
            updateAutoSilenceSummary((ListPreference*) &pref,
                    any_cast<std::string>(newValue));
        } else if (key == data::KEY_AUTO_HOME_CLOCK) {
            TwoStatePreference* autoPref = (TwoStatePreference*) &pref;
            Preference* homeTimeZonePref = findPreference(data::KEY_HOME_TZ);
            if (homeTimeZonePref != nullptr) {
                homeTimeZonePref->setEnabled(!autoPref->isChecked());
            }
        } else if (key == data::KEY_TIMER_VIBRATE) {
            // (timer vibration has no device face on cdroid; stored for parity)
        }

        // Set result so DeskClock knows to refresh itself.
        if (getActivity() != nullptr) getActivity()->setResult(-1 /* RESULT_OK */);
        return true;
    }

    void updateAutoSilenceSummary(ListPreference* pref, const std::string& delay) {
        const std::vector<std::string> entries = pref->getEntries();
        const std::vector<std::string> values = pref->getEntryValues();
        for (size_t i = 0; i < values.size(); i++) {
            if (values[i] == delay) {
                if (i < entries.size()) pref->setSummary(entries[i]);
                return;
            }
        }
    }

    void refresh() {
        auto wire = [this](const std::string& key) -> Preference* {
            Preference* pref = findPreference(key);
            if (pref != nullptr) {
                pref->setOnPreferenceChangeListener(
                        [this](Preference& p, const any& v) {
                            return onPreferenceChange(p, v);
                        });
            }
            return pref;
        };

        if (Preference* p = wire(data::KEY_AUTO_SILENCE)) {
            ListPreference* listPref = (ListPreference*) p;
            updateAutoSilenceSummary(listPref, listPref->getValue());
        }
        for (const std::string key : {std::string(data::KEY_CLOCK_STYLE),
                std::string(data::KEY_VOLUME_BUTTONS)}) {
            if (Preference* p = wire(key)) {
                p->setSummary(((ListPreference*) p)->getEntry());
            }
        }
        wire(data::KEY_CLOCK_DISPLAY_SECONDS);

        Preference* autoHomeClockPref = wire(data::KEY_AUTO_HOME_CLOCK);
        const bool autoHomeClockEnabled =
                autoHomeClockPref != nullptr
                && ((TwoStatePreference*) autoHomeClockPref)->isChecked();
        if (Preference* homeTzPref = findPreference(data::KEY_HOME_TZ)) {
            homeTzPref->setEnabled(autoHomeClockEnabled);
        }
        for (const std::string key : {std::string(data::KEY_ALARM_CRESCENDO),
                std::string(data::KEY_TIMER_CRESCENDO), std::string(data::KEY_ALARM_SNOOZE)}) {
            if (Preference* p = wire(key)) {
                p->setSummary(((ListPreference*) p)->getEntry());
            }
        }

        // date_time opens the system date settings (no system settings app on cdroid;
        // click consumed as no-op). timer_ringtone opens the RingtonePickerActivity
        // (upstream: startActivity(createTimerRingtonePickerIntent)).
        if (Preference* p = findPreference("date_time")) {
            p->setOnPreferenceClickListener([](Preference&) { return true; });
        }
        if (Preference* p = findPreference(data::KEY_TIMER_RINGTONE)) {
            p->setOnPreferenceClickListener([](Preference& pref) {
                Context& context = pref.getContext();
                context.startActivity(ringtone::RingtonePickerActivity::
                        createTimerRingtonePickerIntent(context));
                return true;
            });
        }
    }
};
} // namespace

//
// SettingsActivity
//

SettingsActivity::SettingsActivity() : FragmentActivity(0, 0, -1, -1) {
}

SettingsActivity::~SettingsActivity() {
    delete mDropShadowController;
}

void SettingsActivity::onCreate(Bundle* savedInstanceState) {
    FragmentActivity::onCreate(savedInstanceState);

    View* content = LayoutInflater::from(getContext())->inflate(R::layout::settings, nullptr, false);
    // Opaque base layer: without one, SRC_OVER smears whatever was under the window.
    Utils::setDefaultBackground(content);
    addView(content);

    mOptionsMenuManager.addMenuItemController(
            {new actionbarmenu::NavUpMenuItemController(this)});

    // Create the prefs fragment in code to ensure it's created before PreferenceDialogFragment.
    if (savedInstanceState == nullptr) {
        FragmentTransaction& tx = *getSupportFragmentManager()->beginTransaction();
        tx.replace(R::id::main, new PrefsFragment());
        tx.disallowAddToBackStack();
        tx.commit();
    }
}

void SettingsActivity::onResume() {
    FragmentActivity::onResume();

    View* dropShadow = findViewById(R::id::drop_shadow);
    View* hairline = findViewById(R::id::main);
    if (dropShadow != nullptr && hairline != nullptr) {
        mDropShadowController = new DropShadowController(*dropShadow,
                uidata::UiDataModel::getUiDataModel(), *hairline);
    }
}

void SettingsActivity::onPause() {
    if (mDropShadowController != nullptr) {
        mDropShadowController->stop();
        delete mDropShadowController;
        mDropShadowController = nullptr;
    }
    FragmentActivity::onPause();
}

bool SettingsActivity::onCreateOptionsMenu(Menu& menu) {
    mOptionsMenuManager.onCreateOptionsMenu(menu);
    return true;
}

bool SettingsActivity::onPrepareOptionsMenu(Menu& menu) {
    mOptionsMenuManager.onPrepareOptionsMenu(menu);
    return true;
}

bool SettingsActivity::onOptionsItemSelected(MenuItem& item) {
    return mOptionsMenuManager.onOptionsItemSelected(item) ||
           Window::onOptionsItemSelected(item);
}

} // namespace deskclock

// Registered under the bare class name the Settings menu item's Intent carries.
static const int _cdroid_act_reg_settings =
    (::cdroid::ActivityFactory::registerActivity("SettingsActivity",
        []() -> ::cdroid::Window* {
            ::cdroid::Window* w = new ::cdroid::deskclock::SettingsActivity();
            w->setActivityName("SettingsActivity");
            return w;
        }), 0);

} // namespace cdroid
