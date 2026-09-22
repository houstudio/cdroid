/*********************************************************************************
 * widgetsDemo: a tabbed widget showcase over the ApiDemos-style registry.
 *
 * One FragmentActivity hosts DemoTabsFragment as the root: every demo
 * self-registers at static init through REGISTER_DEMO_FRAGMENT and appears as
 * one scrollable tab (order = "/"-path order: Layouts/, then Views/). BACK at
 * the root exits. The ApiDemos-style category list shell
 * (demolistfragment.cc) stays available, unwired, until the final page-set
 * decision.
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
#include <fragment/fragmentactivity.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>
#include <R.h>
#include <typeinfo>
#include "demoregistry.h"
#include "demotabsfragment.h"

using namespace cdroid;

class WidgetsDemoActivity : public FragmentActivity {
    DemoTabsFragment* mRootTabs = nullptr;  // the seeded root tab screen
public:
    WidgetsDemoActivity() : FragmentActivity(0, 0, -1, -1) {}

    void onCreate(Bundle* savedInstanceState) override {
        FragmentActivity::onCreate(savedInstanceState);
        LayoutInflater::from(getContext())->inflate(widgetsDemo::R::layout::main, this, true);

        // Locale cycle button (header, top-right): flips zh-CN <-> en-US through
        // the AOSP-style configuration-change path. The manifest declares
        // android:configChanges="locale", so the window takes the in-place
        // dispatch (View.onConfigurationChanged) instead of recreate().
        if (Button* localeBtn = (Button*)findViewById(widgetsDemo::R::id::btn_locale)) {
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

        // Seed the tabbed root directly: every registered demo is one
        // scrollable tab (the pre-refactor widgetsDemo form — no category-list
        // hop first). The initial fragment is added — NOT addToBackStack'd
        // (the FragmentNavigator initial-navigation rule) — so BACK at the
        // root falls through to the activity, never popping to a blank screen.
        // The ApiDemos-style category list shell stays in demolistfragment.cc,
        // unwired, until the final page-set decision.
        if (findViewById(widgetsDemo::R::id::demo_content) != nullptr) {
            int initialPage = 0;
            if (const char* open = getenv("WIDGETSDEMO_OPEN")) {
                // WIDGETSDEMO_OPEN="Views/DateTime/Text Clock": land the root
                // tab screen on that demo's tab (headless navigation check —
                // bare Xvfb delivers no pointer input to this window).
                const std::vector<const DemoEntry*> leaves = DemoRegistry::get().leavesUnder("");
                for (size_t i = 0; i < leaves.size(); i++) {
                    if (leaves[i]->path == open) { initialPage = (int)i; break; }
                }
            }
            mRootTabs = DemoTabsFragment::newInstance("", initialPage);
            getSupportFragmentManager()->beginTransaction()
                ->add((int)widgetsDemo::R::id::demo_content, mRootTabs)
                .commit();
        }

        // Valgrind/CI driver: WIDGETSDEMO_AUTOCYCLE sweeps every registered
        // demo once through the root pager's tabs (setCurrentItem — the
        // pre-refactor app's sweep; replacing the root would dangle the
        // pager's page fragments), then exits cleanly for the leak-check
        // report. No input path involved — valgrind's serialized virtual CPU
        // makes evdev/touch interaction impractical (clicks starve).
        if (getenv("WIDGETSDEMO_AUTOCYCLE")) {
            struct Cycle {
                static void step(WidgetsDemoActivity* self) {
                    ViewPager* pager =
                            self->mRootTabs ? self->mRootTabs->getViewPager() : nullptr;
                    const int count = pager && pager->getAdapter()
                            ? pager->getAdapter()->getCount() : 0;
                    const int next = pager ? pager->getCurrentItem() + 1 : count;
                    if (next >= count) { App::getInstance().exit(0); return; }
                    pager->setCurrentItem(next, false);
                    self->postDelayed([self]() { Cycle::step(self); }, 5000);
                }
            };
            postDelayed([this]() {
                // The sweep's leak-check goal needs no focus; EditText pages
                // grabbing IME focus mid-switch races the page detach (segfault
                // under rapid switching), so block descendant focus first.
                if (mRootTabs && mRootTabs->getViewPager()) {
                    mRootTabs->getViewPager()->setDescendantFocusability(
                            ViewGroup::FOCUS_BLOCK_DESCENDANTS);
                }
                Cycle::step(this);
            }, 5000);
        }

        // TEMP STRESS HOOK: PRD_DIALOG=1 opens and dismisses an AlertDialog
        // every 400ms (deterministic repro for the decor leak; input clicks
        // vary in timing).
        if (getenv("PRD_DIALOG")) {
            static AlertDialog* sDialog = nullptr;
            auto step = new std::function<void()>;
            WidgetsDemoActivity* self = this;
            *step = [step, self]() {
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
                self->postDelayed([step](){ (*step)(); }, 400);
            };
            postDelayed([step](){ (*step)(); }, 400);
        }

        // TEMP STRESS HOOK: PRD_STRESS=1 churns pager tabs every 250ms
        // (teardown-UAF repro; same tab-churn shape as the old app).
        if (getenv("PRD_STRESS")) {
            auto step = new std::function<void()>;
            WidgetsDemoActivity* self = this;
            *step = [step, self]() {
                ViewPager* pager =
                        self->mRootTabs ? self->mRootTabs->getViewPager() : nullptr;
                if (pager && pager->getAdapter() && pager->getAdapter()->getCount() > 0) {
                    pager->setCurrentItem(
                            (pager->getCurrentItem() + 1) % pager->getAdapter()->getCount(),
                            false);
                }
                self->postDelayed([step](){ (*step)(); }, 250);
            };
            postDelayed([step](){ (*step)(); }, 250);
        }
    }
};

REGISTER_ACTIVITY(WidgetsDemoActivity);

// A11Y_DUMP=1: register a tree-dumping accessibility service and walk every
// node of the active window on each fragment switch — combined with
// WIDGETSDEMO_AUTOCYCLE this is a full-zoo a11y enumeration report.
namespace {
void dumpNode(AccessibilityNodeInfo* node, int depth) {
    if (node == nullptr || depth > 24) return;
    // AOSP visibility semantics: the node carries isVisibleToUser (window
    // visibility, ancestor alpha/visibility chain, global-rect clipping) —
    // offscreen content stays attached but reports false.
    if (!node->isVisibleToUser()) {
        node->recycle();
        return;
    }
    Rect b; node->getBoundsInScreen(b);
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
        // Fragment switches fire content-changed; throttle to one dump per 2s
        // (AUTOCYCLE dwells 5s per demo -> ~2 dumps each).
        const long now = SystemClock::uptimeMillis();
        if (now - mLastDumpMs < 2000) return;
        mLastDumpMs = now;
        // Dump OUT of the event dispatch stack: a switch event fires while
        // the outgoing fragment is tearing down, and walking that tree
        // dereferences views the manager already freed. The posted dump runs
        // a looper turn later, on the settled tree.
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
