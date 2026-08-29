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
    Window* active = WindowManager::getInstance().getActiveWindow();
    if (active == nullptr) {
        return nullptr;
    }
    return active->createAccessibilityNodeInfo();
}

AccessibilityNodeInfo* AccessibilityService::findFocus(int focus) {
    Window* active = WindowManager::getInstance().getActiveWindow();
    if (active == nullptr) {
        return nullptr;
    }
    if (focus == FOCUS_ACCESSIBILITY) {
        View* host = active->getAccessibilityFocusedHost();
        return host != nullptr ? host->createAccessibilityNodeInfo() : nullptr;
    }
    View* focused = active->findFocus();
    return focused != nullptr ? focused->createAccessibilityNodeInfo() : nullptr;
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
