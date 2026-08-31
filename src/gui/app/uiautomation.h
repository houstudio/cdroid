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
class InputEvent;
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

    /** AOSP UiAutomation.performGlobalAction(int): a system-level action
     *  (AccessibilityService::GLOBAL_ACTION_*). False when unsupported —
     *  HOME/RECENTS/... need an activity-stack manager CDROID does not have
     *  (AOSP returns false for undeliverable actions too). */
    bool performGlobalAction(int action);

    /** AOSP UiAutomation.injectInputEvent(InputEvent, boolean): inject an
     *  input event through the input pipeline. AOSP routes it over the
     *  IUiAutomationConnection to InputManager, where the InputDispatcher
     *  queues it behind real events and dispatches it like a device event;
     *  in-process the InputEventSource drain queue plays that role. The
     *  caller keeps ownership of its event. sync=true maps to
     *  INJECT_INPUT_EVENT_MODE_WAIT_FOR_FINISH, false to ASYNC — both only
     *  enqueue here: this class runs on the UI (delivery) thread, so a
     *  blocking wait would deadlock (AOSP blocks its instrumentation thread
     *  instead). Pump the looper (executeAndWaitForEvent) to observe the
     *  delivery. */
    bool injectInputEvent(InputEvent& event, bool sync);

    /** The hidden variant taking the InputManager mode directly. */
    bool injectInputEvent(InputEvent& event, int injectMode);

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
