/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <view/motionevent.h>
#include <private/inputeventlabels.h>
#include <core/inputdevice.h>
#include <utils/mathutils.h>
#include <utils/textutils.h>
#include <porting/cdlog.h>

namespace cdroid{

// --- PointerCoords ---
void PointerCoords::clear() {
    BitSet64::clear(bits);
    isResampled = false;
}

bool PointerCoords::isEmpty() const {
    return BitSet64::isEmpty(bits);
}

float PointerCoords::getAxisValue(int32_t axis) const {
    if (axis < 0 || axis > 63 || !BitSet64::hasBit(bits, axis)){
        return 0;
    }
    return values[BitSet64::getIndexOfBit(bits, axis)];
}

int PointerCoords::setAxisValue(int32_t axis, float value) {
    if (axis < 0 || axis > 63) {
        return -1;//NAME_NOT_FOUND;
    }

    const uint32_t index = BitSet64::getIndexOfBit(bits, axis);
    if (!BitSet64::hasBit(bits, axis)) {
        if (value == 0) {
            return 0;//OK; // axes with value 0 do not need to be stored
        }

        const uint32_t count = BitSet64::count(bits);
        if (count >= MAX_AXES) {
            tooManyAxes(axis);
            return -1;//NO_MEMORY;
        }
        BitSet64::markBit(bits, axis);
        for (uint32_t i = count; i > index; i--) {
            values[i] = values[i - 1];
        }
    }
    values[index] = value;
    return 0;//OK;
}

float PointerCoords::getX()const{
    return getAxisValue(MotionEvent::AXIS_X);
}

float PointerCoords::getY()const{
    return getAxisValue(MotionEvent::AXIS_Y);
}

std::array<float,2> PointerCoords::getXYValue()const{
    return{getX(),getY()};
}

static inline void scaleAxisValue(PointerCoords& c, int axis, float scaleFactor) {
    const float value = c.getAxisValue(axis);
    if (value != 0) {
        c.setAxisValue(axis, value * scaleFactor);
    }
}

float MotionEvent::getRawXOffset() const {
    // This is equivalent to the x-coordinate of the point that the origin of the raw coordinate
    // space maps to.
    return (mTransform * mRawTransform.inverse()).tx();
}

float MotionEvent::getRawYOffset() const {
    // This is equivalent to the y-coordinate of the point that the origin of the raw coordinate
    // space maps to.
    return (mTransform * mRawTransform.inverse()).ty();
}

void PointerCoords::scale(float globalScaleFactor,float windowXScale,float windowYScale) {
    // No need to scale pressure or size since they are normalized.
    // No need to scale orientation since it is meaningless to do so.
    scaleAxisValue(*this, MotionEvent::AXIS_X, windowXScale);
    scaleAxisValue(*this, MotionEvent::AXIS_Y, windowYScale);
    scaleAxisValue(*this, MotionEvent::AXIS_TOUCH_MAJOR, globalScaleFactor);
    scaleAxisValue(*this, MotionEvent::AXIS_TOUCH_MINOR, globalScaleFactor);
    scaleAxisValue(*this, MotionEvent::AXIS_TOOL_MAJOR, globalScaleFactor);
    scaleAxisValue(*this, MotionEvent::AXIS_TOOL_MINOR, globalScaleFactor);
    scaleAxisValue(*this, MotionEvent::AXIS_RELATIVE_X, windowXScale);
    scaleAxisValue(*this, MotionEvent::AXIS_RELATIVE_Y, windowYScale);
}

void PointerCoords::applyOffset(float xOffset, float yOffset) {
    setAxisValue(MotionEvent::AXIS_X, getX() + xOffset);
    setAxisValue(MotionEvent::AXIS_Y, getY() + yOffset);
}

void PointerCoords::tooManyAxes(int axis) {
    LOGW("Could not set value for axis %d because the PointerCoords structure is full and "
            "cannot contain more than %d axis values.", axis, int(MAX_AXES));
}

bool PointerCoords::operator==(const PointerCoords& other) const {
    if (bits != other.bits) {
        return false;
    }
    const uint32_t count = BitSet64::count(bits);
    for (uint32_t i = 0; i < count; i++) {
        if (values[i] != other.values[i]) {
            return false;
        }
    }
    if(isResampled != other.isResampled)return false;
    return true;
}

bool PointerCoords::operator!=(const PointerCoords& other) const {
     return !(*this == other);
}

void PointerCoords::copyFrom(const PointerCoords& other) {
    bits = other.bits;
    const uint32_t count = BitSet64::count(bits);
    for (uint32_t i = 0; i < count; i++) {
        values[i] = other.values[i];
    }
}

// --- PointerProperties ---
PointerProperties::PointerProperties(){
    clear();
}
void PointerProperties::clear() {
    id = -1;
    toolType = 0;
}

bool PointerProperties::operator==(const PointerProperties& other) const {
    return (id == other.id) && (toolType == other.toolType);
}

bool PointerProperties::operator!=(const PointerProperties& other) const {
    return !(*this == other);
}

void PointerProperties::copyFrom(const PointerProperties& other) {
    id = other.id;
    toolType = other.toolType;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
static std::vector<PointerProperties> gSharedTempPointerProperties;
static std::vector<PointerCoords> gSharedTempPointerCoords;
static std::vector<int> gSharedTempPointerIndexMap;

void MotionEvent::ensureSharedTempPointerCapacity(size_t desiredCapacity){
    if ( gSharedTempPointerCoords.size() < desiredCapacity) {
        int capacity = std::max((int)gSharedTempPointerCoords.size(),8);
        while (capacity < desiredCapacity) {
            capacity *= 2;
        }
        gSharedTempPointerCoords.resize(capacity);
        gSharedTempPointerProperties.resize(capacity);
        gSharedTempPointerIndexMap .resize(capacity);
    }
}

MotionEvent::MotionEvent(){
    mXPrecision =1;
    mYPrecision =1;
    mPointerProperties.clear();
    mSamplePointerCoords.clear();
    mSampleEventTimes.clear();
}

void MotionEvent::setSource(int32_t source){
    if(source==mSource)return;
    InputEvent::setSource(source);
    updateCursorPosition();
}

MotionEvent*MotionEvent::obtain(){
    MotionEvent*ev = PooledInputEventFactory::getInstance().createMotionEvent();
    ev->mTransform.reset();
    ev->mPointerProperties.clear();
    ev->mSamplePointerCoords.clear();
    ev->mSampleEventTimes.clear();
    return ev;
}

MotionEvent*MotionEvent::obtain(nsecs_t downTime , nsecs_t eventTime, int action,
        int pointerCount, const PointerProperties* pointerProperties,const PointerCoords* pointerCoords,
        int metaState, int buttonState,float xPrecision,float yPrecision,int deviceId,
        int edgeFlags,int source,int displayId,int flags,int classification){
    MotionEvent* ev = obtain();
    ev->initialize(deviceId, source,displayId, action,0/*actionbutton*/,
        flags, edgeFlags, metaState, buttonState, classification,
        0/*xoffset*/,0/*yoffset*/,xPrecision, yPrecision,
        INVALID_CURSOR_POSITION/*rawXCursorPosition*/,
        INVALID_CURSOR_POSITION/*rawYCursorPosition*/,
        downTime, eventTime,pointerCount, pointerProperties, pointerCoords);
    return ev;
}

MotionEvent* MotionEvent::obtain(nsecs_t downTime, nsecs_t eventTime, int action,
        int pointerCount,const PointerProperties* pointerProperties, const PointerCoords* pointerCoords,
        int metaState, int buttonState, float xPrecision, float yPrecision, int deviceId,
        int edgeFlags, int source, int displayId, int flags){
     return obtain(downTime, eventTime, action, pointerCount, pointerProperties, pointerCoords,
                metaState, buttonState, xPrecision, yPrecision, deviceId, edgeFlags, source,
                displayId, flags, CLASSIFICATION_NONE);
}

MotionEvent* MotionEvent::obtain(nsecs_t downTime, nsecs_t eventTime, int action,
        int pointerCount, const PointerProperties* pointerProperties, const PointerCoords* pointerCoords,
        int metaState, int buttonState, float xPrecision, float yPrecision, int deviceId,
        int edgeFlags, int source, int flags) {
    return obtain(downTime, eventTime, action, pointerCount, pointerProperties, pointerCoords,
            metaState, buttonState, xPrecision, yPrecision, deviceId, edgeFlags, source,
            0/*DEFAULT_DISPLAY*/, flags);
}

MotionEvent* MotionEvent::obtain(nsecs_t downTime,nsecs_t eventTime, int action,
            float x, float y, float pressure, float size, int metaState,
            float xPrecision, float yPrecision, int deviceId, int edgeFlags,
            int source,int displayId){
    MotionEvent* ev = obtain();
    ensureSharedTempPointerCapacity(1);
    PointerProperties pp;
    pp.clear();
    pp.id = 0;

    PointerCoords pc;
    pc.clear();
    pc.setAxisValue(AXIS_X,x);
    pc.setAxisValue(AXIS_Y,y);
    pc.setAxisValue(AXIS_PRESSURE,pressure);
    pc.setAxisValue(AXIS_SIZE,size);
    ev->initialize(deviceId, source, displayId, action, 0/*actionButton*/,
        0/*flags*/, edgeFlags, metaState,0/*buttonState*/,CLASSIFICATION_NONE,
        0/*xoffset*/,0/*yoffset*/,xPrecision, yPrecision,
        0/*rawXCursorPosition*/,0/*rawYCursorPosition*/,
        downTime , eventTime, 1, &pp,&pc);
    return ev;
}

MotionEvent* MotionEvent::obtain(nsecs_t downTime, nsecs_t eventTime, int action, float x, float y, int metaState){
    return obtain(downTime, eventTime, action, x, y, 1.f/*pressue*/, 1.f/*size*/,metaState,
            1.f/*xPrecision*/, 1.f/*yPrecision*/, 0/*deviceId*/, 0/*edgeFlags*/,0/*source*/,0/*displayid*/);
}

MotionEvent* MotionEvent::obtain(const MotionEvent& other) {
    MotionEvent* ev = obtain();
    ev->copyFrom(other,true);
    return ev;
}

MotionEvent* MotionEvent::obtainNoHistory(const MotionEvent& other){
    MotionEvent* ev = obtain();
    ev->copyFrom(other,false);
    return ev;
}

void MotionEvent::initialize(
        int deviceId,
        int source,
        int displayId,
        int action,
        int actionButton,
        int flags,
        int edgeFlags,
        int metaState,
        int buttonState,
        int classification,
        float xOffset,
        float yOffset,
        float xPrecision,
        float yPrecision,
        float rawXCursorPosition,
        float rawYCursorPosition,
        nsecs_t downTime,
        nsecs_t eventTime,
        size_t pointerCount,
        const PointerProperties* pointerProperties,
        const PointerCoords* pointerCoords) {
    InputEvent::initialize(0,deviceId, source,displayId);
    mAction = action;
    mActionButton = actionButton;
    mFlags = flags;
    mEdgeFlags = edgeFlags;
    mMetaState = metaState;
    mButtonState = buttonState;
    mClassification=classification;
    mXPrecision = xPrecision;
    mYPrecision = yPrecision;
    mRawXCursorPosition = rawXCursorPosition;
    mRawYCursorPosition = rawYCursorPosition;
    mDownTime = downTime;
    mEventTime= eventTime;
    mDisplayId = displayId;
    ui::Transform transform;
    transform.set(xOffset, yOffset);
    mTransform = transform;
    //mRawTransform=rawTransform;
    mPointerProperties.clear();

    mSampleEventTimes.clear();
    mSamplePointerCoords.clear();
    for(int i=0;i<pointerCount;i++){
        mPointerProperties.push_back(pointerProperties[i]);
    }
    addSample(eventTime,pointerCoords);
    updateCursorPosition();
}

MotionEvent::MotionEvent(const MotionEvent&other){
    copyFrom(other,false);
}

void MotionEvent::copyFrom(const MotionEvent& other, bool keepHistory) {
    InputEvent::initialize(nextId(),other.mDeviceId, other.mSource,other.mDisplayId);
    mAction = other.mAction;
    mActionButton = other.mActionButton;
    mFlags = other.mFlags;
    mEdgeFlags = other.mEdgeFlags;
    mMetaState = other.mMetaState;
    mButtonState = other.mButtonState;
    mClassification=other.mClassification;
    mXPrecision = other.mXPrecision;
    mYPrecision = other.mYPrecision;
    mDownTime = other.mDownTime;
    mEventTime= other.mEventTime;
    mTransform= other.mTransform;
    mPointerProperties = other.mPointerProperties;

    if (keepHistory) {
        mSampleEventTimes = other.mSampleEventTimes;
        mSamplePointerCoords = other.mSamplePointerCoords;
    } else {
        mSampleEventTimes.clear();
        mSampleEventTimes.push_back(other.getEventTime());
        mSamplePointerCoords.clear();
        const size_t pointerCount = other.getPointerCount();
        const size_t historySize = other.getHistorySize();

        mSamplePointerCoords.resize(pointerCount);
        for(int i=0;i<pointerCount;i++)
            mSamplePointerCoords.push_back(other.mSamplePointerCoords.at(historySize*pointerCount+i));
    }
}

MotionEvent*MotionEvent::split(int idBits){
    MotionEvent*ev = obtain();
    const size_t oldPointerCount = getPointerCount();
    ensureSharedTempPointerCapacity(oldPointerCount);
    PointerProperties *pp = gSharedTempPointerProperties.data();
    PointerCoords *pc = gSharedTempPointerCoords.data();
    int *map = gSharedTempPointerIndexMap.data();

    const int oldAction = getAction();
    const int oldActionMasked = oldAction & ACTION_MASK;
    const int oldActionPointerIndex = (oldAction & ACTION_POINTER_INDEX_MASK)>>ACTION_POINTER_INDEX_SHIFT;
    int newActionPointerIndex = -1;
    int newPointerCount = 0;
    for (size_t i = 0; i < oldPointerCount; i++) {
        getPointerProperties(i,pp[newPointerCount]);
        const int idBit = 1 << pp[newPointerCount].id;
        if ((idBit & idBits) != 0) {
            if (i == oldActionPointerIndex) {
                newActionPointerIndex = newPointerCount;
            }
            map[newPointerCount] = i;
            newPointerCount += 1;
        }
    }

    if (newPointerCount == 0) {
        LOGE("idBits did not match any ids in the event");
    }

    int newAction;
    if ( (oldActionMasked == ACTION_POINTER_DOWN) || (oldActionMasked == ACTION_POINTER_UP) ) {
        if (newActionPointerIndex < 0) { // An unrelated pointer changed.
            newAction = ACTION_MOVE;
        } else if (newPointerCount == 1) { // The first/last pointer went down/up.
            newAction = (oldActionMasked == ACTION_POINTER_DOWN) ? ACTION_DOWN : ACTION_UP;
        } else { // A secondary pointer went down/up.
            newAction = oldActionMasked | (newActionPointerIndex << ACTION_POINTER_INDEX_SHIFT);
        }
    } else { // Simple up/down/cancel/move or other motion action.
        newAction = oldAction;
    }

    const int historySize = (int)getHistorySize();
    for (int h = 0; h <= historySize; h++) {
        const int historyPos = h == historySize ? HISTORY_CURRENT : h;
        for (int i = 0; i < newPointerCount; i++) {
            //getPointerCoords(map[i], historyPos, &pc[i]);
            getHistoricalRawPointerCoords(map[i], historyPos,pc[i]);
        }
        const auto eventTimeNanos = getHistoricalEventTime(historyPos);
        if (h == 0) {
            ev->initialize( getDeviceId(),getSource(),getDisplayId(), newAction, 0,
                getFlags(), getEdgeFlags(), getMetaState() , getButtonState(), getClassification(),
                getRawXOffset(), getRawYOffset(),getXPrecision(), getYPrecision(),
                0/*rawXCursorPosition*/,0/*rawXCursorPosition*/,
                mDownTime, eventTimeNanos,  newPointerCount, pp, pc);
            ev->mId=InputEvent::nextId();
        } else {
            //nativeAddBatch(ev.mNativePtr, eventTimeNanos, pc, 0);
            ev->addSample(eventTimeNanos,pc);
        }
    }
    return ev;
}

bool MotionEvent::isButtonPressed(int button)const{
    return (button!=0) && ((getButtonState() & button) == button);
}

int MotionEvent::getPointerProperties(size_t pointerIndex,PointerProperties&pp) const{
    pp = mPointerProperties[pointerIndex];
    return 0;
}

void MotionEvent::addSample(nsecs_t eventTime, const PointerCoords*coords) {
    mSampleEventTimes.push_back(eventTime);
    mSamplePointerCoords.insert(mSamplePointerCoords.end(),
        &coords[0],&coords[getPointerCount()]);
}

const PointerCoords& MotionEvent::getRawPointerCoords(size_t pointerIndex) const {
    return mSamplePointerCoords[getHistorySize() * getPointerCount() + pointerIndex];
}

MotionEvent* MotionEvent::clampNoHistory(float left, float top, float right, float bottom){
    MotionEvent*ev = obtain();
    const size_t pointerCount = getPointerCount();
    ensureSharedTempPointerCapacity(pointerCount);

    PointerProperties* pp = gSharedTempPointerProperties.data();
    PointerCoords* pc = gSharedTempPointerCoords.data();
    for (size_t i = 0; i < pointerCount; i++) {
        pp[i] = mPointerProperties[i];//nativeGetPointerProperties(mNativePtr,i,pp[i]);
        getPointerCoords(i,pc[i]);//nativeGetPointerCoords(mNativePtr,i,HISTORY_CURRENT,pc[i]);
        pc[i].setAxisValue(AXIS_X,MathUtils::clamp(pc[i].getX(), left, right));
        pc[i].setAxisValue(AXIS_Y,MathUtils::clamp(pc[i].getY(), top, bottom));
    }
    /*ev->initialize(mDeviceId,mSource,mDisplayId,
            mAction,mActionButton,mFlags,
            mEdgeFlags,mMetaState,
            mButtonState,mClassification,
            mRawXCursorPosition,mRawYCursorPosition,
            mXPrecision,mYPrecision,
            mDownTime,getEventTime(),
            pointerCount,pp,pc);*/
    return ev;
}

int MotionEvent::getPointerIdBits()const{
    int idBits = 0;
    const int pointerCount = getPointerCount();
    for (int i = 0; i < pointerCount; i++) {
        idBits |= 1 << getPointerId(i);
    }
    return idBits;
}

float MotionEvent::getRawAxisValue(int32_t axis, size_t pointerIndex) const {
    return getHistoricalRawAxisValue(axis,pointerIndex,getHistorySize());
}

float MotionEvent::getAxisValue(int axis)const {
    return getAxisValue(axis, 0);
}

float MotionEvent::getAxisValue(int32_t axis, size_t pointerIndex) const {
    return getHistoricalAxisValue(axis,pointerIndex,getHistorySize());
}

nsecs_t MotionEvent::getHistoricalEventTime(size_t historyPos) const{
    if(historyPos==HISTORY_CURRENT){
         return getEventTime();
    }else{
        return mSampleEventTimes[historyPos];
    }
}

nsecs_t MotionEvent::getHistoricalEventTimeNanos(size_t historyPos) const{
    if(historyPos==HISTORY_CURRENT){
        return getEventTimeNanos();
    }else{
        return mSampleEventTimes[historyPos]*NS_PER_MS;
    }
}

int32_t MotionEvent::findPointerIndex(int32_t pointerId) const {
    const size_t pointerCount = mPointerProperties.size();
    for (size_t i = 0; i < pointerCount; i++) {
        if (mPointerProperties[i].id == pointerId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool  MotionEvent::isTouchEvent(uint32_t source, int32_t action){
    if (source & InputDevice::SOURCE_CLASS_POINTER) {
        // Specifically excludes HOVER_MOVE and SCROLL.
        switch (action & ACTION_MASK) {
        case ACTION_DOWN:
        case ACTION_MOVE:
        case ACTION_UP:
        case ACTION_POINTER_DOWN:
        case ACTION_POINTER_UP:
        case ACTION_CANCEL:
        case ACTION_OUTSIDE:
            return true;
        }
    }
    return false;
}

bool MotionEvent::isResampled(size_t pointerIndex, size_t historicalIndex) const {
    PointerCoords pc;
    getHistoricalRawPointerCoords(pointerIndex, historicalIndex,pc);
    return pc.isResampled;
}

bool MotionEvent::isTouchEvent()const{
    return isTouchEvent(mSource, mAction);
}

bool MotionEvent::isStylusPointer()const{
    const int actionIndex = getActionIndex();
    return isFromSource(InputDevice::SOURCE_STYLUS)
            && (getToolType(actionIndex) == TOOL_TYPE_STYLUS
            || getToolType(actionIndex) == TOOL_TYPE_ERASER);
}

bool MotionEvent::isHoverEvent()const{
    return getActionMasked() == ACTION_HOVER_ENTER
            || getActionMasked() == ACTION_HOVER_EXIT
            || getActionMasked() == ACTION_HOVER_MOVE;
}

void MotionEvent::offsetLocation(float xOffset, float yOffset) {
    const float currXOffset = mTransform.tx();
    const float currYOffset = mTransform.ty();
    mTransform.set(currXOffset + xOffset, currYOffset + yOffset);
}

void MotionEvent::setLocation(float x, float y){
    const float oldX = getX();
    const float oldY = getY();
    offsetLocation(x - oldX,y -oldY);
}

void MotionEvent::scale(float globalScaleFactor) {
    mTransform.set(mTransform.tx() * globalScaleFactor, mTransform.ty() * globalScaleFactor);
    mRawTransform.set(mRawTransform.tx() * globalScaleFactor,mRawTransform.ty() * globalScaleFactor);
    mXPrecision *= globalScaleFactor;
    mYPrecision *= globalScaleFactor;

    const size_t numSamples = mSamplePointerCoords.size();
    for (size_t i = 0; i < numSamples; i++) {
        mSamplePointerCoords[i].scale(globalScaleFactor, globalScaleFactor, globalScaleFactor);
    }
}

std::string MotionEvent::actionToString(int action){
    switch (action) {
    case ACTION_DOWN   :return "ACTION_DOWN";
    case ACTION_UP     :return "ACTION_UP";
    case ACTION_CANCEL :return "ACTION_CANCEL";
    case ACTION_OUTSIDE:return "ACTION_OUTSIDE";
    case ACTION_MOVE   :return "ACTION_MOVE";
    case ACTION_HOVER_MOVE: return "ACTION_HOVER_MOVE";
    case ACTION_SCROLL :    return "ACTION_SCROLL";
    case ACTION_HOVER_ENTER:return "ACTION_HOVER_ENTER";
    case ACTION_HOVER_EXIT :return "ACTION_HOVER_EXIT";
    case ACTION_BUTTON_PRESS  :return "ACTION_BUTTON_PRESS";
    case ACTION_BUTTON_RELEASE:return "ACTION_BUTTON_RELEASE";
    }
    const int index = (action & ACTION_POINTER_INDEX_MASK) >> ACTION_POINTER_INDEX_SHIFT;
    switch (action & ACTION_MASK) {
    case ACTION_POINTER_DOWN: return TextUtils::stringPrintf("ACTION_POINTER_DOWN(%d)",index);
    case ACTION_POINTER_UP  : return TextUtils::stringPrintf("ACTION_POINTER_UP(%d)",index);
    default: return std::to_string(action);
    }
}

std::string MotionEvent::axisToString(int axis){
    return InputEventLookup::getAxisLabel(axis);
}

static const char*BUTTON_SYMBOLIC_NAMES[]{
        "BUTTON_PRIMARY",
        "BUTTON_SECONDARY",
        "BUTTON_TERTIARY",
        "BUTTON_BACK",
        "BUTTON_FORWARD",
        "BUTTON_STYLUS_PRIMARY",
        "BUTTON_STYLUS_SECONDARY",
        "0x00000080",
        "0x00000100",
        "0x00000200",
        "0x00000400",
        "0x00000800",
        "0x00001000",
        "0x00002000",
        "0x00004000",
        "0x00008000",
        "0x00010000",
        "0x00020000",
        "0x00040000",
        "0x00080000",
        "0x00100000",
        "0x00200000",
        "0x00400000",
        "0x00800000",
        "0x01000000",
        "0x02000000",
        "0x04000000",
        "0x08000000",
        "0x10000000",
        "0x20000000",
        "0x40000000",
        "0x80000000",
};

std::string MotionEvent::buttonStateToString(int buttonState){
    if (buttonState == 0) {
        return "0";
    }
    std::ostringstream result;
    int i = 0,bitsSet=0;
    while (buttonState != 0) {
        const bool isSet = (buttonState & 1) != 0;
        buttonState >>= 1; // unsigned shift!
        if (isSet) {
            const char* name = BUTTON_SYMBOLIC_NAMES[i];
            if (bitsSet==0){
                if (buttonState == 0) {
                    return name;
                }
                result << name;
            } else {
                result<<'|'<<name;
            }
            bitsSet++;
        }
        i += 1;
    }
    return result.str();
}

std::string MotionEvent::toolTypeToString(int toolType){
    switch(toolType){
    case TOOL_TYPE_UNKNOWN: return "TOOL_TYPE_UNKNOWN";
    case TOOL_TYPE_FINGER : return "TOOL_TYPE_FINGER";
    case TOOL_TYPE_STYLUS : return "TOOL_TYPE_STYLUS";
    case TOOL_TYPE_MOUSE  : return "TOOL_TYPE_MOUSE";
    case TOOL_TYPE_ERASER : return "TOOL_TYPE_ERASER";
    default: return std::to_string(toolType);
    }
}

int MotionEvent::axisFromString(const std::string&symbolicName){
    if(symbolicName.compare(0,5,"AXIS_")==0){
        const int axis = InputEventLookup::getAxisByLabel(symbolicName.c_str()+5);
        if(axis>=0)return axis;
    }
    return -1;
}

std::string MotionEvent::classificationToString(int classification) {
    switch (classification) {
    default:
    case CLASSIFICATION_NONE:
        return "NONE";
    case CLASSIFICATION_AMBIGUOUS_GESTURE:
        return "AMBIGUOUS_GESTURE";
    case CLASSIFICATION_DEEP_PRESS:
        return "DEEP_PRESS";
    case CLASSIFICATION_TWO_FINGER_SWIPE:
        return "TWO_FINGER_SWIPE";
    case CLASSIFICATION_MULTI_FINGER_SWIPE:
        return "MULTI_FINGER_SWIPE";
    case CLASSIFICATION_PINCH:
        return "PINCH";
    }
}

void MotionEvent::transform(const std::array<float,9>& matrix){
    // We want to preserve the raw axes values stored in the PointerCoords, so we just update the
    // transform using the values passed in.
    ui::Transform newTransform;
    newTransform.set(matrix);
    mTransform = newTransform * mTransform;
}

void MotionEvent::applyTransform(const std::array<float,9>&matrix){
    ui::Transform transform;
    transform.set(matrix);

    // Apply the transformation to all samples.
    std::for_each(mSamplePointerCoords.begin(), mSamplePointerCoords.end(), [&](PointerCoords& c) {
        calculateTransformedCoordsInPlace(c, mSource, mFlags, transform);
    });

    if (mRawXCursorPosition != INVALID_CURSOR_POSITION &&
        mRawYCursorPosition != INVALID_CURSOR_POSITION) {
        const vec2 cursor = transform.transform(mRawXCursorPosition, mRawYCursorPosition);
        mRawXCursorPosition = cursor[0];
        mRawYCursorPosition = cursor[1];
    }
}

void MotionEvent::transform(const Cairo::Matrix& matrix){
    std::array<float,9> f9;
    /*Matrix{   double xx; double yx;  double xy; double yy;  double x0; double y0;}*/
    //x_new = xx * x + xy * y + x0; //y_new = yx * x + yy * y + y0;
    f9[0] = matrix.xx ; //scaleX
    f9[1] = matrix.xy ; //skewX
    f9[2] = matrix.x0 ; //skewY

    f9[3] = matrix.yx ;
    f9[4] = matrix.yy ;
    f9[5] = matrix.y0 ;

    f9[6] = f9[7] =0.0f;
    f9[8] = 1.f;
    transform(f9);
}

Cairo::Matrix MotionEvent::createRotateMatrix(/*@Surface.Rotation*/ int rotation, int rotatedFrameWidth, int rotatedFrameHeight) {
    if (rotation == 0/*Surface.ROTATION_0*/) {
        return Cairo::identity_matrix();//Matrix.IDENTITY_MATRIX);
    }
    // values is row-major
    std::array<float,9> values;
    if (rotation == 1/*Surface.ROTATION_90*/) {
        values = {0, 1, 0,
                -1, 0, (float)rotatedFrameHeight,
                0, 0, 1};
    } else if (rotation == 2/*Surface.ROTATION_180*/) {
        values = {-1, 0, (float)rotatedFrameWidth,
                0, -1, (float)rotatedFrameHeight,
                0, 0, 1};
    } else if (rotation == 3/*Surface.ROTATION_270*/) {
        values = {0, -1, (float)rotatedFrameWidth,
                1, 0, 0,
                0, 0, 1};
    }
    Cairo::Matrix toOrient;
    toOrient.xx = values[0];
    toOrient.xy = values[1];
    toOrient.x0 = values[2];
    toOrient.yx = values[3];
    toOrient.yy = values[4];
    toOrient.y0 = values[5];
    return toOrient;
}

int MotionEvent::getHistoricalRawPointerCoords( size_t pointerIndex, size_t historicalIndex,PointerCoords&pc) const {
    const size_t pointerCount = getPointerCount();
    if((pointerIndex<0)||(pointerIndex>=pointerCount)){
        LOGE("outof Range pointerIndex=%d/%d action=%d",pointerIndex,pointerCount,mAction);
        return -1;
    }
    if(historicalIndex==HISTORY_CURRENT){
        pc = mSamplePointerCoords[pointerIndex];
    }else{
        const size_t position = historicalIndex * getPointerCount() + pointerIndex;
        pc = mSamplePointerCoords[position];
    }
    return 0;
}

void MotionEvent::addBatch(nsecs_t eventTime, float x, float y, float pressure, float size, int metaState){
    ensureSharedTempPointerCapacity(1);
    PointerCoords* pc = gSharedTempPointerCoords.data();
    pc[0].clear();
    pc[0].setAxisValue(AXIS_X,x);//x = x;
    pc[0].setAxisValue(AXIS_Y,y);//y = y;
    pc[0].setAxisValue(AXIS_PRESSURE,pressure);//pressure = pressure;
    pc[0].setAxisValue(AXIS_SIZE,size);//size = size;
    //nativeAddBatch(mNativePtr, eventTime * NS_PER_MS, pc, metaState);
    addSample(eventTime * NS_PER_MS,pc);
    setMetaState(metaState|getMetaState());
}

void MotionEvent::addBatch(nsecs_t eventTime,const std::vector<PointerCoords>& pointerCoords, int metaState){
     addSample(eventTime,pointerCoords.data());
     setMetaState(metaState|getMetaState());
}

bool MotionEvent::addBatch(const MotionEvent& event){
    const int action = getActionMasked();//nativeGetAction(mNativePtr);
    if ((action != ACTION_MOVE) && (action != ACTION_HOVER_MOVE)) {
        return false;
    }
    if (action != event.getActionMasked()){//nativeGetAction(event.mNativePtr)) {
        return false;
    }

    if ((getDeviceId() != event.getDeviceId())
            || (getSource() != event.getSource())
            || (getFlags() != event.getFlags())) {
        return false;
    }

    const int pointerCount = getPointerCount();
    if (pointerCount != event.getPointerCount()) {
        return false;
    }

    /*synchronized (gSharedTempLock)*/{
        ensureSharedTempPointerCapacity(std::max(pointerCount, 2));
        PointerProperties* pp = gSharedTempPointerProperties.data();
        PointerCoords* pc = gSharedTempPointerCoords.data();

        for (int i = 0; i < pointerCount; i++) {
            getPointerProperties(i, pp[0]);
            event.getPointerProperties(i, pp[1]);
            if (pp[0]!=(pp[1])) {
                return false;
            }
        }

        const int metaState = event.getMetaState();
        const int historySize = event.getHistorySize();
        for (int h = 0; h <= historySize; h++) {
            const int historyPos = (h == historySize ? HISTORY_CURRENT : h);

            for (int i = 0; i < pointerCount; i++) {
                event.getHistoricalPointerCoords(i, historyPos, pc[i]);
            }

            const int64_t eventTimeNanos = event.getHistoricalEventTimeNanos(historyPos);
            //nativeAddBatch(mNativePtr, eventTimeNanos, pc, metaState);
            addSample(eventTimeNanos,pc);
            setMetaState(metaState|getMetaState());
        }
    }
    return true;
}

float MotionEvent::getHistoricalRawAxisValue(int32_t axis, size_t pointerIndex,
        size_t historicalIndex) const {
    PointerCoords pc;
    getHistoricalRawPointerCoords(pointerIndex,historicalIndex,pc);
    return pc.getAxisValue(axis);
}

float MotionEvent::getHistoricalRawX(size_t pointerIndex, size_t historicalIndex) const {
    return getHistoricalRawAxisValue(AXIS_X, pointerIndex, historicalIndex);
}

float MotionEvent::getHistoricalRawY(size_t pointerIndex, size_t historicalIndex) const {
    return getHistoricalRawAxisValue(AXIS_Y, pointerIndex, historicalIndex);
}

int MotionEvent::getPointerCoords(int pointerIndex,PointerCoords&pc)const{
    return getHistoricalRawPointerCoords(pointerIndex,HISTORY_CURRENT,pc);
}

int MotionEvent::getHistoricalPointerCoords(size_t pointerIndex, size_t historicalIndex,PointerCoords&pc) const{
    return getHistoricalRawPointerCoords(pointerIndex,historicalIndex,pc);
}

float MotionEvent::getHistoricalAxisValue(int axis,size_t pointerIndex,size_t historicalIndex)const{
    PointerCoords pc;
    getHistoricalRawPointerCoords(pointerIndex,historicalIndex,pc);
    return calculateTransformedAxisValue(axis, mSource, mFlags, mTransform, pc);
    //return pc.getAxisValue(axis);
}

float MotionEvent::getHistoricalX(size_t pointerIndex, size_t historicalIndex) const{
    return getHistoricalAxisValue(AXIS_X, pointerIndex, historicalIndex);
}

float MotionEvent::getHistoricalY(size_t pointerIndex, size_t historicalIndex) const{
    return getHistoricalAxisValue(AXIS_Y, pointerIndex, historicalIndex);
}

float MotionEvent::getHistoricalPressure(size_t pointerIndex, size_t historicalIndex) const{
    return getHistoricalAxisValue(AXIS_PRESSURE, pointerIndex, historicalIndex);
}

float MotionEvent::getHistoricalSize(size_t pointerIndex, size_t historicalIndex) const{
    return getHistoricalAxisValue(AXIS_SIZE, pointerIndex, historicalIndex);
}

float MotionEvent::getHistoricalTouchMajor(size_t pointerIndex, size_t historicalIndex) const{
    return getHistoricalAxisValue(AXIS_TOUCH_MAJOR, pointerIndex, historicalIndex);
}

float MotionEvent::getHistoricalTouchMinor(size_t pointerIndex, size_t historicalIndex) const{
    return getHistoricalAxisValue(AXIS_TOUCH_MINOR, pointerIndex, historicalIndex);
}

float MotionEvent::getHistoricalToolMajor(size_t pointerIndex, size_t historicalIndex) const{
    return getHistoricalAxisValue(AXIS_TOOL_MAJOR, pointerIndex, historicalIndex);
}

float MotionEvent::getHistoricalToolMinor(size_t pointerIndex, size_t historicalIndex) const{
    return getHistoricalAxisValue(AXIS_TOOL_MINOR, pointerIndex, historicalIndex);
}

float MotionEvent::getHistoricalOrientation(size_t pointerIndex, size_t historicalIndex) const{
    return getHistoricalAxisValue(AXIS_ORIENTATION, pointerIndex, historicalIndex);
}

void MotionEvent::cancel(){
    setAction(ACTION_CANCEL);
}

int MotionEvent::getSurfaceRotation() const{
    switch (mTransform.getOrientation()) {
        case ui::Transform::ROT_0:  return 0;//ui::ROTATION_0;
        case ui::Transform::ROT_90: return 3;//ui::ROTATION_270;
        case ui::Transform::ROT_180:return 2;//ui::ROTATION_180;
        case ui::Transform::ROT_270:return 1;//ui::ROTATION_90;
        default:  return -1;//std::nullopt;
    }
}

float MotionEvent::getXCursorPosition()const{
    vec2 vals = mTransform.transform(getRawXCursorPosition(), getRawYCursorPosition());
    //roundTransformedCoords(vals[0]);
    return vals[0];
}

float MotionEvent::getYCursorPosition()const{
    vec2 vals = mTransform.transform(getRawXCursorPosition(), getRawYCursorPosition());
    //roundTransformedCoords(vals[1]);
    return vals[1];
}

void MotionEvent::setCursorPosition(float x, float y){
    ui::Transform inverse = mTransform.inverse();
    vec2 vals = inverse.transform(x,y);
    //mCursorXPosition = vals[0];  mCursorYPosition = vals[1];
    mRawXCursorPosition = vals[0];
    mRawYCursorPosition = vals[1];
}

float MotionEvent::getXDispatchLocation(int pointerIndex){
    if (isFromSource(InputDevice::SOURCE_MOUSE)) {
        const float xCursorPosition = getXCursorPosition();
        if (xCursorPosition != INVALID_CURSOR_POSITION) {
            return xCursorPosition;
        }
    }
    return getX(pointerIndex);
}

float MotionEvent::getYDispatchLocation(int pointerIndex){
    if (isFromSource(InputDevice::SOURCE_MOUSE)) {
        const float yCursorPosition = getYCursorPosition();
        if (yCursorPosition != INVALID_CURSOR_POSITION) {
            return yCursorPosition;
        }
    }
    return getY(pointerIndex);
}

void MotionEvent::updateCursorPosition() {
    if (getSource() != InputDevice::SOURCE_MOUSE) {
        setCursorPosition(INVALID_CURSOR_POSITION, INVALID_CURSOR_POSITION);
        return;
    }

    float x = 0;
    float y = 0;

    const size_t pointerCount = getPointerCount();
    for (int i = 0; i < pointerCount; ++i) {
        x += getX(i);
        y += getY(i);
    }

    // If pointer count is 0, divisions below yield NaN, which is an acceptable result for this
    // corner case.
    x /= pointerCount;
    y /= pointerCount;
    setCursorPosition(x, y);
}

std::ostream& operator<<(std::ostream& out, const MotionEvent& event) {
    out << "MotionEvent { action=" << MotionEvent::actionToString(event.getAction());
    if (event.getActionButton() != 0) {
        out << ", actionButton=" << std::to_string(event.getActionButton());
    }
    const size_t pointerCount = event.getPointerCount();
    FATAL_IF(pointerCount > MotionEvent::MAX_POINTERS, "Too many pointers : pointerCount = %zu",
                        pointerCount);
    for (size_t i = 0; i < pointerCount; i++) {
        out << ", id[" << i << "]=" << event.getPointerId(i);
        float x = event.getX(i);
        float y = event.getY(i);
        if (x != 0 || y != 0) {
            out << ", x[" << i << "]=" << x;
            out << ", y[" << i << "]=" << y;
        }
        const int toolType = event.getToolType(i);
        if (toolType != MotionEvent::TOOL_TYPE_FINGER) {
            out << ", toolType[" << i << "]=" << MotionEvent::toolTypeToString(toolType);
        }
    }
    if (event.getButtonState() != 0) {
        out << ", buttonState=" << event.getButtonState();
    }
    if (event.getClassification() != MotionEvent::CLASSIFICATION_NONE) {
        out << ", classification=" << MotionEvent::classificationToString(event.getClassification());
    }
    if (event.getMetaState() != 0) {
        out << ", metaState=" << event.getMetaState();
    }
    if (event.getFlags() != 0) {
        out << ", flags=0x" << std::hex << event.getFlags() << std::dec;
    }
    if (event.getEdgeFlags() != 0) {
        out << ", edgeFlags=" << event.getEdgeFlags();
    }
    if (pointerCount != 1) {
        out << ", pointerCount=" << pointerCount;
    }
    if (event.getHistorySize() != 0) {
        out << ", historySize=" << event.getHistorySize();
    }
    out << ", eventTime=" << event.getEventTime();
    out << ", downTime=" << event.getDownTime();
    out << ", deviceId=" << event.getDeviceId();
    out << ", source=" << InputEvent::sourceToString(event.getSource());
    out << ", displayId=" << event.getDisplayId();
    out << ", eventId=0x" << std::hex << event.getId() << std::dec;
    out << "}";
    return out;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool isFromSource(uint32_t source, uint32_t test) {
    return (source & test) == test;
}

bool shouldDisregardTransformation(uint32_t source) {
    // Do not apply any transformations to axes from joysticks, touchpads, or relative mice.
    return isFromSource(source, InputDevice::SOURCE_CLASS_JOYSTICK) ||
            isFromSource(source, InputDevice::SOURCE_CLASS_POSITION) ||
            isFromSource(source, InputDevice::SOURCE_MOUSE_RELATIVE);
}

bool shouldDisregardOffset(uint32_t source) {
    // Pointer events are the only type of events that refer to absolute coordinates on the display,
    // so we should apply the entire window transform. For other types of events, we should make
    // sure to not apply the window translation/offset.
    return !isFromSource(source, InputDevice::SOURCE_CLASS_POINTER);
}

float roundTransformedCoords(float val) {
    // Use a power to two to approximate three decimal places to potentially reduce some cycles.
    // This should be at least as precise as MotionEvent::ROUNDING_PRECISION.
    return std::round(val * 1024.f) / 1024.f;
}

vec2 roundTransformedCoords(vec2 p) {
    return {roundTransformedCoords(p[0]), roundTransformedCoords(p[1])};
}

vec2 transformWithoutTranslation(const ui::Transform& transform, const vec2& xy) {
    const vec2 transformedXy = transform.transform(xy);
    const vec2 transformedOrigin = transform.transform(0, 0);
    vec2  v2 = {transformedXy[0]-transformedOrigin[0],transformedXy[1]-transformedOrigin[1]};
    return roundTransformedCoords(v2);//transformedXy - transformedOrigin);
}

// Apply the given transformation to the point without checking whether the entire transform
// should be disregarded altogether for the provided source.
static inline vec2 calculateTransformedXYUnchecked(uint32_t source,
        const ui::Transform& transform , const vec2& xy) {
    return shouldDisregardOffset(source) ? transformWithoutTranslation(transform, xy)
                                   : roundTransformedCoords(transform.transform(xy));
}

vec2 MotionEvent::calculateTransformedXY(uint32_t source,
        const ui::Transform& transform , const vec2& xy) {
    if (shouldDisregardTransformation(source)) {
        return xy;
    }
    return calculateTransformedXYUnchecked(source, transform, xy);
}

float transformAngle(const ui::Transform& transform, float angleRadians, bool isDirectional) {
    // Construct and transform a vector oriented at the specified clockwise angle from vertical.
    // Coordinate system: down is increasing Y, right is increasing X.
    const float x = sinf(angleRadians);
    const float y = -cosf(angleRadians);
    vec2 transformedPoint = transform.transform(x, y);

    // Determine how the origin is transformed by the matrix so that we
    // can transform orientation vectors.
    const vec2 origin = transform.transform(0, 0);

    transformedPoint[0] -= origin[0];//transformedPoint.x -= origin.x;
    transformedPoint[1] -= origin[1];//transformedPoint.y -= origin.y;

    if (!isDirectional && transformedPoint[1] > 0) {
        // Limit the range of atan2f to [-pi/2, pi/2] by reversing the direction of the vector.
        transformedPoint[0]=-transformedPoint[0];
        transformedPoint[1]=-transformedPoint[1];//transformedPoint *= -1;
    }

    // Derive the transformed vector's clockwise angle from vertical.
    // The return value of atan2f is in range [-pi, pi] which conforms to the orientation API.
    return atan2f(transformedPoint[0], -transformedPoint[1]);
}

float transformOrientation(const ui::Transform& transform,
        const PointerCoords& coords,int32_t motionEventFlags) {
    if ((motionEventFlags & MotionEvent::PRIVATE_FLAG_SUPPORTS_ORIENTATION) == 0) {
        return 0;
    }

    const bool isDirectionalAngle =
            (motionEventFlags & MotionEvent::PRIVATE_FLAG_SUPPORTS_DIRECTIONAL_ORIENTATION) != 0;

    return transformAngle(transform, coords.getAxisValue(MotionEvent::AXIS_ORIENTATION),
                          isDirectionalAngle);
}
// Keep in sync with calculateTransformedCoords.
float MotionEvent::calculateTransformedAxisValue(int32_t axis, uint32_t source,
        int32_t flags , const ui::Transform& transform, const PointerCoords& coords) {
    if (shouldDisregardTransformation(source)) {
        return coords.getAxisValue(axis);
    }

    if (axis == MotionEvent::AXIS_X || axis == MotionEvent::AXIS_Y) {
        const vec2 xy = calculateTransformedXYUnchecked(source, transform, coords.getXYValue());
        static_assert(MotionEvent::AXIS_X == 0 && MotionEvent::AXIS_Y == 1,"please check your AXIS_XXX definntions");
        return xy[axis];
    }

    if (axis == MotionEvent::AXIS_RELATIVE_X || axis == MotionEvent::AXIS_RELATIVE_Y) {
        const vec2 relativeXy = transformWithoutTranslation(transform,
                                    {coords.getAxisValue(MotionEvent::AXIS_RELATIVE_X),
                                    coords.getAxisValue(MotionEvent::AXIS_RELATIVE_Y)});
        return axis == MotionEvent::AXIS_RELATIVE_X ? relativeXy[0] : relativeXy[1];
    }

    if (axis == MotionEvent::AXIS_ORIENTATION) {
        return transformOrientation(transform, coords, flags);
    }

    return coords.getAxisValue(axis);
}

// Keep in sync with calculateTransformedAxisValue. This is an optimization of
// calculateTransformedAxisValue for all PointerCoords axes.
void MotionEvent::calculateTransformedCoordsInPlace(PointerCoords& coords,
        uint32_t source, int32_t flags, const ui::Transform& transform) {
    if (shouldDisregardTransformation(source)) {
        return;
    }

    const vec2 xy = calculateTransformedXYUnchecked(source, transform, coords.getXYValue());
    coords.setAxisValue(MotionEvent::AXIS_X, xy[0]);
    coords.setAxisValue(MotionEvent::AXIS_Y, xy[1]);

    const vec2 relativeXy = transformWithoutTranslation(transform,
                                {coords.getAxisValue(MotionEvent::AXIS_RELATIVE_X),
                                 coords.getAxisValue(MotionEvent::AXIS_RELATIVE_Y)});
    coords.setAxisValue(MotionEvent::AXIS_RELATIVE_X, relativeXy[0]);
    coords.setAxisValue(MotionEvent::AXIS_RELATIVE_Y, relativeXy[1]);

    coords.setAxisValue(MotionEvent::AXIS_ORIENTATION,
                        transformOrientation(transform, coords, flags));
}

PointerCoords MotionEvent::calculateTransformedCoords(uint32_t source, int32_t flags,
        const ui::Transform& transform, const PointerCoords& coords) {
    PointerCoords out = coords;
    calculateTransformedCoordsInPlace(out, source, flags, transform);
    return out;
}
}
