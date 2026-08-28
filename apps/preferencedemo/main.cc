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
#include <transition/slide.h>
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
        // Pushed screens drive their motion with fragment Transitions (SEC
        // Priority 1): one scene change on the container, so the enter/exit
        // pair stays in lock-step. The legacy custom-Animation path remains
        // selectable (PREFDEMO_ANIM=1 in openScreen) but runs the two ops as
        // independent Animations whose handoff shows a seam. Slide edges mirror
        // the old R.anim slides: enter from right / exit to left; pop returns
        // to right, reenter comes back from left. The root keeps no transition,
        // so the initial show is static and covering it fades (SEC default).
        if (!rootKey.empty()) {
            setEnterTransition(new cdroid::Slide(cdroid::Gravity::RIGHT));
            setExitTransition(new cdroid::Slide(cdroid::Gravity::LEFT));
            setReenterTransition(new cdroid::Slide(cdroid::Gravity::LEFT));
            setReturnTransition(new cdroid::Slide(cdroid::Gravity::RIGHT));
        }
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
        // Valgrind/CI driver: PREFDEMO_AUTOCYCLE clicks through every root
        // preference (sub-screens get dwell + Back via the virtual
        // Window::onBackPressed), then exits cleanly so the leak report lands.
        // Only the ROOT screen drives — pushed sub-screen fragments would
        // otherwise start a nested cycle of their own.
        if (rootKey.empty() && getenv("PREFDEMO_AUTOCYCLE") != nullptr) {
            // File-local handler (mHandler is private); generous delays —
            // this runs under valgrind.
            static cdroid::Handler sCycleHandler(cdroid::Looper::getMainLooper());
            sCycleHandler.postDelayed([this]() { cycleStep(0); }, 4000);
        }
    }

    /** AUTOCYCLE walker (see onCreatePreferences). Each step is a fresh
     *  lambda capturing values only — no self-referencing runnable. */
    void cycleStep(int index);

    /** PreferenceFragment layout wrapped with the settings chrome (the AOSP
     *  Settings screen title — CDROID windows have no title API). The chrome
     *  inflates from R::layout::prefdemo_settings: layout/ is header-above-
     *  list, the layout-land/ twin is a title column beside the list — same
     *  ids and view types, so this one wiring path serves both. */
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
            cdroid::Bundle* savedInstanceState) override {
        cdroid::View* content = PreferenceFragment::onCreateView(inflater, container,
                savedInstanceState);
        auto* root = dynamic_cast<cdroid::ViewGroup*>(inflater->inflate(
                (int)preferencedemo::R::layout::prefdemo_settings, container, false));
        // The window surface is transparent — the chrome must carry fully
        // opaque backgrounds or the preference list floats over the desktop.
        // Both colors follow the live theme (?android:attr/colorBackground /
        // textColorPrimary), so switching Material <-> Material.Light restyles
        // the chrome with no per-theme constants.
        const int bg = themeColor(*requireContext(),
                (int)internal::R::attr::colorBackground, 0xFFF8F9FA);
        const int fg = themeColor(*requireContext(),
                (int)internal::R::attr::textColorPrimary, 0xFF1B1B1F);
        if (root != nullptr) root->setBackgroundColor(bg);
        if (auto* header = root ? root->findViewById((int)preferencedemo::R::id::prefdemo_header)
                                : nullptr)
            header->setBackgroundColor(bg);
        const std::string title = getPreferenceScreen() && !getPreferenceScreen()->getTitle().empty()
                ? getPreferenceScreen()->getTitle() : std::string("Settings");
        auto* titleView = dynamic_cast<cdroid::TextView*>(
                root ? root->findViewById((int)preferencedemo::R::id::prefdemo_title) : nullptr);
        if (titleView != nullptr) {
            titleView->setText(title);
            titleView->setTextColor(fg);
        }
        // Up affordance (the AOSP Settings app bar arrow): present in the XML,
        // visible only on portrait nested screens (push navigation). Landscape
        // runs two-pane — selecting on the left replaces the right pane, there
        // is no parent to pop; independent of keyboard BACK either way.
        auto* up = dynamic_cast<cdroid::TextView*>(
                root ? root->findViewById((int)preferencedemo::R::id::prefdemo_back) : nullptr);
        const bool portrait = requireContext()->getResources().getConfiguration().orientation
                != cdroid::Configuration::ORIENTATION_LANDSCAPE;
        if (up != nullptr) {
            up->setText(u8"←");
            up->setTextColor(fg);
            up->setClickable(true);
            up->setVisibility((!mRootKey.empty() && portrait)
                    ? cdroid::View::VISIBLE : cdroid::View::GONE);
            up->setOnClickListener([this](cdroid::View&) { requestGoBack(); });
        }
        auto* host = dynamic_cast<cdroid::ViewGroup*>(
                root ? root->findViewById((int)preferencedemo::R::id::prefdemo_content) : nullptr);
        if (host != nullptr) {
            host->setBackgroundColor(bg);
            if (content != nullptr) {
                host->addView(content, new cdroid::LinearLayout::LayoutParams(
                        cdroid::LayoutParams::MATCH_PARENT,
                        cdroid::LayoutParams::MATCH_PARENT));
            }
        } else {
            LOGW("prefdemo_settings layout has no prefdemo_content host");
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
                lp->setSummaryProvider(cdroid::ListPreference::SimpleSummaryProvider());
            } else if (auto* ep = dynamic_cast<cdroid::EditTextPreference*>(p)) {
                ep->setSummaryProvider(cdroid::EditTextPreference::SimpleSummaryProvider());
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

void SettingsFragment::cycleStep(int index) {
    // File-local main-looper handler: PreferenceFragment::mHandler is private.
    static cdroid::Handler sCycleHandler(cdroid::Looper::getMainLooper());
    cdroid::PreferenceScreen* screen = getPreferenceScreen();
    if (screen == nullptr || index >= screen->getPreferenceCount()) {
        cdroid::App::getInstance().exit(0);
        return;
    }
    cdroid::Preference* p = screen->getPreference(index);
    const bool isScreen = dynamic_cast<cdroid::PreferenceScreen*>(p) != nullptr;
    p->performClick();
    if (isScreen) {
        // Let the pushed sub-screen settle, pop it via the host Window's
        // virtual onBackPressed, then walk on.
        sCycleHandler.postDelayed([this, index]() {
            getActivity()->onBackPressed();
            sCycleHandler.postDelayed([this, index]() { cycleStep(index + 1); }, 3000);
        }, 6000);
    } else {
        sCycleHandler.postDelayed([this, index]() { cycleStep(index + 1); }, 2500);
    }
}

/** Hosts the settings hierarchy; nested screens push a new SettingsFragment. */
class SettingsActivity : public FragmentActivity, public cdroid::OnPreferenceStartScreenCallback {
public:
    SettingsActivity() : FragmentActivity(0, 0, -1, -1) {
        // Framework theme carries the whole palette (text appearances,
        // ripples, window background); the opaque backdrops below only
        // guarantee no desktop leak from the transparent window surface.
        setTheme((getenv("PREFDEMO_DARK") != nullptr)
                ? (int)internal::R::style::Theme_Material
                : (int)internal::R::style::Theme_Material_Light);
    }

    bool onPreferenceStartScreen(PreferenceFragment& /*caller*/,
            PreferenceScreen& pref) override {
        navigateTo(pref.getKey(), pref.getTitle());
        return true;
    }

    /** Route a second-level screen by orientation: portrait pushes it onto
     *  the back stack (Back pops), landscape swaps the detail pane in place
     *  (two-pane has no back navigation). */
    void navigateTo(const std::string& key, const std::string& title) {
        if (mLandscape) showDetail(key);
        else openScreen(key, title);
    }

    /** Portrait: push the second-level screen; Back pops it (onBackPressed). */
    void openScreen(const std::string& key, const std::string& /*title*/) {
        auto* fragment = newFragmentForKey(key);
        auto* tx = getSupportFragmentManager()->beginTransaction();
        // Default motion is the fragment Transitions set in onCreatePreferences
        // (SEC Priority 1). PREFDEMO_ANIM=1 opts this transaction back into the
        // legacy custom slides (SEC Priority 2, kept for comparison/regression).
        if (getenv("PREFDEMO_ANIM") != nullptr) {
            tx->setCustomAnimations((int)preferencedemo::R::anim::slide_in_right,
                                    (int)preferencedemo::R::anim::slide_out_left,
                                    (int)preferencedemo::R::anim::slide_in_left,
                                    (int)preferencedemo::R::anim::slide_out_right);
        }
        tx->replace((int)preferencedemo::R::id::prefdemo_single, fragment)
           .addToBackStack(key)
           .commit();
    }

    /** Landscape: swap the right pane to the selected second-level screen. */
    void showDetail(const std::string& key) {
        getSupportFragmentManager()->beginTransaction()
            ->replace((int)preferencedemo::R::id::prefdemo_detail, newFragmentForKey(key))
            .commit();
    }

    static SettingsFragment* newFragmentForKey(const std::string& key) {
        auto* fragment = new SettingsFragment();
        auto* args = new cdroid::Bundle();
        args->putString(PreferenceFragment::ARG_PREFERENCE_ROOT, key);
        fragment->setArguments(args);
        return fragment;
    }

protected:
    void onCreate(cdroid::Bundle* savedInstanceState) override {
        FragmentActivity::onCreate(savedInstanceState);
        mLandscape = getContext()->getResources().getConfiguration().orientation
                == cdroid::Configuration::ORIENTATION_LANDSCAPE;
        // Activity content comes from R::layout::prefdemo_main: layout/ is a
        // single slot, the layout-land/ twin is master + detail (two-pane).
        // The window surface is transparent: back the slots with an opaque
        // color too, so pane swaps never leak the desktop.
        auto* host = dynamic_cast<cdroid::ViewGroup*>(findViewById(getFragmentContainerId()));
        if (host == nullptr) {
            LOGW("no fragment container to host prefdemo_main");
            return;
        }
        auto* main = cdroid::LayoutInflater::from(getContext())->inflate(
                (int)preferencedemo::R::layout::prefdemo_main, host, true);
        main->setBackgroundColor(themeColor(*getContext(),
                (int)internal::R::attr::colorBackground, 0xFFF8F9FA));
        // Optional argv[1]: start directly at a nested screen (smoke-testing
        // every second-level page without touch input), e.g.
        //   ./preferencedemo screen_network
        std::string initialRoot;
        if (mLaunchArg != nullptr) initialRoot = mLaunchArg;
        if (mLandscape) {
            // Two-pane: master carries the top-level list, detail starts at
            // the given (or first) top-level entry's screen.
            auto* tx = getSupportFragmentManager()->beginTransaction();
            tx->replace((int)preferencedemo::R::id::prefdemo_master, new SettingsFragment());
            tx->replace((int)preferencedemo::R::id::prefdemo_detail,
                    newFragmentForKey(initialRoot.empty() ? "screen_network" : initialRoot));
            tx->commit();
        } else {
            auto* fragment = new SettingsFragment();
            if (!initialRoot.empty()) fragment = newFragmentForKey(initialRoot);
            getSupportFragmentManager()->beginTransaction()
                ->replace((int)preferencedemo::R::id::prefdemo_single, fragment)
                .commit();
        }
    }

private:
    const char* mLaunchArg = nullptr;
    bool mLandscape = false;   // set in onCreate from the resource config

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
            host->navigateTo(preference.getKey(), preference.getTitle());
            return true;
        }
    }
    return handleTreeClick(preference);
}

int main(int argc, const char* argv[]) {
    cdroid::App app(argc, argv);

    // Orientation -> resource config: PREFDEMO_ORIENTATION forces a variant
    // here (like PREFDEMO_DARK below); with no env the framework already
    // resolved it in App::applyOrientationConfig (CDROID_ORIENTATION env >
    // the manifest's android:screenOrientation > screen shape), so there is
    // nothing to redo. Either way this stays before the first inflate
    // (SettingsActivity below) so layouts resolve under the right config.
    int orientation = 0;
    const char* forced = getenv("PREFDEMO_ORIENTATION");
    if (forced != nullptr && forced[0] != '\0') {
        const std::string v = forced;
        if (v == "land" || v == "landscape")
            orientation = cdroid::Configuration::ORIENTATION_LANDSCAPE;
        else if (v == "port" || v == "portrait")
            orientation = cdroid::Configuration::ORIENTATION_PORTRAIT;
        else
            LOGW("PREFDEMO_ORIENTATION='%s' invalid (land|landscape|port|portrait)",
                 forced);
    }
    if (orientation != 0) {
        cdroid::Resources& res = app.getResources();
        cdroid::Configuration cfg = res.getConfiguration();
        cfg.orientation = orientation;
        res.updateConfiguration(&cfg, nullptr);
        LOGI("prefdemo orientation=%s (env override)",
             orientation == cdroid::Configuration::ORIENTATION_LANDSCAPE
                     ? "landscape" : "portrait");
    }
    // App-level theme: every LayoutInflater::from(ctx) in the preference
    // chain resolves ?android:attr/textAppearance against the App context,
    // so the Material Light palette reaches the row TextViews (a Window-only
    // setTheme would not propagate to those contexts).
    // PREFDEMO_DARK=1 selects the dark Material theme (chrome colors all
    // resolve from the live theme, so both palettes exercise the same code).
    const int themeId = (getenv("PREFDEMO_DARK") != nullptr)
            ? (int)internal::R::style::Theme_Material
            : (int)internal::R::style::Theme_Material_Light;
    app.setTheme(themeId);
    auto* w = new SettingsActivity();
    if (argc > 1) w->setLaunchRoot(argv[1]);
    LOGD("settings demo window created");
    return app.exec();
}
