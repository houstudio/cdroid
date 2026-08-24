/*********************************************************************************
 * printerdemo: a printer's built-in control-panel UI (MFP HMI), built with
 * Fragment + Navigation + android.transition (Slide). Pure facade — no driver.
 *
 * One page per file (REGISTER_FRAGMENT self-registers each with the
 * FragmentFactory; the nav graph references the class names):
 *   home_fragment.cc      control-panel dashboard: status, ink (K/C/M/Y), paper, function grid
 *   copy_fragment.cc      copier settings: copies stepper, color/paper/quality radios, zoom seekbar
 *   scan_fragment.cc      scan bed with a MotionLayout-driven beam sweep
 *   maintain_fragment.cc  printhead maintenance actions + supplies status
 *   settings_fragment.cc  network / preferences / system rows
 *   about_fragment.cc     rich-text intro (Html.fromHtml)
 *
 * Cross-page plumbing (shared ViewModel access, locale switch) lives in
 * printer_common.{h,cc}. This file: the host activity (toolbar +
 * BottomNavigationView + NavHostFragment) and main(). Fragment enter/exit use
 * android.transition Slide (set per-Fragment in onCreate), NOT legacy
 * enterAnim/exitAnim — see navdemo_transition. IDs (R.h / ID.xml) are
 * auto-generated.
 *********************************************************************************/
#include <cdroid.h>
#include <cdlog.h>
#include <core/activityfactory.h>
#include <drawable/colordrawable.h>
#include <fragment/fragmentactivity.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <menu/menuinflater.h>
#include <navigation/navcontroller.h>
#include <navigation/navdestination.h>
#include <navigation/navhostfragment.h>
#include <navigation/navigationui.h>
#include <widget/toolbar.h>
#include <widgetEx/navigationview/bottomnavigationview.h>
#include "printer_common.h"
#include "R.h"

// Persisted theme choice — re-read by every new window (AOSP recreate +
// onCreate re-reads the persisted selection). Only this file reads/writes it,
// so it stays file-local.
static bool sDarkTheme = false;

// ---------------------------------------------------------------------------
class PrinterDemoWindow : public cdroid::fragment::FragmentActivity{
    cdroid::NavHostFragment* mNavHost = nullptr;
    cdroid::Toolbar* mToolbar = nullptr;
    cdroid::BottomNavigationView* mBottomNavigation = nullptr;
    bool mInited = false;
public:
    PrinterDemoWindow() : FragmentActivity(0, 0, -1, -1){
        cdroid::ViewGroup* root = (cdroid::ViewGroup*)cdroid::LayoutInflater::from(getContext())
            ->inflate(printerdemo::R::layout::main, this, false);
        addView(root);
        // The window surface is transparent by default — any region not covered by an opaque
        // child (e.g. a strip left after a full-screen overlay is hidden, before it repaints)
        // shows black. Give the Window itself an opaque background so uncovered areas are bg_screen.
        setBackground(getContext()->getDrawable(printerdemo::R::drawable::bg_screen));
        mToolbar = (cdroid::Toolbar*)root->findViewById(printerdemo::R::id::toolbar);
        mBottomNavigation = (cdroid::BottomNavigationView*)root->findViewById(
            printerdemo::R::id::bottom_navigation);
    }
    void onCreate(cdroid::Bundle* savedInstanceState) override{
        FragmentActivity::onCreate(savedInstanceState);
        mNavHost = new cdroid::NavHostFragment(printerdemo::R::navigation::nav_graph);
        getSupportFragmentManager()->beginTransaction()
            ->replace(printerdemo::R::id::nav_host_container, mNavHost)
            .commit();
    }
    void onActive() override{
        cdroid::fragment::FragmentActivity::onActive();
        if(mInited) return;
        mInited = true;
        if(!mToolbar || !mNavHost) return;
        cdroid::NavController* nc = mNavHost->getNavController();
        if(!nc) return;
        // NavigationUI owns the destination title, Up behavior, and navigation click. The sample
        // keeps only its app-local icons as a visual fallback because the framework indicator may
        // be absent in the embedded resource pack.
        cdroid::NavigationUI::setupWithNavController(mToolbar, nc);
        if (mBottomNavigation != nullptr) {
            getMenuInflater()->inflate(printerdemo::R::menu::bottom_navigation,
                mBottomNavigation->getMenu());
            mBottomNavigation->refreshMenuView();
            cdroid::NavigationUI::setupWithNavController(mBottomNavigation, nc);
        }

        nc->addOnDestinationChangedListener([this](cdroid::NavController&,
                                                    cdroid::NavDestination& d, cdroid::Bundle*){
            if(!mToolbar) return;
            const bool isHome = (d.getRoute() == "home");
            // Home screen: a HOME icon (branding, non-clickable). Sub-pages: the back arrow.
            mToolbar->setNavigationIcon(getContext()->getDrawable(isHome ? printerdemo::R::drawable::ic_home : printerdemo::R::drawable::ic_back));
            // The nav button (ImageButton) inherits a default button background — override it with
            // an explicitly transparent one so only the icon shows on the gradient toolbar.
            if(cdroid::View* nav = mToolbar->getNavigationView())
                nav->setBackground(new cdroid::ColorDrawable(0));
        });

        // Options menu: "关于 CDroid" overflow item -> intro dialog (the Toolbar's own menu; no
        // ActionBar, so it coexists with the hand-driven nav icon above).
        // Options menu: "关于 CDroid" overflow item -> intro dialog. Toolbar::inflateMenu is a
        // declared-but-undefined stub, so populate the Toolbar's own menu via MenuInflater.
        if(cdroid::Menu* menu = mToolbar->getMenu())
            getMenuInflater()->inflate(printerdemo::R::menu::main, menu);
        mToolbar->setOnMenuItemClickListener([this, nc](cdroid::MenuItem& item)->bool{
            if(item.getItemId() == printerdemo::R::id::action_toggle_theme){
                // AOSP dynamic theming: flip the persisted app-level choice,
                // apply it app-wide, and relaunch this activity so the new
                // instance inflates under it (already-inflated views are never
                // re-themed in place).
                sDarkTheme = !sDarkTheme;
                cdroid::App::getInstance().setTheme(sDarkTheme
                        ? printerdemo::R::style::AppTheme_Dark
                        : printerdemo::R::style::AppTheme);
                recreate();
                return true;
            }
            if(item.getItemId() == printerdemo::R::id::action_toggle_language){
                // Quick toggle between the app's two shipped locales; the
                // settings page offers the resource-driven picker.
                applyLocale(sLocaleTag.compare(0, 2, "zh") == 0 ? "en-US" : "zh-CN");
                return true;
            }
            if(item.getItemId() == printerdemo::R::id::action_about){
                if(nc) nc->navigate("about");   // "关于 CDroid" is its own Fragment destination
                return true;
            }
            return false;
        });
    }
};

REGISTER_ACTIVITY(PrinterDemoWindow);

int main(int argc, const char* argv[]){
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    cdroid::App app(argc, argv);
    // The application theme comes from AndroidManifest.xml (application
    // android:theme="@style/AppTheme"). A persisted dark choice overrides it
    // app-wide before any window is created (AOSP: the stored
    // android:isUiEnabled/night mode is applied at process start).
    if(sDarkTheme) app.setTheme(printerdemo::R::style::AppTheme_Dark);
    // Seed the live Configuration with the startup locale so the resource
    // layer selects the right variants from the start; a language switch later
    // flips this via applyLocale() -> handleConfigurationChanged (recreate).
    {
        cdroid::Configuration c = app.getResources().getConfiguration();
        c.setLocales(cdroid::LocaleList(std::vector<cdroid::Locale>{
                cdroid::Locale::forLanguageTag(sLocaleTag)}));
        app.handleConfigurationChanged(c);   // no windows yet — resources only
    }
    // The launcher activity starts itself: exec() launches the manifest's
    // MAIN/LAUNCHER window when no window is up (App plays the system side).
    return app.exec();
}
