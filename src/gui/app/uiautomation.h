#ifndef __UI_AUTOMATION_H__
#define __UI_AUTOMATION_H__
#include <functional>
#include <memory>
#include <view/accessibility/accessibilityevent.h>
#include <accessibilityservice/accessibilityserviceinfo.h>

namespace cdroid {

class AccessibilityNodeInfo;
class AccessibilityService;
class Handler;
class Looper;

/**
 * AOSP android.app.UiAutomation, in-process port — the semantic UI driver.
 *
 * AOSP's UiAutomation owns an IAccessibilityServiceClient connection used by
 * test frameworks (UiDevice/Instrumentation) for semantic queries and event
 * waits. CDROID collapses that binder connection onto the in-process
 * AccessibilityService it registers on connect(), so a driver (tests,
 * automation, future TTS tooling) gets the same surface:
 *
 *   connect()                  register the backing service
 *   getRootInActiveWindow()    the active window's node tree root
 *   findAccessibilityNodeInfosByText / ByViewId (on the node)
 *   node->performAction(...)   semantic activation — no coordinates
 *   executeAndWaitForEvent()   run a command, pump until a matching event
 *
 * Intended to run on the main thread; executeAndWaitForEvent pumps the
 * looper while blocked (same idiom as the gui_test harness), so posted
 * work keeps flowing during the wait.
 */
class UiAutomation {
public:
    /** @see AccessibilityServiceInfo.flags — which to forward. */
    typedef std::function<void(AccessibilityEvent&)> OnAccessibilityEventListener;

    /** Event filter for executeAndWaitForEvent: true = this is the one. */
    typedef std::function<bool(AccessibilityEvent&)> AccessibilityEventFilter;

    static constexpr int FLAG_DONT_SUPPRESS_ACCESSIBILITY_SERVICES = 1;

    UiAutomation();
    ~UiAutomation();

    /** A shared default instance (the App-level --auto-test driver's). */
    static UiAutomation& getInstance();

    /** Registers the backing service with the AccessibilityManager. */
    void connect();
    /** Unregisters the backing service (AOSP disconnect). */
    void disconnect();

    void setOnAccessibilityEventListener(const OnAccessibilityEventListener& listener);

    /** The root node of the active window, or nullptr (caller recycles). */
    AccessibilityNodeInfo* getRootInActiveWindow();

    /** Runs command and pumps the main looper until filter matches or the
     *  timeout (ms) elapses; returns the matching event or nullptr. The
     *  event is a recycled-in-place snapshot — copy what you need. */
    AccessibilityEvent* executeAndWaitForEvent(const std::function<void()>& command,
            const AccessibilityEventFilter& filter, long timeoutMillis);

private:
    class BackingService;   // the registered AccessibilityService
    friend class BackingService;
    std::shared_ptr<BackingService> mService;
    OnAccessibilityEventListener mListener;
    AccessibilityEventFilter mWaitFilter;      // active during executeAndWaitForEvent
    AccessibilityEvent* mWaitMatch = nullptr;  // filter hit lands here (a copy)
};

} /*endof namespace*/
#endif/*__UI_AUTOMATION_H__*/
