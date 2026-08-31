#include <accessibilityservice/accessibilityservice.h>
#include <core/looper.h>
#include <core/systemclock.h>
#include <core/windowmanager.h>
#include <view/accessibility/accessibilitymanager.h>
#include <view/accessibility/accessibilityevent.h>
#include <view/accessibility/accessibilitynodeinfo.h>
#include <view/view.h>
#include <view/keyevent.h>
#include <widget/cdwindow.h>
#include <porting/cdlog.h>

namespace cdroid {

AccessibilityService::AccessibilityService() = default;

AccessibilityService::~AccessibilityService() {
    // The system unbinds a destroyed service (AOSP Service.onDestroy path).
    onDestroy();
    AccessibilityManager::getInstance(nullptr).removeAccessibilityService(this);
}

void AccessibilityService::onServiceConnected() {
}

void AccessibilityService::onInterrupt() {
}

void AccessibilityService::onDestroy() {
}

AccessibilityServiceInfo AccessibilityService::getServiceInfo() const {
    return mInfo;
}

void AccessibilityService::setServiceInfo(const AccessibilityServiceInfo& info) {
    mInfo = info;
    // AOSP pushes the new info to the connection; the manager recomputes its
    // event routing from the bound services' infos.
    if (mConnected) {
        AccessibilityManager::getInstance(nullptr).onServiceInfoChanged(this);
    }
}

std::vector<Window*> AccessibilityService::getWindows() {
    std::vector<Window*> windows;
    WindowManager::getInstance().getWindows(windows);
    return windows;
}

AccessibilityNodeInfo* AccessibilityService::getRootInActiveWindow() {
    // AOSP's active window is the top APPLICATION window: the IME may hold
    // input focus (and the key dispatch) without ever becoming the a11y
    // active window — reading its tree here starved drivers while typing.
    Window* active = WindowManager::getInstance().getActiveApplicationWindow();
    if (active == nullptr) {
        return nullptr;
    }
    // AOSP's AccessibilityInteractionClient maps the service's capability
    // flags onto the per-request node fetch flags, and the ViewRootImpl side
    // writes them into AttachInfo before nodes materialize — mirror that on
    // the in-process bridge (they gate viewIdResourceName reporting and
    // include-not-important filtering in View.onInitializeAccessibilityNodeInfo).
    int fetchFlags = 0;
    const int serviceFlags = getServiceInfo().flags;
    if (serviceFlags & AccessibilityServiceInfo::FLAG_REPORT_VIEW_IDS) {
        fetchFlags |= AccessibilityNodeInfo::FLAG_REPORT_VIEW_IDS;
    }
    if (serviceFlags & AccessibilityServiceInfo::FLAG_INCLUDE_NOT_IMPORTANT_VIEWS) {
        fetchFlags |= AccessibilityNodeInfo::FLAG_INCLUDE_NOT_IMPORTANT_VIEWS;
    }
    WindowManager::getInstance().setAccessibilityFetchFlags(active, fetchFlags);
    // Seal at the service boundary (AOSP: sealed when ViewRootImpl marshals
    // the reply) — the framework keeps factory nodes unsealed.
    AccessibilityNodeInfo* root = active->createAccessibilityNodeInfo();
    if (root != nullptr) root->setSealed(true);
    return root;
}

AccessibilityNodeInfo* AccessibilityService::findFocus(int focus) {
    Window* active = WindowManager::getInstance().getActiveApplicationWindow();
    if (active == nullptr) {
        return nullptr;
    }
    View* source = nullptr;
    if (focus == FOCUS_ACCESSIBILITY) {
        source = active->getAccessibilityFocusedHost();
    } else {
        source = active->findFocus();
    }
    if (source == nullptr) return nullptr;
    AccessibilityNodeInfo* node = source->createAccessibilityNodeInfo();
    if (node != nullptr) node->setSealed(true);  // sealed snapshot at the boundary
    return node;
}

bool AccessibilityService::performGlobalAction(int action) {
    switch (action) {
    case GLOBAL_ACTION_BACK: {
        // Synthesize a BACK key through the normal input pipeline (AOSP asks the
        // window manager via the connection; in-process that is processEvent).
        const nsecs_t now = SystemClock::uptimeMillis();
        KeyEvent* down = KeyEvent::obtain(now, now, KeyEvent::ACTION_DOWN, KeyEvent::KEYCODE_BACK,
                0, 0, 0, KeyEvent::KEYCODE_BACK, 0, 0x101 /* SOURCE_KEYBOARD */);
        WindowManager::getInstance().processEvent(*down);
        down->recycle();
        KeyEvent* up = KeyEvent::obtain(now, now, KeyEvent::ACTION_UP, KeyEvent::KEYCODE_BACK,
                0, 0, 0, KeyEvent::KEYCODE_BACK, 0, 0x101 /* SOURCE_KEYBOARD */);
        WindowManager::getInstance().processEvent(*up);
        up->recycle();
        return true;
    }
    default:
        // HOME/RECENTS/etc. need an activity-stack manager CDROID does not have yet.
        LOGI("performGlobalAction(%d) not supported yet", action);
        return false;
    }
}

void AccessibilityService::disableSelf() {
    AccessibilityManager::getInstance(nullptr).removeAccessibilityService(this);
}

} /*endof namespace*/
