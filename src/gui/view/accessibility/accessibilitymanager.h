#ifndef __ACCESSIBILITY_MANAGER_H__
#define __ACCESSIBILITY_MANAGER_H__
#include <string>
#include <vector>
#include <map>
#include <core/sparsearray.h>
#include <core/callbackbase.h>
#include <accessibilityservice/accessibilityserviceinfo.h>
namespace cdroid{
class Context;
class AccessibilityEvent;
class AccessibilityService;
class AccessibilityManager {
public:
    static constexpr int STATE_FLAG_ACCESSIBILITY_ENABLED = 0x00000001;
    static constexpr int STATE_FLAG_TOUCH_EXPLORATION_ENABLED = 0x00000002;
    static constexpr int STATE_FLAG_HIGH_TEXT_CONTRAST_ENABLED = 0x00000004;

    static constexpr int DALTONIZER_DISABLED = -1;
    static constexpr int DALTONIZER_SIMULATE_MONOCHROMACY = 0;
    static constexpr int DALTONIZER_CORRECT_DEUTERANOMALY = 12;
    static constexpr int AUTOCLICK_DELAY_DEFAULT = 600;
    static constexpr const char*ACTION_CHOOSE_ACCESSIBILITY_BUTTON ="com.android.internal.intent.action.CHOOSE_ACCESSIBILITY_BUTTON";
public:
    typedef CallbackBase<void,bool>AccessibilityStateChangeListener;
    typedef CallbackBase<void,bool>TouchExplorationStateChangeListener;
    typedef CallbackBase<void,AccessibilityManager&>AccessibilityServicesStateChangeListener;
    typedef CallbackBase<void,bool>HighTextContrastChangeListener;
#if 0
    struct /*interface*/ AccessibilityPolicy {
        bool isEnabled(bool accessibilityEnabled);
        AccessibilityEvent* onAccessibilityEvent(AccessibilityEvent& event, bool accessibilityEnabled, int relevantEventTypes);
        int getRelevantEventTypes(int relevantEventTypes);
        std::vector<AccessibilityServiceInfo> getInstalledAccessibilityServiceList(std::vector<AccessibilityServiceInfo> installedService);
        std::vector<AccessibilityServiceInfo> getEnabledAccessibilityServiceList(
                int feedbackTypeFlags, std::vector<AccessibilityServiceInfo> enabledService);
    };
#endif
private:
    static constexpr bool Debug = false;
private:
    //private IAccessibilityManager mService;
    int mUserId;
    //Handler mHandler;
    //Handler.Callback mCallback;
    bool mIsEnabled;
    int mRelevantEventTypes;// = AccessibilityEvent::TYPES_ALL_MASK;
    bool mIsTouchExplorationEnabled;
    bool mIsHighTextContrastEnabled;

    //AccessibilityPolicy mAccessibilityPolicy;

    std::vector<AccessibilityStateChangeListener> mAccessibilityStateChangeListeners;
    std::vector<TouchExplorationStateChangeListener> mTouchExplorationStateChangeListeners;
    std::vector<HighTextContrastChangeListener> mHighTextContrastStateChangeListeners;
    std::vector<AccessibilityServicesStateChangeListener>  mServicesStateChangeListeners;

    // In-process AccessibilityManagerService role (the binder seam collapsed,
    // InputMethodManager-adapter idiom): the bound services. Borrowed pointers —
    // a service unregisters itself in its destructor; nobody deletes them here.
    std::vector<AccessibilityService*> mBoundServices;

    //SparseArray<std::vector<AccessibilityRequestPreparer>> mRequestPreparerLists;
    //IAccessibilityManagerClient.Stub mClient;
private:
    void setStateLocked(int stateFlags);
    // Recompute enabled/touch-exploration/relevant-event state from the
    // bound services (AOSP: AMS state push on bind/unbind/info change).
    void updateBoundServicesStateLocked();
    //IAccessibilityManager getServiceLocked();
    //void tryConnectToServiceLocked(IAccessibilityManager service);
    void notifyAccessibilityStateChanged();
    void notifyTouchExplorationStateChanged();
    void notifyHighTextContrastStateChanged();
public:
    static AccessibilityManager& getInstance(Context* context);

    AccessibilityManager(Context* context/*, IAccessibilityManager service*/, int userId);

    //AccessibilityManager(Handler handler, IAccessibilityManager service, int userId);

    //IAccessibilityManagerClient getClient();

    //Handler.Callback getCallback();

    bool isEnabled();

    bool isTouchExplorationEnabled();

    bool isHighTextContrastEnabled();

    void sendAccessibilityEvent(AccessibilityEvent& event);

    // --- In-process AccessibilityManagerService seam -------------------------
    /** The system binds the service: registers it, recomputes the relevant
     *  event types, flips the enabled state and fires onServiceConnected. */
    void addAccessibilityService(AccessibilityService* service);
    /** Unbind (disableSelf / service destructor); flips enabled off with the
     *  last service gone. Idempotent. */
    void removeAccessibilityService(AccessibilityService* service);
    /** A bound service pushed a new info — refresh the event routing. */
    void onServiceInfoChanged(AccessibilityService* service);
    /** The enabled services providing the given feedback types. */
    std::vector<AccessibilityServiceInfo> getEnabledAccessibilityServiceList(int feedbackTypeFlags);
    /** All bound services get onInterrupt (AOSP manager.interrupt). */
    void interrupt();

#if 0
    std::vector<ServiceInfo> getAccessibilityServiceList();
    std::vector<AccessibilityServiceInfo> getInstalledAccessibilityServiceList();
#endif
    void addAccessibilityStateChangeListener(const AccessibilityStateChangeListener& listener);

    void removeAccessibilityStateChangeListener(const AccessibilityStateChangeListener& listener);

    void addTouchExplorationStateChangeListener(const TouchExplorationStateChangeListener& listener);

    void removeTouchExplorationStateChangeListener(const TouchExplorationStateChangeListener& listener);
#if 0
    void addAccessibilityServicesStateChangeListener(const AccessibilityServicesStateChangeListener& listener);//,handler);

    void removeAccessibilityServicesStateChangeListener(const AccessibilityServicesStateChangeListener& listener);

    void addAccessibilityRequestPreparer(AccessibilityRequestPreparer preparer);

    void removeAccessibilityRequestPreparer(AccessibilityRequestPreparer preparer);

    std::vector<AccessibilityRequestPreparer> getRequestPreparersForAccessibilityId(int id);

    void addHighTextContrastStateChangeListener(const HighTextContrastChangeListener& listener, Handler handler);

    void removeHighTextContrastStateChangeListener(const HighTextContrastChangeListener& listener);

    void setAccessibilityPolicy(AccessibilityPolicy policy);

    bool isAccessibilityVolumeStreamActive();

    bool sendFingerprintGesture(int keyCode);

    AccessibilityServiceInfo getInstalledServiceInfoWithComponentName(const std::string&componentName);

    int addAccessibilityInteractionConnection(IWindow windowToken, const std::string& packageName, IAccessibilityInteractionConnection connection);

    void removeAccessibilityInteractionConnection(IWindow windowToken);

    void performAccessibilityShortcut();

    void notifyAccessibilityButtonClicked();

    void notifyAccessibilityButtonVisibilityChanged(bool shown);

    void setPictureInPictureActionReplacingConnection(IAccessibilityInteractionConnection connection);
#endif
    static bool isAccessibilityButtonSupported();
};
}/*endof namespace*/
#endif/*__ACCESSIBILITY_MANAGER_H__*/
