/*********************************************************************************
 * widgetsDemo: a widget showcase framed as a smart-home control panel
 * ("CDroid Home"). Fragment-based — each of the 10 tab pages is a DemoPageFragment
 * driven by the ported androidx FragmentPagerAdapter on a classic ViewPager,
 * with TabLayout wired via setupWithViewPager. The per-page widget demos and
 * their wiring live in pages.cc.
 *********************************************************************************/
#include <cdroid.h>
#include <core/activityfactory.h>
#include <view/layoutinflater.h>
#include <widget/viewpager.h>
#include <widgetEx/tablayout/tablayout.h>
#include <fragment/fragmentactivity.h>
#include <fragment/fragmentmanager.h>
#include <R.h>
#include <typeinfo>
#include "fragments.h"

using namespace cdroid;

static void pager_post(std::function<void()>* fn);

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

            // TEMP STRESS HOOK: PRD_STRESS=1 pages through tabs every 250ms
            // (deterministic repro for the teardown UAF; input clicks vary in timing).
            if (getenv("PRD_STRESS")) {
                static ViewPager* sPager = pager;
                static int sNext = 0;
                auto step = new std::function<void()>;
                *step = [step]() {
                    sPager->setCurrentItem(sNext, false);
                    sNext = (sNext + 1) % 10;
                    pager_post(step);
                };
                pager_post(step);
            }
        }
    }
};

static std::function<void()>* gStep;
static Handler gPagerTimer;
static void pager_post(std::function<void()>* fn) {
    gStep = fn;
    gPagerTimer.postDelayed([](){ if (gStep) (*gStep)(); }, 250);
}

REGISTER_ACTIVITY(WidgetsDemoActivity);

int main(int argc, const char* argv[]) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    App app(argc, argv);
    // The launcher activity starts itself from the manifest (App plays the
    // system side when no window is up).
    return app.exec();
}
