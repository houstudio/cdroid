/*********************************************************************************
 * Deterministic repro for the widgetsDemo "window recreate" leak pair. The
 * auto-test sweep only hits them when a recreate lands while a page-switch
 * exit transition is still settling (sweep-path drift), so this sample drives
 * that interleaving on purpose, in a loop, over the real fragment machinery:
 *
 *  A) FragmentStateManager::stepDown no-SEC fall-through owning mView
 *     (pre-fix: whole page inflation definite-lost per round).
 *  B) scheduleViewReclaim hop posted on the container, dropped when the
 *     container detaches before the hop runs (pre-fix: off-tree fragment
 *     view + subtree definite-lost).
 *  C) initialAwakenScrollBars' delayed fade Message stranded by teardown
 *     (the 176B+96B wd-fc0 residual) — pages carry a ScrollView so every
 *     (re)attach posts one.
 *
 * Each round: fresh FragmentActivity + ViewPager + FragmentPagerAdapter with
 * offscreenPageLimit(1) so page flips destroy views through the
 * SpecialEffects pipeline, then close() at a delay after the flip:
 * even rounds close mid-settle (B), odd rounds close after settling (A).
 * Expect 0 definite lost after the fixes; run under valgrind with -f 3000.
 *********************************************************************************/
#include <cdroid.h>
#include <cdlog.h>
#include <core/handler.h>
#include <fragment/fragmentactivity.h>
#include <fragment/fragment.h>
#include <fragment/fragmentpageradapter.h>
#include <widget/viewpager.h>
#include <widget/scrollview.h>
#include <widget/linearlayout.h>
#include <widget/textview.h>

using cdroid::Fragment;
using cdroid::FragmentActivity;
using cdroid::FragmentManager;
using cdroid::View;
using cdroid::ViewGroup;

/* Scrollable page: enough TextViews to overflow the window, so attach runs
 * initialAwakenScrollBars (path C) and flips destroy/rebuild through the SEC
 * (paths A/B). */
class RecreatePageFragment : public Fragment {
    int mPage = 0;
public:
    static RecreatePageFragment* newInstance(int page) {
        RecreatePageFragment* f = new RecreatePageFragment();
        f->mPage = page;
        return f;
    }
    View* onCreateView(cdroid::LayoutInflater*, ViewGroup* container,
                       cdroid::Bundle*) override {
        cdroid::ScrollView* sv = new cdroid::ScrollView(getContext());
        cdroid::LinearLayout* box = new cdroid::LinearLayout(getContext());
        box->setOrientation(cdroid::LinearLayout::VERTICAL);
        for (int i = 0; i < 30; i++) {
            cdroid::TextView* tv = new cdroid::TextView(getContext());
            tv->setText("page " + std::to_string(mPage) + " row " + std::to_string(i));
            tv->setPadding(8, 8, 8, 8);
            box->addView(tv);
        }
        sv->addView(box);
        return sv;
    }
};

class RecreatePagerAdapter : public cdroid::FragmentPagerAdapter {
public:
    RecreatePagerAdapter(FragmentManager* fm) : FragmentPagerAdapter(fm) {}
    Fragment* getItem(int position) override {
        return RecreatePageFragment::newInstance(position);
    }
    int getCount() override { return 4; }
};

class RecreateWindow : public FragmentActivity {
public:
    cdroid::ViewPager* mPager;
    RecreateWindow() : FragmentActivity(0, 0, -1, -1) {
        cdroid::ViewPager* pager = new cdroid::ViewPager(getContext());
        pager->setId(View::generateViewId());
        // Creator-owns rule (widgetsDemo pattern): anchor adapter deletion to
        // the pager's lifetime with an owned keyed tag.
        auto* adapter = new RecreatePagerAdapter(getSupportFragmentManager());
        pager->setAdapter(adapter);
        pager->setTag(View::generateViewId(), adapter,
                      [](void* p) { delete static_cast<RecreatePagerAdapter*>(p); });
        // Low limit: page flips tear down off-screen views through the
        // SpecialEffects exit pipeline (the leak path), unlike widgetsDemo's
        // keep-everything-attached limit of 9.
        pager->setOffscreenPageLimit(1);
        pager->setCurrentItem(0, false);
        addView(pager, new ViewGroup::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::MATCH_PARENT));
        mPager = pager;
    }
};

int main(int argc, const char* argv[]) {
    App app(argc, argv);
    // Rounds are driven from a process-lifetime handler, never from the doomed
    // window: a post on a closing window's queue is exactly the dropped-post
    // leak this sample hunts.
    cdroid::Handler driver;

    int round = 0;
    std::function<void()> tick = [&]() {
        if (round >= 16) { app.exit(0); return; }
        const bool midSettle = (round & 1) == 0;
        RecreateWindow* w = new RecreateWindow();
        cdroid::ViewPager* pager = w->mPager;
        // Flip far enough that off-screen pages get destroyed through the SEC.
        pager->postDelayed([pager, w, midSettle]() {
            pager->setCurrentItem(3, midSettle);
            // Mid-settle: close while the exit transition / reclaim hops are
            // still pending. Late: close after everything settled.
            w->postDelayed([w]() { w->close(); }, midSettle ? 250 : 1800);
        }, 600);
        round++;
        driver.postDelayed(tick, midSettle ? 1200 : 3000);
    };

    tick();
    app.exec();
    return 0;
}
