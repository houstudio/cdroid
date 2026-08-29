/*********************************************************************************
 * widgetsDemo: a widget showcase framed as a smart-home control panel
 * ("CDroid Home"). Fragment-based — each of the 10 tab pages is a DemoPageFragment
 * driven by the ported androidx FragmentPagerAdapter on a classic ViewPager,
 * with TabLayout wired via setupWithViewPager. The per-page widget demos and
 * their wiring live in pages.cc.
 *********************************************************************************/
#include <core/app.h>
#include <cdroid.h>
#include <core/systemclock.h>
#include <core/handler.h>
#include <core/looper.h>
#include <accessibilityservice/accessibilityservice.h>
#include <app/uiautomation.h>
#include <view/accessibility/accessibilitymanager.h>
#include <view/accessibility/accessibilityevent.h>
#include <view/accessibility/accessibilitynodeinfo.h>
#include <cstdlib>
#include <core/activityfactory.h>
#include <content/LocaleList.h>
#include <app/alertdialog.h>
#include <view/layoutinflater.h>
#include <widget/button.h>
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
            // Creator-owns rule: the adapter is ours, not the pager's — anchor its
            // deletion to the pager's lifetime with an owned keyed tag.
            auto* pagerAdapter = new DemoFragmentPagerAdapter(getSupportFragmentManager());
            pager->setAdapter(pagerAdapter);
            pager->setTag(View::generateViewId(), pagerAdapter,
                          [](void* p) { delete static_cast<DemoFragmentPagerAdapter*>(p); });
            // 10 small static pages: keep them all attached (androidx guidance for
            // FragmentPagerAdapter) instead of tearing down/rebuilding views through
            // the SpecialEffects exit pipeline on every tab hop.
            pager->setOffscreenPageLimit(9);
            // Start on the progress page (page 1) instead of the buttons page.
            pager->setCurrentItem(1, false);
            host->addView(pager, new ViewGroup::LayoutParams(
                    ViewGroup::LayoutParams::MATCH_PARENT, ViewGroup::LayoutParams::MATCH_PARENT));
            if (mTabs) mTabs->setupWithViewPager(pager);

            // Valgrind/CI driver: WIDGETSDEMO_AUTOCYCLE sweeps every page once
            // (0..count-1), then exits cleanly so the leak-check report lands.
            // No input path involved — valgrind's serialized virtual CPU makes
            // evdev/touch interaction impractical (clicks starve).
            if (getenv("WIDGETSDEMO_AUTOCYCLE")) {
                struct Cycle {
                    static void step(ViewPager* p, int page) {
                        const int n = (p->getAdapter() != nullptr) ? p->getAdapter()->getCount() : 0;
                        if (n == 0 || page >= n) { App::getInstance().exit(0); return; }
                        p->setCurrentItem(page, false);
                        p->postDelayed([p, page]() { Cycle::step(p, page + 1); }, 5000);
                    }
                };
                pager->postDelayed([pager]() { Cycle::step(pager, 0); }, 5000);
            }

        // Locale cycle button (top-right): flips zh-CN <-> en-US through the
        // AOSP-style configuration-change path. The manifest declares
        // android:configChanges="locale", so the window takes the in-place
        // dispatch (View.onConfigurationChanged) instead of recreate().
        if (Button* localeBtn = (Button*)root->findViewById(widgetsDemo::R::id::btn_locale)) {
            auto applyButtonLabel = [localeBtn]() {
                const std::string cur = App::getInstance().getResources()
                        .getConfiguration().getLocales().get(0).toLanguageTag();
                localeBtn->setText(cur == "zh-CN" ? "EN" : "中文");
            };
            localeBtn->setOnClickListener([applyButtonLabel](View&) {
                App& app = App::getInstance();
                const std::string cur = app.getResources().getConfiguration()
                        .getLocales().get(0).toLanguageTag();
                const char* next = (cur == "zh-CN") ? "en-US" : "zh-CN";
                // Copy the live config and change only the locale, so the
                // diff is CONFIG_LOCALE alone.
                Configuration cfg = app.getResources().getConfiguration();
                cfg.setLocales(LocaleList(std::vector<Locale>{
                        Locale::forLanguageTag(next)}));
                app.handleConfigurationChanged(cfg);
                applyButtonLabel();
            });
            applyButtonLabel();
        }

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

// A11Y_DUMP=1: register a tree-dumping accessibility service and walk every
// node of the active window on each page switch — combined with
// WIDGETSDEMO_AUTOCYCLE this is a full-zoo a11y enumeration report.
namespace {
void dumpNode(AccessibilityNodeInfo* node, int depth) {
    if (node == nullptr || depth > 24) return;
    Rect b; node->getBoundsInScreen(b);
    // Screen-reader visibility filter: ViewPager keeps every page attached,
    // so the tree always contains ALL pages — readers announce only what is
    // on the display (AOSP isVisibleToUser semantics). Skip offscreen
    // subtrees (children clip inside their parent).
    if (b.left >= 1280 || b.top >= 720 || b.left + b.width <= 0 || b.top + b.height <= 0) {
        node->recycle();
        return;
    }
    LOGD("A11YTREE %*s%s text=[%s] clickable=%d checkable=%d enabled=%d bounds=(%d,%d %dx%d)",
         depth * 2, "", node->getClassName().c_str(), node->getText().c_str(),
         (int)node->isClickable(), (int)node->isCheckable(), (int)node->isEnabled(),
         b.left, b.top, b.width, b.height);
    for (int i = 0; i < node->getChildCount(); i++) {
        dumpNode(node->getChild(i), depth + 1);
    }
    node->recycle();  // AOSP consumer contract: nodes are recycled after use
}
// A11Y_DRIVE=1: after each page's dump, drive the page semantically — find
// the first on-screen clickable, activate it with performAction(ACTION_CLICK)
// and wait for the resulting TYPE_VIEW_CLICKED event through UiAutomation.
// This is the xclick replacement: no coordinates, no window geometry.
void driveStep(UiAutomation& automation) {
    AccessibilityNodeInfo* root = automation.getRootInActiveWindow();
    if (root == nullptr) return;
    AccessibilityNodeInfo* target = nullptr;
    std::function<void(AccessibilityNodeInfo*, int)> pick =
        [&](AccessibilityNodeInfo* node, int depth) {
            if (target != nullptr || node == nullptr || depth > 12) return;
            Rect b; node->getBoundsInScreen(b);
            if (b.left < 1280 && b.top < 720 && b.left + b.width > 0 && b.top + b.height > 0) {
                const std::string text = node->getText();
                if (node->isClickable() && text != "EN") { target = node; return; }
            }
            for (int i = 0; i < node->getChildCount() && target == nullptr; i++) {
                pick(node->getChild(i), depth + 1);
            }
        };
    pick(root, 0);
    if (target == nullptr) { LOGD("A11YDRIVE no clickable on this page"); root->recycle(); return; }
    const std::string label = target->getText();
    AccessibilityNodeInfo* keepRoot = root;  (void)keepRoot;
    AccessibilityEvent* hit = automation.executeAndWaitForEvent(
        [target]() { target->performAction(AccessibilityNodeInfo::ACTION_CLICK); },
        [](AccessibilityEvent& e) { return e.getEventType() == AccessibilityEvent::TYPE_VIEW_CLICKED; },
        1500);
    LOGD("A11YDRIVE click [%s] -> %s", label.c_str(), hit ? "event HIT" : "timeout");
    if (hit) hit->recycle();
    // NOTE: picked/target nodes ride the same pool as the walk; the pool
    // recycles them on later obtains — the driver demo keeps it simple.
}

class DumpService : public AccessibilityService {
public:
    void onServiceConnected() override {
        AccessibilityServiceInfo info;
        info.eventTypes = AccessibilityEvent::TYPE_WINDOW_STATE_CHANGED
                        | AccessibilityEvent::TYPE_WINDOW_CONTENT_CHANGED;
        info.feedbackType = AccessibilityServiceInfo::FEEDBACK_GENERIC;
        setServiceInfo(info);
        LOGD("A11YTREE service connected");
    }
    void onAccessibilityEvent(AccessibilityEvent& event) override {
        // ViewPager paging fires content-changed, not window-state; throttle
        // to one dump per 2s (AUTOCYCLE dwells 5s per page -> ~2 dumps each).
        const long now = SystemClock::uptimeMillis();
        if (now - mLastDumpMs < 2000) return;
        mLastDumpMs = now;
        // Dump OUT of the event dispatch stack: a page-switch event fires while
        // the outgoing page is tearing down, and walking that tree dereferences
        // views the adapter already freed. The posted dump runs a looper turn
        // later, on the settled tree.
        static Handler sDumpHandler(Looper::getMainLooper());
        sDumpHandler.post([this]() {
            AccessibilityNodeInfo* root = getRootInActiveWindow();
            LOGD("A11YTREE ==== page begin ====");
            dumpNode(root, 0);
            LOGD("A11YTREE ==== page end ====");
            if (sDrive != nullptr) {
                driveStep(*sDrive);
            }
        });
    }
    void onInterrupt() override {}

    static UiAutomation* sDrive;  // non-null under A11Y_DRIVE
private:
    long mLastDumpMs = 0;
};
UiAutomation* DumpService::sDrive = nullptr;
}// namespace

int main(int argc, const char* argv[]) {
    setvbuf(stdout, nullptr, _IONBF, 0);
    setvbuf(stderr, nullptr, _IONBF, 0);
    App app(argc, argv);
    // Stack lifetime (NOT static): unregistering from ~AccessibilityService
    // during exit() would race the manager's own static destruction order.
    DumpService dumpService;
    if (getenv("A11Y_DUMP")) {
        AccessibilityManager::getInstance(&app).addAccessibilityService(&dumpService);
    }
    UiAutomation driveAutomation;
    if (getenv("A11Y_DRIVE")) {
        driveAutomation.connect();
        DumpService::sDrive = &driveAutomation;
    }
    // The launcher activity starts itself from the manifest (App plays the
    // system side when no window is up).
    return app.exec();
}
