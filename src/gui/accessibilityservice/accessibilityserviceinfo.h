#ifndef __ACCESSIBILITY_SERVICE_INFO_H__
#define __ACCESSIBILITY_SERVICE_INFO_H__
#include <string>
#include <vector>

namespace cdroid {
class AccessibilityServiceInfo {
public:
    /** Capability: this service can retrieve window content. */
    static constexpr int CAPABILITY_CAN_RETRIEVE_WINDOW_CONTENT = 1 /* << 0 */;
    /** Capability: this service can request touch exploration mode. */
    static constexpr int CAPABILITY_CAN_REQUEST_TOUCH_EXPLORATION = 1 << 1;
    /** Capability: this service can request enhanced web accessibility. */
    static constexpr int CAPABILITY_CAN_REQUEST_ENHANCED_WEB_ACCESSIBILITY = 1 << 2;
    /** Capability: this service can request to filter key events. */
    static constexpr int CAPABILITY_CAN_REQUEST_FILTER_KEY_EVENTS = 1 << 3;
    /** Capability: this service can control magnification. */
    static constexpr int CAPABILITY_CAN_CONTROL_MAGNIFICATION = 1 << 4;
    /** Capability: this service can perform gestures. */
    static constexpr int CAPABILITY_CAN_PERFORM_GESTURES = 1 << 5;
    /** Capability: this service can capture fingerprint gestures. */
    static constexpr int CAPABILITY_CAN_REQUEST_FINGERPRINT_GESTURES = 1 << 6;
    /** Capability: this service can take screenshot. */
    static constexpr int CAPABILITY_CAN_TAKE_SCREENSHOT = 1 << 7;

    /** Denotes spoken feedback. */
    static constexpr int FEEDBACK_SPOKEN = 1 /* << 0 */;
    /** Denotes haptic feedback. */
    static constexpr int FEEDBACK_HAPTIC = 1 << 1;
    /** Denotes audible (not spoken) feedback. */
    static constexpr int FEEDBACK_AUDIBLE = 1 << 2;
    /** Denotes visual feedback. */
    static constexpr int FEEDBACK_VISUAL = 1 << 3;
    /** Denotes generic feedback. */
    static constexpr int FEEDBACK_GENERIC = 1 << 4;
    /** Denotes braille feedback. */
    static constexpr int FEEDBACK_BRAILLE = 1 << 5;
    /** Mask for all feedback. */
    static constexpr int FEEDBACK_ALL_MASK = 0xFFFFFFFF;

    /** If an {@link AccessibilityServiceInfo} is NOT set to DEFAULT the containing
     *  service is only invoked after the user explicitly enables it. */
    static constexpr int DEFAULT = 1 /* << 0 */;

    /**
     * This flag requests that the AccessibilityNodeInfo objects retrieved by an
     * AccessibilityService contain the view IDs.
     */
    static constexpr int FLAG_REQUEST_ENHANCED_WEB_ACCESSIBILITY = 1 << 3;
    static constexpr int FLAG_REPORT_VIEW_IDS = 1 << 4;
    static constexpr int FLAG_INCLUDE_NOT_IMPORTANT_VIEWS = 1 << 1;
    static constexpr int FLAG_REQUEST_TOUCH_EXPLORATION_MODE = 1 << 2;
    static constexpr int FLAG_REQUEST_FILTER_KEY_EVENTS = 1 << 5;
    static constexpr int FLAG_RETRIEVE_INTERACTIVE_WINDOWS = 1 << 6;

    /** The event types an AccessibilityService wants to receive. */
    int eventTypes = 0;
    /** The package names an AccessibilityService wants to receive events from,
     *  empty = all packages (CDROID: the process is the only package producer). */
    std::vector<std::string> packageNames;
    /** The feedback type an AccessibilityService provides. */
    int feedbackType = FEEDBACK_GENERIC;
    /** The timeout after the most recent event of a given type before an
     *  AccessibilityService receives that event (ms). */
    long notificationTimeout = 0;
    /** The flags an AccessibilityService would like to have set. */
    int flags = 0;

    AccessibilityServiceInfo() = default;

    bool getCanRetrieveWindowContent() const {
        return (mCapabilities & CAPABILITY_CAN_RETRIEVE_WINDOW_CONTENT) != 0;
    }
    int getCapabilities() const { return mCapabilities; }
    void setCapabilities(int capabilities) { mCapabilities = capabilities; }

private:
    int mCapabilities = 0;
};

} /*endof namespace*/
#endif/*__ACCESSIBILITY_SERVICE_INFO_H__*/
