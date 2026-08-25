/*********************************************************************************
 * widgetsDemo: a widget showcase framed as a smart-home control panel
 * ("CDroid Home"). Fragment-based — each of the 10 tab pages is a DemoPageFragment
 * driven by the ported androidx FragmentPagerAdapter on a classic ViewPager,
 * with TabLayout wired via setupWithViewPager. The per-page widget demos and
 * their wiring live in pages.cc.
 *********************************************************************************/
#include <cdroid.h>
#include <core/activityfactory.h>
#include <app/alertdialog.h>
#include <view/layoutinflater.h>
#include <widget/viewpager.h>
#include <widgetEx/tablayout/tablayout.h>
#include <fragment/fragmentactivity.h>
#include <fragment/fragmentmanager.h>
#include <R.h>
#include <typeinfo>
#include "fragments.h"

using namespace cdroid;

class WidgetsDemoActivity : public fragment::FragmentActivity {
    TabLayout* mTabs = nullptr;
public:
    WidgetsDemoActivity() : FragmentActivity(0, 0, -1, -1) {
        ViewGroup* root = (ViewGroup*)LayoutInflater::from(getContext())
            ->inflate(widgetsDemo::R::layout::main, this, false);
        addView(root);
        mTabs = (TabLayout*)root->findViewById(widgetsDemo::R::id::tabs);
        if (ViewGroup* host = (ViewGroup*)root->findViewById(widgetsDemo::R::id::pager_container)) {
            // The legacy androidx ViewPager is not registered for XML inflation,
            // so the pager is created here and hosted in the layout's container.
            ViewPager* pager = new ViewPager(getContext());
            pager->setId(View::generateViewId());  // unique tag base for pager fragments
            pager->setAdapter(new DemoFragmentPagerAdapter(getSupportFragmentManager()));
            // 10 small static pages: keep them all attached (androidx guidance for
            // FragmentPagerAdapter) instead of tearing down/rebuilding views through
            // the SpecialEffects exit pipeline on every tab hop.
            pager->setOffscreenPageLimit(9);
            // Start on the progress page (page 1) instead of the buttons page.
            pager->setCurrentItem(1, false);
            host->addView(pager, new ViewGroup::LayoutParams(
                    ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::MATCH_PARENT));
            if (mTabs) mTabs->setupWithViewPager(pager);

            // TEMP STRESS HOOK: PRD_DIALOG=1 opens and dismisses the misc-page
            // AlertDialog every 400ms (deterministic repro for the decor leak;
            // input clicks vary in timing). Same builder chain as pages.cc btn_dialog.
            if (getenv("PRD_DIALOG")) {
                static AlertDialog* sDialog = nullptr;
                auto step = new std::function<void()>;
                ViewPager* p = pager;
                *step = [step, p]() {
                    if (sDialog && sDialog->isShowing()) {
                        sDialog->dismiss();  // same as tapping OK
                    } else {
                        auto noop = [](DialogInterface&, int) {};
                        sDialog = AlertDialog::Builder(&App::getInstance())
                            .setTitle("cdroid")
                            .setMessage("stress dialog")
                            .setPositiveButton("OK", noop)
                            .setNegativeButton("Cancel", noop)
                            .show();
                    }
                    p->postDelayed([step](){ (*step)(); }, 400);
                };
                pager->postDelayed([step](){ (*step)(); }, 400);
            }

            // TEMP STRESS HOOK: PRD_STRESS=1 pages through tabs every 250ms
            // (deterministic repro for the teardown UAF; input clicks vary in timing).
            if (getenv("PRD_STRESS")) {
                auto step = new std::function<void()>;
                ViewPager* p = pager;
                *step = [step, p]() {
                    static int sNext = 0;
                    p->setCurrentItem(sNext, false);
                    sNext = (sNext + 1) % 10;
                    p->postDelayed([step](){ (*step)(); }, 250);
                };
                pager->postDelayed([step](){ (*step)(); }, 250);
            }
        }
    }
};

REGISTER_ACTIVITY(WidgetsDemoActivity);

int main(int argc, const char* argv[]) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    App app(argc, argv);
    // The launcher activity starts itself from the manifest (App plays the
    // system side when no window is up).
    return app.exec();
}
