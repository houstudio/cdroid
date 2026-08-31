#include <app/uiautomation.h>
#include <core/looper.h>
#include <core/handler.h>
#include <core/systemclock.h>
#include <view/accessibility/accessibilitymanager.h>
#include <view/accessibility/accessibilitynodeinfo.h>
#include <accessibilityservice/accessibilityservice.h>
#include <core/inputeventsource.h>
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
        // FLAG_REPORT_VIEW_IDS is what AOSP's UiAutomation connection sets —
        // nodes then carry viewIdResourceName (id= selectors see real ids).
        info.flags = AccessibilityServiceInfo::FLAG_REQUEST_TOUCH_EXPLORATION_MODE
                | AccessibilityServiceInfo::FLAG_REPORT_VIEW_IDS;
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

// AOSP routes this over the service connection (IAccessibilityServiceConnection
// → AccessibilityManagerService); in-process that is the backing service.
bool UiAutomation::performGlobalAction(int action) {
    return mService ? mService->performGlobalAction(action) : false;
}

// AOSP routes injection over the IUiAutomationConnection to InputManager;
// in-process the InputEventSource queue (the InputDispatcher analog) plays
// that role — the event rides the same drain as device input.
bool UiAutomation::injectInputEvent(InputEvent& event, bool sync) {
    return injectInputEvent(event, sync ? InputEventSource::INJECT_INPUT_EVENT_MODE_WAIT_FOR_FINISH
                                        : InputEventSource::INJECT_INPUT_EVENT_MODE_ASYNC);
}

bool UiAutomation::injectInputEvent(InputEvent& event, int injectMode) {
    return InputEventSource::getInstance().injectInputEvent(event, injectMode);
}

AccessibilityEvent* UiAutomation::executeAndWaitForEvent(const std::function<void()>& command,
        const AccessibilityEventFilter& filter, long timeoutMillis) {
    if (!mService) return nullptr;
    // The pump below can run a posted, already-due step of the caller (a
    // re-entrant wait): this entry then overwrites a match the OUTER wait has
    // not returned yet — recycle the orphan first, nobody else will.
    if (mWaitMatch) mWaitMatch->recycle();
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
    // Transfer the match to the caller (it recycles); clearing the member
    // also stops a re-entrant wait from stomping a live pointer.
    AccessibilityEvent* result = mWaitMatch;
    mWaitMatch = nullptr;
    return result;
}

} /*endof namespace*/
