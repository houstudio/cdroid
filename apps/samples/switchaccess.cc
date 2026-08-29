// Switch-access scanner: the accessibility pipeline's acceptance demo and the
// shortest path to real operational capability for a motor-impaired user.
//
// A SwitchScanner (an in-process AccessibilityService) walks the ACTIVE
// window's node tree, collects every clickable node, and drives them with two
// keys — F1 advances the scan, F2 activates the scanned node via
// performAction(ACTION_CLICK). No coordinates, no window positions: the
// interaction is purely semantic.
//
// F1/F2 land on the Window first; the demo buttons show the clicks happening.
#include <cdroid.h>
#include <widget/R.h>
#include <cdlog.h>
#include <algorithm>
#include <accessibilityservice/accessibilityservice.h>
#include <view/accessibility/accessibilitymanager.h>
#include <view/accessibility/accessibilityevent.h>
#include <view/accessibility/accessibilitynodeinfo.h>

using namespace cdroid;

namespace {
void collectClickable(AccessibilityNodeInfo* node, std::vector<AccessibilityNodeInfo*>& out) {
    if (node == nullptr) return;
    if (node->isClickable()) out.push_back(node);
    for (int i = 0; i < node->getChildCount(); i++) {
        collectClickable(node->getChild(i), out);
    }
}
std::string nodeLabel(AccessibilityNodeInfo* node) {
    const std::string text = node->getText();
    const std::string desc = node->getContentDescription();
    return text.length() ? text : (desc.length() ? desc : "(no text)");
}
}// namespace

class SwitchScanner : public AccessibilityService {
public:
    void onServiceConnected() override {
        AccessibilityServiceInfo info;
        info.eventTypes = AccessibilityEvent::TYPES_ALL_MASK;
        info.feedbackType = AccessibilityServiceInfo::FEEDBACK_GENERIC;
        info.setCapabilities(AccessibilityServiceInfo::CAPABILITY_CAN_RETRIEVE_WINDOW_CONTENT);
        // Drives the manager's touch-exploration state, which gates the
        // accessibility focus highlight drawing.
        info.flags = AccessibilityServiceInfo::FLAG_REQUEST_TOUCH_EXPLORATION_MODE;
        setServiceInfo(info);
        LOGD("SwitchScanner connected");
    }
    void onAccessibilityEvent(AccessibilityEvent& event) override {
        // A screen reader would speak here; the scanner just traces.
        LOGD("a11y event type=0x%x cls=%s", event.getEventType(), event.getClassName().c_str());
    }
    void onInterrupt() override {}

    // One switch = advance; the other = activate (the classic two-switch scan).
    // The list is a SNAPSHOT sorted by screen position: re-collecting on every
    // key made the order wobble (the picker's virtual nodes appear/disappear
    // with focus, 9<->10 items), so the index jumped around. Rebuild only on
    // wrap or when the scanned node no longer resolves.
    void scan(Window* window) {
        if (mClickable.empty()) rebuild();
        if (mClickable.empty()) { LOGD("no clickable nodes"); return; }
        mScanIndex = (mScanIndex + 1) % mClickable.size();
        if (mScanIndex == 0) {  // wrapped: refresh the snapshot
            rebuild();
            if (mClickable.empty()) { LOGD("no clickable nodes"); return; }
            mScanIndex = 0;
        }
        AccessibilityNodeInfo* node = mClickable.at(mScanIndex);
        // Accessibility focus draws the highlight (view_accessibility_focused).
        node->performAction(AccessibilityNodeInfo::ACTION_ACCESSIBILITY_FOCUS);
        node->performAction(AccessibilityNodeInfo::ACTION_FOCUS);
        Rect bounds; node->getBoundsInScreen(bounds);
        LOGD("scan[%zu/%zu] %s bounds=(%d,%d %dx%d)", mScanIndex + 1, mClickable.size(),
                nodeLabel(node).c_str(), bounds.left, bounds.top, bounds.width, bounds.height);
    }
    void rebuild() {
        mClickable.clear();
        mScanIndex = (size_t)-1;
        AccessibilityNodeInfo* root = getRootInActiveWindow();
        if (root == nullptr) return;
        collectClickable(root, mClickable);
        std::sort(mClickable.begin(), mClickable.end(),
                  [](AccessibilityNodeInfo* a, AccessibilityNodeInfo* b) {
                      Rect ra, rb;
                      a->getBoundsInScreen(ra);
                      b->getBoundsInScreen(rb);
                      if (ra.top != rb.top) return ra.top < rb.top;
                      return ra.left < rb.left;
                  });
    }
    void activate() {
        if (mScanIndex >= mClickable.size()) return;
        AccessibilityNodeInfo* node = mClickable.at(mScanIndex);
        LOGD("activate %s", nodeLabel(node).c_str());
        node->performAction(AccessibilityNodeInfo::ACTION_CLICK);
    }
private:
    std::vector<AccessibilityNodeInfo*> mClickable;
    size_t mScanIndex = (size_t)-1;
};

class DemoWindow : public Window {
private:
    SwitchScanner* mScanner;
    TextView* mLog;
    std::string mNpInputName;
public:
    DemoWindow(SwitchScanner* scanner) : Window(0, 0, -1, -1), mScanner(scanner) {
        setBackgroundColor(0xFF202830);
        LinearLayout* content = new LinearLayout(&App::getInstance());
        content->setOrientation(LinearLayout::VERTICAL);
        addView(content);

        mLog = new TextView(&App::getInstance());
        mLog->setText("F1=scan  F2=activate");
        mLog->setTextSize(22);
        content->addView(mLog);

        // The scan targets: every button below is reachable purely via nodes.
        const char* names[] = { "Alpha", "Bravo", "Charlie", "Delta", "Echo" };
        for (const char* name : names) {
            Button* btn = new Button(&App::getInstance());
            btn->setText(name);
            btn->setClickable(true);
            btn->setOnClickListener([this, name](View&) {
                mLog->setText(std::string("clicked: ") + name);
            });
            content->addView(btn);
        }
        CheckBox* chk = new CheckBox(&App::getInstance());
        chk->setText("a checkbox too");
        content->addView(chk);

        // Virtual-view exercise: NumberPicker exposes its +/-/input as virtual
        // nodes via its AccessibilityNodeProvider — the scanner walks them like
        // any other node, and clicking the increment virtual node drives the
        // real picker (onValueChanged below proves it end-to-end).
        NumberPicker* np = new NumberPicker(&App::getInstance());
        View* npInput = np->findViewById(cdroid::R::id::numberpicker_input);
        std::string npInputName;
        if (npInput != nullptr) {
            App::getInstance().getResources().getResourceName(npInput->getId(), &npInputName);
            LOGD("picker input view id name: %s", npInputName.c_str());
        }
        mNpInputName = npInputName;
        np->setMinValue(1);
        np->setMaxValue(12);
        np->setValue(7);
        np->setMinHeight(160);
        np->setOnValueChangedListener([](NumberPicker&, int prev, int value) {
            LOGD("numberpicker value %d->%d (virtual click worked)", prev, value);
        });
        content->addView(np);
    }
    bool onKeyDown(int keyCode, KeyEvent& event) override {
        if (keyCode == KeyEvent::KEYCODE_F1) { mScanner->scan(this); return true; }
        if (keyCode == KeyEvent::KEYCODE_F2) { mScanner->activate(); return true; }
        if (keyCode == KeyEvent::KEYCODE_F3) { performGlobalActionFromScanner(); return true; }
        if (keyCode == KeyEvent::KEYCODE_F4) {
            // ByViewId exercise: search the tree by the picker input's
            // fully-qualified resource name.
            AccessibilityNodeInfo* root = mScanner->getRootInActiveWindow();
            if (root != nullptr && mNpInputName.length()) {
                auto hits = root->findAccessibilityNodeInfosByViewId(mNpInputName);
                for (auto* n : hits) {
                    LOGD("ByViewId hit: %s (text=%s)", mNpInputName.c_str(), n->getText().c_str());
                }
                LOGD("ByViewId %s -> %zu hit(s)", mNpInputName.c_str(), hits.size());
            }
            return true;
        }
        return Window::onKeyDown(keyCode, event);
    }
    void performGlobalActionFromScanner() {
        // GLOBAL_ACTION_BACK goes through the real input pipeline.
        LOGD("global BACK: %d", mScanner->performGlobalAction(AccessibilityService::GLOBAL_ACTION_BACK));
    }
};

int main(int argc, const char* argv[]) {
    App app(argc, argv);

    SwitchScanner scanner;  // the assistive-tech client
    AccessibilityManager::getInstance(&app).addAccessibilityService(&scanner);

    new DemoWindow(&scanner);
    return app.exec();
}
