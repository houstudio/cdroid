#include <core/looper.h>
#include <core/systemclock.h>
#include <porting/cdlog.h>
#include <view/accessibility/accessibilitymanager.h>
#include <view/accessibility/accessibilityevent.h>
namespace cdroid{
AccessibilityManager*AccessibilityManager::sInstance=nullptr;
AccessibilityManager& AccessibilityManager::getInstance(Context* context) {
    if (sInstance == nullptr) {
        sInstance = new AccessibilityManager(context,0/*userId*/);
    }
    return *sInstance;
}

AccessibilityManager::AccessibilityManager(Context* context/*, IAccessibilityManager service*/, int userId) {
    // Constructor can't be chained because we can't create an instance of an inner class
    // before calling another constructor.
    //mCallback = new MyCallback();
    //mHandler = new Handler(context.getMainLooper(), mCallback);
    mUserId = userId;
    mIsEnabled = false;
    mIsTouchExplorationEnabled = false;
    mIsHighTextContrastEnabled = false;
    mRelevantEventTypes = AccessibilityEvent::TYPES_ALL_MASK;
    //tryConnectToServiceLocked(service);
}
#if 0
AccessibilityManager::AccessibilityManager(Handler handler, IAccessibilityManager service, int userId) {
    mCallback = new MyCallback();
    mHandler = handler;
    mUserId = userId;
    synchronized (mLock) {
        tryConnectToServiceLocked(service);
    }
}

IAccessibilityManagerClient AccessibilityManager::getClient() {
    return mClient;
}

Handler.Callback AccessibilityManager::getCallback() {
    return mCallback;
}
#endif

bool AccessibilityManager::isEnabled() {
    return mIsEnabled/*|| (mAccessibilityPolicy && mAccessibilityPolicy.isEnabled(mIsEnabled))*/;
}

bool AccessibilityManager::isTouchExplorationEnabled() {
    /*IAccessibilityManager service = getServiceLocked();
    if (service == null) {
        return false;
    }*/
    return mIsTouchExplorationEnabled;
}

bool AccessibilityManager::isHighTextContrastEnabled() {
    /*IAccessibilityManager service = getServiceLocked();
    if (service == null) {
        return false;
    }*/
    return mIsHighTextContrastEnabled;
}

void AccessibilityManager::sendAccessibilityEvent(AccessibilityEvent& event) {
    int userId;
    AccessibilityEvent* dispatchedEvent = &event;
#if 0
    IAccessibilityManager service = getServiceLocked();
    if (service == null) {
        return;
    }
#endif
    event.setEventTime(SystemClock::uptimeMillis());
#if 0
    if (mAccessibilityPolicy != nullptr) {
        dispatchedEvent = mAccessibilityPolicy.onAccessibilityEvent(event, mIsEnabled, mRelevantEventTypes);
        if (dispatchedEvent == null) {
            return;
        }
    } else {
        dispatchedEvent = event;
    }
#endif
    if (!isEnabled()) {
        Looper* myLooper = Looper::myLooper();
        if (myLooper == Looper::getMainLooper()) {
            throw std::runtime_error("Accessibility off. Did you forget to check that?");
        } else {
            LOGE("AccessibilityEvent sent with accessibility disabled");
            return;
        }
    }

    if ((dispatchedEvent->getEventType() & mRelevantEventTypes) == 0) {
        LOGI_IF(Debug,"Not dispatching irrelevant event: %p that is not among ",
            dispatchedEvent, AccessibilityEvent::eventTypeToString(mRelevantEventTypes).c_str());
        return;
    }
#if 0
    userId = mUserId;

    long identityToken = Binder.clearCallingIdentity();
    try {
        service.sendAccessibilityEvent(dispatchedEvent, userId);
    } finally {
        Binder.restoreCallingIdentity(identityToken);
    }
    LOGI_IF(Debug,"send dispatchedEvent %p",dispatchedEvent);
#endif
    setStateLocked(0xFFFFFFFF);
    LOGD("%s",event.toString().c_str());
    if (&event != dispatchedEvent) {
        event.recycle();
    }
    dispatchedEvent->recycle();
}

#if 0
void AccessibilityManager::interrupt() {
    IAccessibilityManager service;
    int userId;
    synchronized (mLock) {
        service = getServiceLocked();
        if (service == null) {
            return;
        }
        if (!isEnabled()) {
            Looper myLooper = Looper.myLooper();
            if (myLooper == Looper.getMainLooper()) {
                throw std::runtime_error("Accessibility off. Did you forget to check that?");
            } else {
                // If we're not running on the thread with the main looper, it's possible for
                // the state of accessibility to change between checking isEnabled and
                // calling this method. So just log the error rather than throwing the
                // exception.
                LOGE("Interrupt called with accessibility disabled");
                return;
            }
        }
        userId = mUserId;
    }
    service.interrupt(userId);
    LOGI_IF(Debug,"Requested interrupt from all services");
}

List<ServiceInfo> AccessibilityManager::getAccessibilityServiceList() {
    List<AccessibilityServiceInfo> infos = getInstalledAccessibilityServiceList();
    List<ServiceInfo> services = new ArrayList<>();
    const int infoCount = infos.size();
    for (int i = 0; i < infoCount; i++) {
        AccessibilityServiceInfo info = infos.get(i);
        services.add(info.getResolveInfo().serviceInfo);
    }
    return Collections.unmodifiableList(services);
}

List<AccessibilityServiceInfo> AccessibilityManager::getInstalledAccessibilityServiceList() {
    IAccessibilityManager service;
    int userId;
    synchronized (mLock) {
        service = getServiceLocked();
        if (service == null) {
            return Collections.emptyList();
        }
        userId = mUserId;
    }

    List<AccessibilityServiceInfo> services = null;
    services = service.getInstalledAccessibilityServiceList(userId);
    LOGI_IF(Debug,"Installed AccessibilityServices " + services);
    if (mAccessibilityPolicy != null) {
        services = mAccessibilityPolicy.getInstalledAccessibilityServiceList(services);
    }
    if (services != null) {
        return Collections.unmodifiableList(services);
    } else {
        return Collections.emptyList();
    }
}

List<AccessibilityServiceInfo> AccessibilityManager::getEnabledAccessibilityServiceList(int feedbackTypeFlags) {
    IAccessibilityManager service;
    int userId;
    synchronized (mLock) {
        service = getServiceLocked();
        if (service == null) {
            return Collections.emptyList();
        }
        userId = mUserId;
    }

    List<AccessibilityServiceInfo> services = null;
    services = service.getEnabledAccessibilityServiceList(feedbackTypeFlags, userId);
    LogI_IF(Debug,"Installed AccessibilityServices " + services);
    if (mAccessibilityPolicy != null) {
        services = mAccessibilityPolicy.getEnabledAccessibilityServiceList(
                feedbackTypeFlags, services);
    }
    if (services != null) {
        return Collections.unmodifiableList(services);
    } else {
        return Collections.emptyList();
    }
}
#endif

void AccessibilityManager::addAccessibilityStateChangeListener(const AccessibilityStateChangeListener& listener) {
    mAccessibilityStateChangeListeners.push_back(listener);
}

void AccessibilityManager::removeAccessibilityStateChangeListener(const AccessibilityStateChangeListener& listener) {
    auto it = std::find(mAccessibilityStateChangeListeners.begin(),
            mAccessibilityStateChangeListeners.end(),listener);
    if(it!=mAccessibilityStateChangeListeners.end()){
        mAccessibilityStateChangeListeners.erase(it);
    }
}

void AccessibilityManager::addTouchExplorationStateChangeListener(const TouchExplorationStateChangeListener& listener) {
    mTouchExplorationStateChangeListeners.push_back(listener);
}

void AccessibilityManager::removeTouchExplorationStateChangeListener(const TouchExplorationStateChangeListener& listener) {
    auto it = std::find(mTouchExplorationStateChangeListeners.begin(),
            mTouchExplorationStateChangeListeners.end(),listener);
    if(it!=mTouchExplorationStateChangeListeners.end()){
        mTouchExplorationStateChangeListeners.erase(it);
    }
}
#if 0
void AccessibilityManager::addAccessibilityServicesStateChangeListener(const AccessibilityServicesStateChangeListener& listener) {
    mServicesStateChangeListeners.put_back(listener);
}

void AccessibilityManager::removeAccessibilityServicesStateChangeListener(const AccessibilityServicesStateChangeListener& listener) {
    auto it = std::find(mServicesStateChangeListeners.begin(),
            mServicesStateChangeListeners.end(),listener);
    if(it!=mServicesStateChangeListeners.end()){
        mServicesStateChangeListeners.erase(it);
    }
}

void AccessibilityManager::addAccessibilityRequestPreparer(AccessibilityRequestPreparer preparer) {
    if (mRequestPreparerLists == null) {
        mRequestPreparerLists = new SparseArray<>(1);
    }
    int id = preparer.getView().getAccessibilityViewId();
    List<AccessibilityRequestPreparer> requestPreparerList = mRequestPreparerLists.get(id);
    if (requestPreparerList == null) {
        requestPreparerList = new ArrayList<>(1);
        mRequestPreparerLists.put(id, requestPreparerList);
    }
    requestPreparerList.add(preparer);
}

void AccessibilityManager::removeAccessibilityRequestPreparer(AccessibilityRequestPreparer preparer) {
    if (mRequestPreparerLists == null) {
        return;
    }
    int viewId = preparer.getView().getAccessibilityViewId();
    List<AccessibilityRequestPreparer> requestPreparerList = mRequestPreparerLists.get(viewId);
    if (requestPreparerList != null) {
        requestPreparerList.remove(preparer);
        if (requestPreparerList.isEmpty()) {
            mRequestPreparerLists.remove(viewId);
        }
    }
}

List<AccessibilityRequestPreparer> AccessibilityManager::getRequestPreparersForAccessibilityId(int id) {
    if (mRequestPreparerLists == null) {
        return null;
    }
    return mRequestPreparerLists.get(id);
}

void AccessibilityManager::addHighTextContrastStateChangeListener(const HighTextContrastChangeListener& listener, Handler handler) {
    mHighTextContrastStateChangeListeners
            .put(listener, (handler == null) ? mHandler : handler);
}

void AccessibilityManager::removeHighTextContrastStateChangeListener(const HighTextContrastChangeListener& listener) {
    mHighTextContrastStateChangeListeners.remove(listener);
}

void AccessibilityManager::setAccessibilityPolicy(AccessibilityPolicy policy) {
    mAccessibilityPolicy = policy;
}

bool AccessibilityManager::isAccessibilityVolumeStreamActive() {
    List<AccessibilityServiceInfo> serviceInfos =
            getEnabledAccessibilityServiceList(AccessibilityServiceInfo.FEEDBACK_ALL_MASK);
    for (int i = 0; i < serviceInfos.size(); i++) {
        if ((serviceInfos.get(i).flags & FLAG_ENABLE_ACCESSIBILITY_VOLUME) != 0) {
            return true;
        }
    }
    return false;
}

bool AccessibilityManager::sendFingerprintGesture(int keyCode) {
    IAccessibilityManager service;
    service = getServiceLocked();
    if (service == null) {
        return false;
    }
    return service.sendFingerprintGesture(keyCode);
}
#endif

void AccessibilityManager::setStateLocked(int stateFlags) {
    const bool enabled = (stateFlags & STATE_FLAG_ACCESSIBILITY_ENABLED) != 0;
    const bool touchExplorationEnabled = (stateFlags & STATE_FLAG_TOUCH_EXPLORATION_ENABLED) != 0;
    const bool highTextContrastEnabled = (stateFlags & STATE_FLAG_HIGH_TEXT_CONTRAST_ENABLED) != 0;

    const bool wasEnabled = isEnabled();
    const bool wasTouchExplorationEnabled = mIsTouchExplorationEnabled;
    const bool wasHighTextContrastEnabled = mIsHighTextContrastEnabled;

    // Ensure listeners get current state from isZzzEnabled() calls.
    mIsEnabled = enabled;
    mIsTouchExplorationEnabled = touchExplorationEnabled;
    mIsHighTextContrastEnabled = highTextContrastEnabled;

    if (wasEnabled != isEnabled()||1) {
        notifyAccessibilityStateChanged();
    }

    if (wasTouchExplorationEnabled != touchExplorationEnabled||1) {
        notifyTouchExplorationStateChanged();
    }

    if (wasHighTextContrastEnabled != highTextContrastEnabled||1) {
        notifyHighTextContrastStateChanged();
    }
}

#if 0
AccessibilityServiceInfo AccessibilityManager::getInstalledServiceInfoWithComponentName(const std::string& componentName) {
    List<AccessibilityServiceInfo> installedServiceInfos =
            getInstalledAccessibilityServiceList();
    if ((installedServiceInfos == null) || (componentName == null)) {
        return null;
    }
    for (int i = 0; i < installedServiceInfos.size(); i++) {
        if (componentName.equals(installedServiceInfos.get(i).getComponentName())) {
            return installedServiceInfos.get(i);
        }
    }
    return null;
}

int AccessibilityManager::addAccessibilityInteractionConnection(IWindow windowToken,
        String packageName, IAccessibilityInteractionConnection connection) {
    IAccessibilityManager service;
    int userId;
    service = getServiceLocked();
    if (service == null) {
        return View.NO_ID;
    }
    userId = mUserId;
    return service.addAccessibilityInteractionConnection(windowToken, connection,
            packageName, userId);
    return View.NO_ID;
}

void AccessibilityManager::removeAccessibilityInteractionConnection(IWindow windowToken) {
    IAccessibilityManager service;
    service = getServiceLocked();
    if (service == null) {
        return;
    }
    service.removeAccessibilityInteractionConnection(windowToken);
}

void AccessibilityManager::performAccessibilityShortcut() {
    final IAccessibilityManager service;
    service = getServiceLocked();
    if (service == null) {
        return;
    }
    service.performAccessibilityShortcut();
}

void AccessibilityManager::notifyAccessibilityButtonClicked() {
    IAccessibilityManager service;
    service = getServiceLocked();
    if (service == null) {
        return;
    }
    service.notifyAccessibilityButtonClicked();
}

void AccessibilityManager::notifyAccessibilityButtonVisibilityChanged(bool shown) {
    final IAccessibilityManager service;
    synchronized (mLock) {
        service = getServiceLocked();
        if (service == null) {
            return;
        }
    }
    service.notifyAccessibilityButtonVisibilityChanged(shown);
}

void AccessibilityManager::setPictureInPictureActionReplacingConnection(
        IAccessibilityInteractionConnection connection) {
    IAccessibilityManager service;
    synchronized (mLock) {
        service = getServiceLocked();
        if (service == null) {
            return;
        }
    }
    service.setPictureInPictureActionReplacingConnection(connection);
}

IAccessibilityManager AccessibilityManager::getServiceLocked() {
    if (mService == null) {
        tryConnectToServiceLocked(null);
    }
    return mService;
}

void AccessibilityManager::tryConnectToServiceLocked(IAccessibilityManager service) {
    if (service == null) {
        IBinder iBinder = ServiceManager.getService(Context.ACCESSIBILITY_SERVICE);
        if (iBinder == null) {
            return;
        }
        service = IAccessibilityManager.Stub.asInterface(iBinder);
    }

    const long userStateAndRelevantEvents = service.addClient(mClient, mUserId);
    setStateLocked(IntPair.first(userStateAndRelevantEvents));
    mRelevantEventTypes = IntPair.second(userStateAndRelevantEvents);
    mService = service;
}
#endif

void AccessibilityManager::notifyAccessibilityStateChanged() {
    bool isEnabled;
    std::vector<AccessibilityStateChangeListener>& listeners = mAccessibilityStateChangeListeners;

    const int numListeners = listeners.size();
    LOGD("numListeners=%d",numListeners);
    for (int i = 0; i < numListeners; i++) {
        AccessibilityStateChangeListener listener = listeners.at(i);
        listener(isEnabled);
    }
}

void AccessibilityManager::notifyTouchExplorationStateChanged() {
    bool isTouchExplorationEnabled;
    std::vector<TouchExplorationStateChangeListener>& listeners =mTouchExplorationStateChangeListeners;
    isTouchExplorationEnabled = mIsTouchExplorationEnabled;

    const int numListeners = listeners.size();
    for (int i = 0; i < numListeners; i++) {
        TouchExplorationStateChangeListener listener = listeners.at(i);
        listener(isTouchExplorationEnabled);
    }
}

void AccessibilityManager::notifyHighTextContrastStateChanged() {
    bool isHighTextContrastEnabled;
    std::vector<HighTextContrastChangeListener>& listeners =mHighTextContrastStateChangeListeners;
    isHighTextContrastEnabled = mIsHighTextContrastEnabled;

    const int numListeners = listeners.size();
    for (int i = 0; i < numListeners; i++) {
        HighTextContrastChangeListener listener = listeners.at(i);
        listener(isHighTextContrastEnabled);
    }
}

bool AccessibilityManager::isAccessibilityButtonSupported() {
    //final Resources res = Resources.getSystem();
    return true;//res.getBoolean(com.android.internal.R.bool.config_showNavigationBar);
}

}
