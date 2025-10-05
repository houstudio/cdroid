#include <view/accessibility/accessibilityevent.h>
#include <utils/mathutils.h>
#include <functional>
#include <sstream>

namespace cdroid{

Pools::SimplePool<AccessibilityEvent> AccessibilityEvent::sPool(MAX_POOL_SIZE);

AccessibilityEvent::AccessibilityEvent(){
    mEventType = 0;
    mEventTime = 0;
    mAction = 0;
    mMovementGranularity = 0;
    mContentChangeTypes = 0;
    mWindowChangeTypes =0;
}

void AccessibilityEvent::init(const AccessibilityEvent&event){
    mEventType = event.mEventType;
    mMovementGranularity = event.mMovementGranularity;
    mAction = event.mAction;
    mContentChangeTypes = event.mContentChangeTypes;
    mWindowChangeTypes = event.mWindowChangeTypes;
    mEventTime = event.mEventTime;
    mPackageName = event.mPackageName;
}

void AccessibilityEvent::setSealed(bool sealed){
}

size_t AccessibilityEvent::getRecordCount()const{
    return (int)mRecords.size();
}

void AccessibilityEvent::appendRecord(AccessibilityRecord*){
}

AccessibilityRecord* AccessibilityEvent::getRecord(int i){
    return mRecords.at(i);
}

int AccessibilityEvent::getEventType()const{
    return mEventType;
}

int AccessibilityEvent::getContentChangeTypes()const{
    return mContentChangeTypes;
}

static std::string flagsToString(int flags,std::function<std::string(int)>func){
    std::ostringstream builder;
    int count = 0;
    if(flags)builder<<"[";
    while (flags != 0) {
        const int flag = 1 << MathUtils::numberOfTrailingZeros(flags);
        flags &= ~flag;
        if (count > 0) builder<<", ";
        builder<<func(flag);
        count++;
    }
    if(count)builder<< "]";
    return builder.str();
}
std::string AccessibilityEvent::contentChangeTypesToString(int types) {
    return flagsToString(types,AccessibilityEvent::singleContentChangeTypeToString);
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
    return flagsToString(types, AccessibilityEvent::singleWindowChangeTypeToString);
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

std::string AccessibilityEvent::getPackageName()const{
    return mPackageName;
}

void AccessibilityEvent::setPackageName(const std::string&name){
    enforceNotSealed();
    mPackageName = name;
}

void AccessibilityEvent::setMovementGranularity(int granularity){
    enforceNotSealed();
    mMovementGranularity = granularity;
}

int AccessibilityEvent::getMovementGranularity()const{
    return mMovementGranularity;
}

void AccessibilityEvent::setAction(int action){
    enforceNotSealed();
    mAction = action;
}

int AccessibilityEvent::getAction()const{
    return mAction;
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
    return event;
}

void AccessibilityEvent::recycle() {
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

template<typename T>
static int numberOfTrailingZeros(T value) {
    if (value == 0) return sizeof(T)*8; // 32 位整数的情况下
    int count = 0;
    while ((value & 1) == 0) {
        value >>= 1;
        count++;
    }
    return count;
}

std::string AccessibilityEvent::toString(){
    std::ostringstream builder;
    builder<<"EventType: "<<eventTypeToString(mEventType);
    builder<<"; EventTime: "<<mEventTime;
    builder<<"; PackageName: "<<mPackageName;
    if (!DEBUG_CONCISE_TOSTRING || mMovementGranularity != 0) {
        builder<<"; MovementGranularity: "<<mMovementGranularity;
    }
    if (!DEBUG_CONCISE_TOSTRING || mAction != 0) {
        builder<<"; Action: "<<mAction;
    }
    if (!DEBUG_CONCISE_TOSTRING || mContentChangeTypes != 0) {
        builder<<"; ContentChangeTypes: "<<
                contentChangeTypesToString(mContentChangeTypes);
    }
    if (!DEBUG_CONCISE_TOSTRING || mWindowChangeTypes != 0) {
        builder<<"; WindowChangeTypes: "<<
                windowChangeTypesToString(mWindowChangeTypes);
    }
    AccessibilityRecord::appendTo(builder);
    if ( DEBUG_CONCISE_TOSTRING) {
        if (!DEBUG_CONCISE_TOSTRING) {
            builder<<"\n";
        }
        if (Debug) {
            builder<<"; SourceWindowId: "<<mSourceWindowId;
            builder<<"; SourceNodeId: "<<mSourceNodeId;
        }
        for (int i = 0; i < getRecordCount(); i++) {
            builder<<"  Record "<<i<<":";
            builder<<getRecord(i)->toString()<<"\n";
        }
    } else {
        builder<<"; recordCount: "<<getRecordCount();
    }
    return builder.str();
}

std::string AccessibilityEvent::eventTypeToString(int eventType){
    if (eventType == TYPES_ALL_MASK) {
        return "TYPES_ALL_MASK";
    }
    std::ostringstream builder;
    int eventTypeCount = 0;
    builder<<"[";
    while (eventType != 0) {
        const int eventTypeFlag = 1 << numberOfTrailingZeros(eventType);
        eventType &= ~eventTypeFlag;

        if (eventTypeCount > 0) {
            builder<<", ";
        }
        builder << singleEventTypeToString(eventTypeFlag);

        eventTypeCount++;
    }
    if (eventTypeCount) {
        builder<<']';
    }
    return builder.str();
}

std::string AccessibilityEvent::singleEventTypeToString(int eventType) {
    switch (eventType) {
    case TYPE_VIEW_CLICKED: return "TYPE_VIEW_CLICKED";
    case TYPE_VIEW_LONG_CLICKED: return "TYPE_VIEW_LONG_CLICKED";
    case TYPE_VIEW_SELECTED: return "TYPE_VIEW_SELECTED";
    case TYPE_VIEW_FOCUSED: return "TYPE_VIEW_FOCUSED";
    case TYPE_VIEW_TEXT_CHANGED: return "TYPE_VIEW_TEXT_CHANGED";
    case TYPE_WINDOW_STATE_CHANGED: return "TYPE_WINDOW_STATE_CHANGED";
    case TYPE_VIEW_HOVER_ENTER: return "TYPE_VIEW_HOVER_ENTER";
    case TYPE_VIEW_HOVER_EXIT: return "TYPE_VIEW_HOVER_EXIT";
    case TYPE_NOTIFICATION_STATE_CHANGED: return "TYPE_NOTIFICATION_STATE_CHANGED";
    case TYPE_TOUCH_EXPLORATION_GESTURE_START: return "TYPE_TOUCH_EXPLORATION_GESTURE_START";
    case TYPE_TOUCH_EXPLORATION_GESTURE_END: return "TYPE_TOUCH_EXPLORATION_GESTURE_END";
    case TYPE_WINDOW_CONTENT_CHANGED: return "TYPE_WINDOW_CONTENT_CHANGED";
    case TYPE_VIEW_TEXT_SELECTION_CHANGED: return "TYPE_VIEW_TEXT_SELECTION_CHANGED";
    case TYPE_VIEW_SCROLLED: return "TYPE_VIEW_SCROLLED";
    case TYPE_ANNOUNCEMENT: return "TYPE_ANNOUNCEMENT";
    case TYPE_VIEW_ACCESSIBILITY_FOCUSED: return "TYPE_VIEW_ACCESSIBILITY_FOCUSED";
    case TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED: return "TYPE_VIEW_ACCESSIBILITY_FOCUS_CLEARED";
    case TYPE_VIEW_TEXT_TRAVERSED_AT_MOVEMENT_GRANULARITY: return "TYPE_VIEW_TEXT_TRAVERSED_AT_MOVEMENT_GRANULARITY";
    case TYPE_GESTURE_DETECTION_START: return "TYPE_GESTURE_DETECTION_START";
    case TYPE_GESTURE_DETECTION_END: return "TYPE_GESTURE_DETECTION_END";
    case TYPE_TOUCH_INTERACTION_START: return "TYPE_TOUCH_INTERACTION_START";
    case TYPE_TOUCH_INTERACTION_END: return "TYPE_TOUCH_INTERACTION_END";
    case TYPE_WINDOWS_CHANGED: return "TYPE_WINDOWS_CHANGED";
    case TYPE_VIEW_CONTEXT_CLICKED: return "TYPE_VIEW_CONTEXT_CLICKED";
    case TYPE_ASSIST_READING_CONTEXT: return "TYPE_ASSIST_READING_CONTEXT";
    default: return std::to_string(eventType);
    }
}
}/*endof namespace*/
