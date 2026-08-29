#include <app/uiautomation.h>
#include <core/looper.h>
#include <core/handler.h>
#include <core/systemclock.h>
#include <view/accessibility/accessibilitymanager.h>
#include <view/accessibility/accessibilitynodeinfo.h>
#include <accessibilityservice/accessibilityservice.h>
#include <porting/cdlog.h>

namespace cdroid {

// The registered service side of the connection: AOSP's
// IAccessibilityServiceClient wrapper, collapsed onto the in-process base.
class UiAutomation::BackingService : public AccessibilityService {
public:
    explicit BackingService(UiAutomation* owner) : mOwner(owner) {}

    void onServiceConnected() override {
        AccessibilityServiceInfo info;
        // A UiAutomation wants everything (FLAG_DEFAULT; the test connection
        // bypasses filtering in AOSP — listening to all types is the
        // in-process equivalent).
        info.eventTypes = AccessibilityEvent::TYPES_ALL_MASK;
        info.feedbackType = AccessibilityServiceInfo::FEEDBACK_GENERIC;
        info.setCapabilities(AccessibilityServiceInfo::CAPABILITY_CAN_RETRIEVE_WINDOW_CONTENT);
        // The focus-rect drawing is gated on touch-exploration state; a test
        // driver wants its operations visible on screen, same as the scanner.
        info.flags = AccessibilityServiceInfo::FLAG_REQUEST_TOUCH_EXPLORATION_MODE;
        setServiceInfo(info);
    }

    void onAccessibilityEvent(AccessibilityEvent& event) override {
        if (mOwner->mWaitFilter && mOwner->mWaitMatch == nullptr
                && mOwner->mWaitFilter(event)) {
            // The dispatched event is pooled and recycled after dispatch —
            // the waiter keeps a copy (caller recycles it).
            mOwner->mWaitMatch = AccessibilityEvent::obtain(event);
        }
        if (mOwner->mListener) mOwner->mListener(event);
    }

    void onInterrupt() override {}

private:
    UiAutomation* mOwner;
};

UiAutomation& UiAutomation::getInstance() {
    static UiAutomation sInstance;
    return sInstance;
}

UiAutomation::UiAutomation() = default;

UiAutomation::~UiAutomation() {
    disconnect();
}

void UiAutomation::connect() {
    if (mService) return;
    mService = std::make_shared<BackingService>(this);
    AccessibilityManager::getInstance(nullptr).addAccessibilityService(mService.get());
}

void UiAutomation::disconnect() {
    if (!mService) return;
    AccessibilityManager::getInstance(nullptr).removeAccessibilityService(mService.get());
    mService.reset();
}

void UiAutomation::setOnAccessibilityEventListener(const OnAccessibilityEventListener& listener) {
    mListener = listener;
}

AccessibilityNodeInfo* UiAutomation::getRootInActiveWindow() {
    return mService ? mService->getRootInActiveWindow() : nullptr;
}

AccessibilityEvent* UiAutomation::executeAndWaitForEvent(const std::function<void()>& command,
        const AccessibilityEventFilter& filter, long timeoutMillis) {
    if (!mService) return nullptr;
    mWaitMatch = nullptr;
    mWaitFilter = filter;
    // A standing heap handler, not a temporary: the posted command must
    // outlive this call frame (the cdwindow teardown-post idiom).
    static Handler sCommandHandler(Looper::getMainLooper());
    sCommandHandler.post(command);
    const long deadline = SystemClock::uptimeMillis() + timeoutMillis;
    while (mWaitMatch == nullptr && SystemClock::uptimeMillis() < deadline) {
        // Pump the queue so posted work (the command, layout, teardown
        // posts) keeps flowing while blocked.
        Looper::getMainLooper()->pollOnce(20);
    }
    mWaitFilter = nullptr;
    return mWaitMatch;  // caller recycles; nullptr on timeout
}

} /*endof namespace*/
