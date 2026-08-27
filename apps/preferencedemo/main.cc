/*********************************************************************************
 * Android-Settings-style demo for the androidx.preference port.
 *
 * One PreferenceFragment (SettingsFragment) serves every screen: the root
 * hierarchy comes from assets/xml/settings_root.xml, each entry navigates to
 * a nested screen via OnPreferenceStartScreenCallback (FragmentTransaction
 * replace + back stack, window title follows the screen). Covers switch/
 * checkbox groups, dependency disabling, dialog preferences (EditText/List/
 * MultiSelect), the expand button, and a confirm dialog on "Factory reset".
 *
 * Run with DISPLAY pointing at an X server (export DISPLAY="localhost:13.0").
 *********************************************************************************/
#include <cdroid.h>
#include <R.h>
#include <preference/preferencefragment.h>
#include <preference/preferencescreen.h>
#include <preference/preferencecategory.h>
#include <preference/preference.h>
#include <preference/checkboxpreference.h>
#include <preference/switchpreference.h>
#include <preference/edittextpreference.h>
#include <preference/listpreference.h>
#include <preference/multiselectlistpreference.h>
#include <fragment/fragmentactivity.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>
#include <app/alertdialog.h>
#include <app/dialoginterface.h>
#include <widget/toast.h>
#include <widget/linearlayout.h>
#include <widget/textview.h>
#include <porting/cdlog.h>
#include <widget/internal_R.h>
#include <content/typedvalue.h>

using cdroid::PreferenceFragment;
using cdroid::PreferenceScreen;
using cdroid::fragment::FragmentActivity;

namespace {

/** Resolve a theme color (fallback = the hardcoded Material Light value).
 *  Attrs like textColorPrimary resolve to a ColorStateList reference — the
 *  resourceId must be loaded (->getDefaultColor), not used as a raw ARGB. */
int themeColor(cdroid::Context& ctx, int attr, int fallback) {
    cdroid::TypedValue v;
    if (!ctx.getTheme().resolveAttribute(attr, &v, true)) {
        return fallback;
    }
    if (v.resourceId != 0) {
        auto csl = ctx.getColorStateList((int)v.resourceId);
        if (csl != nullptr) return csl->getDefaultColor();
    }
    return v.data;
}

/** Nested-screen key -> preference XML resource (both under assets/xml/). */
int screenXmlFor(const std::string& key) {
    if (key == "screen_network")          return (int)preferencedemo::R::xml::settings_network;
    if (key == "screen_connected")        return (int)preferencedemo::R::xml::settings_connected;
    if (key == "screen_apps")             return (int)preferencedemo::R::xml::settings_apps;
    if (key == "screen_notifications")    return (int)preferencedemo::R::xml::settings_notifications;
    if (key == "screen_battery")          return (int)preferencedemo::R::xml::settings_battery;
    if (key == "screen_storage")          return (int)preferencedemo::R::xml::settings_storage;
    if (key == "screen_sound")            return (int)preferencedemo::R::xml::settings_sound;
    if (key == "screen_display")          return (int)preferencedemo::R::xml::settings_display;
    if (key == "screen_accessibility")    return (int)preferencedemo::R::xml::settings_accessibility;
    if (key == "screen_security")         return (int)preferencedemo::R::xml::settings_security;
    if (key == "screen_location")         return (int)preferencedemo::R::xml::settings_location;
    if (key == "screen_privacy")          return (int)preferencedemo::R::xml::settings_privacy;
    if (key == "screen_system")           return (int)preferencedemo::R::xml::settings_system;
    if (key == "screen_about")            return (int)preferencedemo::R::xml::settings_about;
    return 0;
}

} // namespace

class SettingsActivity;

class SettingsFragment : public PreferenceFragment {
public:
    void onCreatePreferences(cdroid::Bundle* /*savedInstanceState*/,
            const std::string& rootKey) override {
        mRootKey = rootKey;
        // Nested screens load their own XML by key; the root loads everything.
        const int xml = rootKey.empty() ? (int)preferencedemo::R::xml::settings_root
                                        : screenXmlFor(rootKey);
        setPreferencesFromResource(xml, std::string());

        applySimpleSummaryProviders(getPreferenceScreen());
        watchTwoStateToggles(getPreferenceScreen());

        // Accessibility screen: collapse "Interaction controls" behind the
        // expand button (initialExpandedChildrenCount is a library-private
        // attr, so it is set from code rather than XML).
        if (getPreferenceScreen() != nullptr) {
            LOGD("[settings] loaded root=%s items=%d", rootKey.empty() ? "<root>" : rootKey.c_str(),
                 getPreferenceScreen()->getPreferenceCount());
        }
        if (rootKey == "screen_accessibility") {
            auto* cat = dynamic_cast<cdroid::PreferenceGroup*>(
                    findPreference("category_accessibility_interaction"));
            if (cat != nullptr) {
                cat->setInitialExpandedChildrenCount(3);
            }
        }
    }

    /** PreferenceFragment layout wrapped with an in-screen header bar
     *  (the AOSP Settings screen title — CDROID windows have no title API). */
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
            cdroid::Bundle* savedInstanceState) override {
        cdroid::View* content = PreferenceFragment::onCreateView(inflater, container,
                savedInstanceState);
        auto* root = new cdroid::LinearLayout(requireContext());
        root->setOrientation(cdroid::LinearLayout::VERTICAL);
        // The window surface is transparent — this root must carry a fully
        // opaque background or the preference list floats over the desktop.
        // Both colors follow the live theme (?android:attr/colorBackground /
        // textColorPrimary), so switching Material <-> Material.Light restyles
        // the chrome with no per-theme constants.
        const int bg = themeColor(*requireContext(),
                (int)internal::R::attr::colorBackground, 0xFFF8F9FA);
        root->setBackgroundColor(bg);
        auto* bar = new cdroid::LinearLayout(requireContext());
        bar->setOrientation(cdroid::LinearLayout::HORIZONTAL);
        bar->setGravity(cdroid::Gravity::CENTER_VERTICAL);
        bar->setBackgroundColor(bg);
        bar->setPadding(16, 12, 24, 12);
        const int fg = themeColor(*requireContext(),
                (int)internal::R::attr::textColorPrimary, 0xFF1B1B1F);
        const bool nested = !mRootKey.empty();
        if (nested) {
            // Up affordance (the AOSP Settings app bar arrow): pops this
            // screen back to its parent, independent of keyboard BACK.
            auto* up = new cdroid::TextView(requireContext());
            up->setText(u8"\u2190");
            up->setTextSize(24);
            up->setTextColor(fg);
            up->setPadding(12, 8, 20, 8);
            up->setClickable(true);
            up->setOnClickListener([this](cdroid::View&) { requestGoBack(); });
            bar->addView(up);
        }
        auto* header = new cdroid::TextView(requireContext());
        const std::string title = getPreferenceScreen() && !getPreferenceScreen()->getTitle().empty()
                ? getPreferenceScreen()->getTitle() : std::string("Settings");
        header->setText(title);
        header->setTextSize(22);
        header->setTextColor(fg);
        bar->addView(header);
        root->addView(bar);
        if (content != nullptr) {
            root->addView(content, new cdroid::LinearLayout::LayoutParams(
                    cdroid::LayoutParams::MATCH_PARENT, 0, 1.0f));
        }
        return root;
    }

    bool onPreferenceTreeClick(Preference& preference) override;
    // Top-level entries (plain Preferences, the androidx app:fragment
    // equivalent) navigate via the host; defined after SettingsActivity
    // (the host type must be complete for the cast). The second-level
    // screen is pushed onto the back stack so Back returns here.

private:
    std::string mRootKey;

    /** Pop via the host (defined after SettingsActivity — complete type). */
    void requestGoBack();

    bool handleTreeClick(Preference& preference) {
        if (preference.getKey() == "factory_reset") {
            confirmFactoryReset();
            return true;
        }
        if (preference.getKey() == "check_for_updates") {
            cdroid::Toast::makeText(requireContext(), "Already up to date",
                    cdroid::Toast::LENGTH_SHORT)->show();
            return true;
        }
        LOGD("[settings] tree click: %s", preference.getKey().c_str());
        return PreferenceFragment::onPreferenceTreeClick(preference);
    }

public:
    /** android:useSimpleSummaryProvider is a library-private attr — apply the
     *  AOSP SimpleSummaryProviders from code for every List/EditText screen. */
    void applySimpleSummaryProviders(cdroid::PreferenceGroup* group) {
        if (group == nullptr) return;
        for (int i = 0; i < group->getPreferenceCount(); i++) {
            Preference* p = group->getPreference(i);
            if (auto* lp = dynamic_cast<cdroid::ListPreference*>(p)) {
                lp->setSummaryProvider(cdroid::ListPreference::SimpleSummaryProvider::getInstance());
            } else if (auto* ep = dynamic_cast<cdroid::EditTextPreference*>(p)) {
                ep->setSummaryProvider(cdroid::EditTextPreference::SimpleSummaryProvider::getInstance());
            } else if (auto* nested = dynamic_cast<cdroid::PreferenceGroup*>(p)) {
                applySimpleSummaryProviders(nested);
            }
        }
    }

    /** Toast feedback on every two-state toggle (like the AOSP Settings QSTile
     *  hint), so value changes are visible without a summary re-read. */
    void watchTwoStateToggles(cdroid::PreferenceGroup* group) {
        if (group == nullptr) return;
        for (int i = 0; i < group->getPreferenceCount(); i++) {
            Preference* p = group->getPreference(i);
            if (auto* nested = dynamic_cast<cdroid::PreferenceGroup*>(p)) {
                watchTwoStateToggles(nested);
                continue;
            }
            auto* tsp = dynamic_cast<cdroid::TwoStatePreference*>(p);
            if (tsp == nullptr) continue;
            const std::string title = tsp->getTitle();
            tsp->setOnPreferenceChangeListener(
                [this, title](Preference&, const nonstd::any& newValue) {
                    const bool on = nonstd::any_cast<bool>(newValue);
                    cdroid::Toast::makeText(requireContext(),
                            title + (on ? ": on" : ": off"),
                            cdroid::Toast::LENGTH_SHORT)->show();
                    return true;
                });
        }
    }

    void confirmFactoryReset() {
        auto* b = new cdroid::AlertDialog::Builder(requireContext());
        b->setTitle("Erase all data?")
         .setMessage("This will reset all settings on this device. This action cannot be undone.")
         .setPositiveButton("Erase all data",
            [this](cdroid::Dialog&, int) {
                cdroid::Toast::makeText(requireContext(), "Reset (demo)",
                        cdroid::Toast::LENGTH_SHORT)->show();
            })
         .setNegativeButton("Cancel", nullptr);
        b->create()->show();
    }
};

/** Hosts the settings hierarchy; nested screens push a new SettingsFragment. */
class SettingsActivity : public FragmentActivity, public cdroid::OnPreferenceStartScreenCallback {
public:
    SettingsActivity() : FragmentActivity(0, 0, -1, -1) {
        // Framework theme carries the whole palette (text appearances,
        // ripples, window background); the opaque backdrops below only
        // guarantee no desktop leak from the transparent window surface.
        setTheme((int)internal::R::style::Theme_Material_Light);
    }

    bool onPreferenceStartScreen(PreferenceFragment& /*caller*/,
            PreferenceScreen& pref) override {
        openScreen(pref.getKey(), pref.getTitle());
        return true;
    }

    /** Push the second-level screen; Back pops it (onBackPressed). */
    void openScreen(const std::string& key, const std::string& title) {
        auto* fragment = new SettingsFragment();
        auto* args = new cdroid::Bundle();
        args->putString(PreferenceFragment::ARG_PREFERENCE_ROOT, key);
        fragment->setArguments(args);
        auto* tx = getSupportFragmentManager()->beginTransaction();
        // PREFDEMO_NO_ANIM=1 skips the custom slides so the push rides the
        // DEFAULT Fade (the transition path whose per-op clone bug was fixed).
        if (getenv("PREFDEMO_NO_ANIM") == nullptr) {
            tx->setCustomAnimations((int)preferencedemo::R::anim::slide_in_right,
                                    (int)preferencedemo::R::anim::slide_out_left,
                                    (int)preferencedemo::R::anim::slide_in_left,
                                    (int)preferencedemo::R::anim::slide_out_right);
        }
        tx->replace(getFragmentContainerId(), fragment)
           .addToBackStack(key)
           .commit();
    }

protected:
    void onCreate(cdroid::Bundle* savedInstanceState) override {
        FragmentActivity::onCreate(savedInstanceState);
        // The window surface is transparent: back the fragment container with
        // an opaque color too, so nested-screen swaps never leak the desktop.
        cdroid::View* container = findViewById(getFragmentContainerId());
        if (container != nullptr) {
            container->setBackgroundColor(themeColor(*getContext(),
                    (int)internal::R::attr::colorBackground, 0xFFF8F9FA));
        }
        // Optional argv[1]: start directly at a nested screen (smoke-testing
        // every second-level page without touch input), e.g.
        //   ./preferencedemo screen_network
        std::string initialRoot;
        if (mLaunchArg != nullptr) initialRoot = mLaunchArg;
        auto* fragment = new SettingsFragment();
        if (!initialRoot.empty()) {
            auto* args = new cdroid::Bundle();
            args->putString(PreferenceFragment::ARG_PREFERENCE_ROOT, initialRoot);
            fragment->setArguments(args);
        }
        getSupportFragmentManager()->beginTransaction()
            ->replace(getFragmentContainerId(), fragment)
            .commit();
    }

private:
    const char* mLaunchArg = nullptr;

public:
    void setLaunchRoot(const char* key) { mLaunchArg = key; }

    /** Pop the current second-level screen back to its parent. */
    void goBack() {
        getSupportFragmentManager()->popBackStackImmediate();
    }

protected:
    void onBackPressed() override {
        // The back stack pops the nested-screen fragment; its own header bar
        // shows the parent title again.
        if (!getSupportFragmentManager()->popBackStackImmediate()) {
            FragmentActivity::onBackPressed();
        }
    }
};

void SettingsFragment::requestGoBack() {
    auto* host = dynamic_cast<SettingsActivity*>(getActivity());
    if (host != nullptr) host->goBack();
}

bool SettingsFragment::onPreferenceTreeClick(Preference& preference) {
    if (screenXmlFor(preference.getKey()) != 0) {
        auto* host = dynamic_cast<SettingsActivity*>(getActivity());
        if (host != nullptr) {
            host->openScreen(preference.getKey(), preference.getTitle());
            return true;
        }
    }
    return handleTreeClick(preference);
}

int main(int argc, const char* argv[]) {
    cdroid::App app(argc, argv);
    // App-level theme: every LayoutInflater::from(ctx) in the preference
    // chain resolves ?android:attr/textAppearance against the App context,
    // so the Material Light palette reaches the row TextViews (a Window-only
    // setTheme would not propagate to those contexts).
    app.setTheme((int)internal::R::style::Theme_Material);
    auto* w = new SettingsActivity();
    if (argc > 1) w->setLaunchRoot(argv[1]);
    LOGD("settings demo window created");
    return app.exec();
}
