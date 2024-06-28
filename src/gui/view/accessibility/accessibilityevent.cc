#include <view/accessibility/accessibilityevent.h>
namespace cdroid{

AccessibilityEvent::AccessibilityEvent(){
}

void AccessibilityEvent::init(AccessibilityEvent&){
#if 0
    mEventType = event.mEventType;
    mMovementGranularity = event.mMovementGranularity;
    mAction = event.mAction;
    mContentChangeTypes = event.mContentChangeTypes;
    mWindowChangeTypes = event.mWindowChangeTypes;
    mEventTime = event.mEventTime;
    mPackageName = event.mPackageName;
    //if (DEBUG_ORIGIN) originStackTrace = event.originStackTrace;
#endif
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
    return 0;
}
int AccessibilityEvent::getContentChangeTypes()const{
    return 0;
}
}
