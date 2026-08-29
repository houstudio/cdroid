#ifndef __ACCESSIBILITY_SERVICE_H__
#define __ACCESSIBILITY_SERVICE_H__
#include <string>
#include <vector>
#include <accessibilityservice/accessibilityserviceinfo.h>

namespace cdroid {
class AccessibilityEvent;
class AccessibilityNodeInfo;
class Window;
class KeyEvent;

/**
 * AOSP android.accessibilityservice.AccessibilityService, in-process port.
 *
 * CDROID collapses the binder seam: what AOSP splits into the service app,
 * IAccessibilityServiceConnection and AccessibilityManagerService, lives in
 * one process, so the service talks to AccessibilityManager directly (the
 * InputMethodManager-adapter idiom). Registration is programmatic —
 * AccessibilityManager::addAccessibilityService(this) plays the system's
 * "bind the service" step, fires onServiceConnected, and enables the
 * event pipeline; ~AccessibilityService unregisters.
 *
 * Events delivered to onAccessibilityEvent are pooled and recycled after
 * dispatch (AOSP's caller-owned rule): a service that keeps one must copy.
 */
class AccessibilityService {
public:
    // AOSP GLOBAL_ACTION_* subset (the ones a window-manager-only system can honor)
    static constexpr int GLOBAL_ACTION_BACK = 1;
    static constexpr int GLOBAL_ACTION_HOME = 2;
    static constexpr int GLOBAL_ACTION_RECENTS = 3;
    static constexpr int GLOBAL_ACTION_NOTIFICATIONS = 4;
    static constexpr int GLOBAL_ACTION_QUICK_SETTINGS = 5;
    static constexpr int GLOBAL_ACTION_POWER_DIALOG = 6;
    static constexpr int GLOBAL_ACTION_TOGGLE_SPLIT_SCREEN = 7;
    static constexpr int GLOBAL_ACTION_LOCK_SCREEN = 8;
    static constexpr int GLOBAL_ACTION_TAKE_SCREENSHOT = 9;

    /** Focus types for findFocus(). */
    static constexpr int FOCUS_INPUT = 1;
    static constexpr int FOCUS_ACCESSIBILITY = 2;

    AccessibilityService();
    virtual ~AccessibilityService();

    /** The system is bound and the connection is live (AOSP fires this after
     *  the wrapper registered the connection). */
    virtual void onServiceConnected();
    /** Callback for AccessibilityEvents filtered by this service's info. */
    virtual void onAccessibilityEvent(AccessibilityEvent& event) = 0;
    /** The system wants the service to stop interrupting feedback. */
    virtual void onInterrupt();
    /** The service is being destroyed (AOSP Service.onDestroy). */
    virtual void onDestroy();

    /** The service's configuration; updates the manager's event routing. */
    AccessibilityServiceInfo getServiceInfo() const;
    void setServiceInfo(const AccessibilityServiceInfo& info);

    /** The windows an a11y service can see (flag/capability gated in AOSP;
     *  the manager trusts its services in-process). */
    std::vector<Window*> getWindows();
    /** The root AccessibilityNodeInfo of the active window, or nullptr. */
    AccessibilityNodeInfo* getRootInActiveWindow();
    /** The node with input (FOCUS_INPUT) or accessibility (FOCUS_ACCESSIBILITY) focus. */
    AccessibilityNodeInfo* findFocus(int focus);

    /** Perform a global action; returns false when unsupported. */
    bool performGlobalAction(int action);

    /** Off to the system: unbind this service (AOSP disableSelf). */
    void disableSelf();

private:
    friend class AccessibilityManager;
    AccessibilityServiceInfo mInfo;
    bool mConnected = false;
};

} /*endof namespace*/
#endif/*__ACCESSIBILITY_SERVICE_H__*/
