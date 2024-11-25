#include <view/accessibility/accessibilityevent.h>
namespace cdroid{

Pools::SimplePool<AccessibilityEvent> AccessibilityEvent::sPool(MAX_POOL_SIZE);

AccessibilityEvent::AccessibilityEvent(){
}

void AccessibilityEvent::init(const AccessibilityEvent&event){
    mEventType = event.mEventType;
    mMovementGranularity = event.mMovementGranularity;
    mAction = event.mAction;
    mContentChangeTypes = event.mContentChangeTypes;
    mWindowChangeTypes = event.mWindowChangeTypes;
    mEventTime = event.mEventTime;
    mPackageName = event.mPackageName;
    //if (DEBUG_ORIGIN) originStackTrace = event.originStackTrace;
}

void AccessibilityEvent::setSealed(bool sealed){
}

void AccessibilityEvent::getRecordCount()const{
}
/*void appendRecord(AccessibilityRecord&){
}
AccessibilityRecord getRecord(int){
}*/

int AccessibilityEvent::getEventType()const{
    return mEventType;
}

int AccessibilityEvent::getContentChangeTypes()const{
    return mContentChangeTypes;
}

std::string AccessibilityEvent::contentChangeTypesToString(int types) {
    return "";//BitUtils.flagsToString(types, AccessibilityEvent::singleContentChangeTypeToString);
}

std::string AccessibilityEvent::singleContentChangeTypeToString(int type) {
    switch (type) {
    case CONTENT_CHANGE_TYPE_CONTENT_DESCRIPTION: {
        return "CONTENT_CHANGE_TYPE_CONTENT_DESCRIPTION";
    }
    case CONTENT_CHANGE_TYPE_SUBTREE: return "CONTENT_CHANGE_TYPE_SUBTREE";
    case CONTENT_CHANGE_TYPE_TEXT: return "CONTENT_CHANGE_TYPE_TEXT";
    case CONTENT_CHANGE_TYPE_PANE_TITLE: return "CONTENT_CHANGE_TYPE_PANE_TITLE";
    case CONTENT_CHANGE_TYPE_UNDEFINED: return "CONTENT_CHANGE_TYPE_UNDEFINED";
    default: return std::to_string(type);
    }
}

void AccessibilityEvent::setContentChangeTypes(int changeTypes) {
    enforceNotSealed();
    mContentChangeTypes = changeTypes;
}

int AccessibilityEvent::getWindowChanges()const {
    return mWindowChangeTypes;
}

void AccessibilityEvent::setWindowChanges(int changes){
    mWindowChangeTypes = changes;
}

std::string AccessibilityEvent::windowChangeTypesToString(int types) {
    return "";//BitUtils.flagsToString(types, AccessibilityEvent::singleWindowChangeTypeToString);
}

std::string AccessibilityEvent::singleWindowChangeTypeToString(int type) {
    switch (type) {
    case WINDOWS_CHANGE_ADDED: return "WINDOWS_CHANGE_ADDED";
    case WINDOWS_CHANGE_REMOVED: return "WINDOWS_CHANGE_REMOVED";
    case WINDOWS_CHANGE_TITLE: return "WINDOWS_CHANGE_TITLE";
    case WINDOWS_CHANGE_BOUNDS: return "WINDOWS_CHANGE_BOUNDS";
    case WINDOWS_CHANGE_LAYER: return "WINDOWS_CHANGE_LAYER";
    case WINDOWS_CHANGE_ACTIVE: return "WINDOWS_CHANGE_ACTIVE";
    case WINDOWS_CHANGE_FOCUSED: return "WINDOWS_CHANGE_FOCUSED";
    case WINDOWS_CHANGE_ACCESSIBILITY_FOCUSED:
        return "WINDOWS_CHANGE_ACCESSIBILITY_FOCUSED";
    case WINDOWS_CHANGE_PARENT: return "WINDOWS_CHANGE_PARENT";
    case WINDOWS_CHANGE_CHILDREN: return "WINDOWS_CHANGE_CHILDREN";
    default: return std::to_string(type);
    }
}

void AccessibilityEvent::setEventType(int eventType) {
    enforceNotSealed();
    mEventType = eventType;
}

int64_t AccessibilityEvent::getEventTime()const {
    return mEventTime;
}

void AccessibilityEvent::setEventTime(int64_t eventTime) {
    enforceNotSealed();
    mEventTime = eventTime;
}

AccessibilityEvent*AccessibilityEvent::obtainWindowsChangedEvent(
        int windowId, int windowChangeTypes) {
    AccessibilityEvent* event = AccessibilityEvent::obtain(TYPE_WINDOWS_CHANGED);
    event->setWindowId(windowId);
    event->setWindowChanges(windowChangeTypes);
    event->setImportantForAccessibility(true);
    return event;
}

AccessibilityEvent* AccessibilityEvent::obtain(int eventType) {
    AccessibilityEvent* event = AccessibilityEvent::obtain();
    event->setEventType(eventType);
    LOGD("event %p type=%x/%d",event,eventType,eventType);
    return event;
}

AccessibilityEvent* AccessibilityEvent::obtain(const AccessibilityEvent& event) {
    AccessibilityEvent* eventClone = AccessibilityEvent::obtain();
    eventClone->init(event);

    if (!event.mRecords.empty()) {
        const int recordCount = event.mRecords.size();
        //eventClone.mRecords = new ArrayList<AccessibilityRecord>(recordCount);
        for (int i = 0; i < recordCount; i++) {
            AccessibilityRecord* record = event.mRecords.at(i);
            AccessibilityRecord* recordClone = AccessibilityRecord::obtain(*record);
            eventClone->mRecords.push_back(recordClone);
        }
    }

    return eventClone;
}

AccessibilityEvent*AccessibilityEvent::obtain() {
    AccessibilityEvent* event = sPool.acquire();
    if (event == nullptr) event = new AccessibilityEvent();
    LOGD("obtain %p",event);
    return event;
}

void AccessibilityEvent::recycle() {
    LOGD("release %p",this);
    clear();
    sPool.release(this);
}

void AccessibilityEvent::clear() {
    AccessibilityRecord::clear();
    mEventType = 0;
    mMovementGranularity = 0;
    mAction = 0;
    mContentChangeTypes = 0;
    mWindowChangeTypes = 0;
    mPackageName.clear();
    mEventTime = 0;
    while (!mRecords.empty()) {
        AccessibilityRecord* record = mRecords.back();
        record->recycle();
        mRecords.pop_back();
    }
}

}/*endof namespace*/
