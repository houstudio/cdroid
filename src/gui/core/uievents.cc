#include <uievents.h>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <sstream> 
#include <inputeventlabels.h>
#include <inputdevice.h>
#include <cdtypes.h>
#include <cdinput.h>
#include <cdlog.h>
#include <math.h>

namespace cdroid{
// --- PointerCoords ---

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

    uint32_t index = BitSet64::getIndexOfBit(bits, axis);
    if (!BitSet64::hasBit(bits, axis)) {
        if (value == 0) {
            return 0;//OK; // axes with value 0 do not need to be stored
        }

        uint32_t count = BitSet64::count(bits);
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

static inline void scaleAxisValue(PointerCoords& c, int axis, float scaleFactor) {
    float value = c.getAxisValue(axis);
    if (value != 0) {
        c.setAxisValue(axis, value * scaleFactor);
    }
}

void PointerCoords::scale(float scaleFactor) {
    // No need to scale pressure or size since they are normalized.
    // No need to scale orientation since it is meaningless to do so.
    scaleAxisValue(*this, MotionEvent::AXIS_X, scaleFactor);
    scaleAxisValue(*this, MotionEvent::AXIS_Y, scaleFactor);
    scaleAxisValue(*this, MotionEvent::AXIS_TOUCH_MAJOR, scaleFactor);
    scaleAxisValue(*this, MotionEvent::AXIS_TOUCH_MINOR, scaleFactor);
    scaleAxisValue(*this, MotionEvent::AXIS_TOOL_MAJOR, scaleFactor);
    scaleAxisValue(*this, MotionEvent::AXIS_TOOL_MINOR, scaleFactor);
}

void PointerCoords::applyOffset(float xOffset, float yOffset) {
    setAxisValue(MotionEvent::AXIS_X, getX() + xOffset);
    setAxisValue(MotionEvent::AXIS_Y, getY() + yOffset);
}

void PointerCoords::tooManyAxes(int axis) {
    LOGD("Could not set value for axis %d because the PointerCoords structure is full and "
            "cannot contain more than %d axis values.", axis, int(MAX_AXES));
}

bool PointerCoords::operator==(const PointerCoords& other) const {
    if (bits != other.bits) {
        return false;
    }
    uint32_t count = BitSet64::count(bits);
    for (uint32_t i = 0; i < count; i++) {
        if (values[i] != other.values[i]) {
            return false;
        }
    }
    return true;
}

void PointerCoords::copyFrom(const PointerCoords& other) {
    bits = other.bits;
    uint32_t count = BitSet64::count(bits);
    for (uint32_t i = 0; i < count; i++) {
        values[i] = other.values[i];
    }
}
// --- PointerProperties ---

bool PointerProperties::operator==(const PointerProperties& other) const {
    return id == other.id
            && toolType == other.toolType;
}

void PointerProperties::copyFrom(const PointerProperties& other) {
    id = other.id;
    toolType = other.toolType;
}

// --- InputEvent ---

InputEvent::InputEvent(){
   mSource = InputDevice::SOURCE_UNKNOWN;
}

InputEvent::~InputEvent(){
}

void InputEvent::initialize(int32_t deviceId, int32_t source) {
    mDeviceId = deviceId;
    mSource = source;
}

bool InputEvent::isFromSource(int source)const{
    return (getSource() & source) == source;
}

void InputEvent::initialize(const InputEvent& from) {
    mDeviceId = from.mDeviceId;
    mSource = from.mSource;
}

void InputEvent::recycle(){
    PooledInputEventFactory::getInstance().recycle(this);
}
/////////////KeyEvent///////////////

KeyEvent* KeyEvent::obtain(){
    return PooledInputEventFactory::getInstance().createKeyEvent();
}

KeyEvent* KeyEvent::obtain(nsecs_t downTime, nsecs_t eventTime, int action,int code, int repeat, int metaState,
              int deviceId, int scancode, int flags, int source/*, std::string characters*/){
    KeyEvent* ev = obtain();
    ev->mDownTime = downTime;
    ev->mEventTime = eventTime;
    ev->mAction = action;
    ev->mKeyCode = code;
    ev->mRepeatCount = repeat;
    ev->mMetaState = metaState;
    ev->mDeviceId = deviceId;
    ev->mScanCode = scancode;
    ev->mFlags = flags;
    ev->mSource = source;
    //ev->mCharacters = characters;
    return ev;
}

KeyEvent* KeyEvent::obtain(const KeyEvent& other){
    KeyEvent* ev = obtain();
    ev->mDownTime = other.mDownTime;
    ev->mEventTime = other.mEventTime;
    ev->mAction = other.mAction;
    ev->mKeyCode = other.mKeyCode;
    ev->mRepeatCount = other.mRepeatCount;
    ev->mMetaState = other.mMetaState;
    ev->mDeviceId = other.mDeviceId;
    ev->mScanCode = other.mScanCode;
    ev->mFlags = other.mFlags;
    ev->mSource = other.mSource;
    //ev->mCharacters = other.mCharacters;
    return ev;    
}

bool KeyEvent::dispatch(KeyEvent::Callback* receiver,KeyEvent::DispatcherState*state,void*target){
    bool res;
    switch (mAction) {
    case ACTION_DOWN:
        mFlags &= ~FLAG_START_TRACKING;
        res = receiver->onKeyDown(mKeyCode, *this);
        if (state != nullptr) {
            if (res && mRepeatCount == 0 && (mFlags&FLAG_START_TRACKING) != 0) {
                LOGV("  Start tracking!");
                state->startTracking(*this, target);
            } else if (isLongPress() && state->isTracking(*this)) {
                if (receiver->onKeyLongPress(mKeyCode, *this)) {
                    LOGV("  Clear from long press!");
                    state->performedLongPress(*this);
                    res = true;
                }
            }
        }
        return res;
    case ACTION_UP:
        if (state )  state->handleUpEvent(*this);
        res=receiver->onKeyUp(mKeyCode, *this);
	break;
    case ACTION_MULTIPLE:
        if (receiver->onKeyMultiple(mKeyCode, mRepeatCount, *this)) {
            return true;
        }
        if (mKeyCode != KEY_UNKNOWN) {
	    int count=mRepeatCount;
            mAction = ACTION_DOWN;
            mRepeatCount = 0;
            bool handled = receiver->onKeyDown(mKeyCode, *this);
            if (handled) {
                mAction = ACTION_UP;
                receiver->onKeyUp(mKeyCode, *this);
            }
            mAction = ACTION_MULTIPLE;
            mRepeatCount = count;
            return handled;
        }
        return false;
    }
    LOGV("%p %s.%s res=%d",receiver,getLabel(mKeyCode),actionToString(mAction).c_str(),res);
    return res;
}

const char* KeyEvent::getLabel(int keyCode) {
    return getLabelByKeyCode(keyCode);
}

int32_t KeyEvent::getKeyCodeFromLabel(const char* label) {
    return getKeyCodeByLabel(label);
}

bool KeyEvent::isModifierKey(int keyCode){
    switch (keyCode) {
    case KEY_SHIFT_LEFT:
    case KEY_SHIFT_RIGHT:
    case KEY_ALT_LEFT:
    case KEY_ALT_RIGHT:
    case KEY_CTRL_LEFT:
    case KEY_CTRL_RIGHT:
    case KEY_META_LEFT:
    case KEY_META_RIGHT:
    case KEY_SYM:
    case KEY_NUM:
    case KEY_FUNCTION:return true;
    default:return false;
    }
}

bool KeyEvent::isConfirmKey(int keyCode){
    switch(keyCode){
    case KEY_ENTER:
    case KEY_DPAD_CENTER:
    case KEY_NUMPAD_ENTER:
        return true;
    default:return false;
    }
}

void KeyEvent::initialize(int32_t deviceId,   int32_t source,
        int32_t action,  int32_t flags,  int32_t keyCode, int32_t scanCode,
        int32_t metaState, int32_t repeatCount, nsecs_t downTime, nsecs_t eventTime) {
    InputEvent::initialize(deviceId, source);
    mAction = action;
    mFlags = flags;
    mKeyCode = keyCode;
    mScanCode = scanCode;
    mMetaState = metaState;
    mRepeatCount = repeatCount;
    mDownTime = downTime;
    mEventTime = eventTime;
}

void KeyEvent::initialize(const KeyEvent& from) {
    InputEvent::initialize(from);
    mAction = from.mAction;
    mFlags = from.mFlags;
    mKeyCode = from.mKeyCode;
    mScanCode = from.mScanCode;
    mMetaState = from.mMetaState;
    mRepeatCount = from.mRepeatCount;
    mDownTime = from.mDownTime;
    mEventTime = from.mEventTime;
}

static const char*META_SYMBOLIC_NAMES[]={
    "META_SHIFT_ON",
    "META_ALT_ON",
    "META_SYM_ON",
    "META_FUNCTION_ON",
    "META_ALT_LEFT_ON",
    "META_ALT_RIGHT_ON",
    "META_SHIFT_LEFT_ON",
    "META_SHIFT_RIGHT_ON",
    "META_CAP_LOCKED",
    "META_ALT_LOCKED",
    "META_SYM_LOCKED",
    "0x00000800",
    "META_CTRL_ON",
    "META_CTRL_LEFT_ON",
    "META_CTRL_RIGHT_ON",
    "0x00008000",
    "META_META_ON",
    "META_META_LEFT_ON",
    "META_META_RIGHT_ON",
    "0x00080000",
    "META_CAPS_LOCK_ON",
    "META_NUM_LOCK_ON",
    "META_SCROLL_LOCK_ON",
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

const std::string KeyEvent::metaStateToString(int metaState){
    std::string result;
    int i=0;
    while (metaState != 0) {
        bool isSet = (metaState & 1) != 0;
        metaState >>= 1; // unsigned shift!
        if (isSet) {
            std::string name = META_SYMBOLIC_NAMES[i];
            if (result.empty()) {
                if (metaState == 0) {
                    return name;
                }
                result = name;
            } else {
                result+="!";
                result+=name;
            }
        }
        i += 1;
    }
    return result;
}

const std::string KeyEvent::actionToString(int action){
    switch (action) {
    case ACTION_DOWN:
        return "ACTION_DOWN";
    case ACTION_UP:
        return "ACTION_UP";
    case ACTION_MULTIPLE:
        return "ACTION_MULTIPLE";
    default:return std::to_string(action);
    }
}

bool KeyEvent::metaStateHasNoModifiers(int metaState) {
    return (normalizeMetaState(metaState) & META_MODIFIER_MASK) == 0;
}

bool KeyEvent::hasNoModifiers()const{
    return metaStateHasNoModifiers(mMetaState);
}

bool KeyEvent::hasModifiers(int modifiers)const{
    return metaStateHasModifiers(mMetaState, modifiers);
}

int KeyEvent::metaStateFilterDirectionalModifiers(int metaState,
        int modifiers, int basic, int left, int right) {
    bool wantBasic = (modifiers & basic) != 0;
    int directional = left | right;
    bool wantLeftOrRight = (modifiers & directional) != 0;

    if (wantBasic) {
        if (wantLeftOrRight) {
            LOGE("modifiers must not contain %s combined with %s or %s",metaStateToString(basic).c_str(),
                    metaStateToString(left).c_str(),metaStateToString(right).c_str());
        }
        return metaState & ~directional;
    } else if (wantLeftOrRight) {
        return metaState & ~basic;
    } else {
        return metaState;
    }
}

bool KeyEvent::metaStateHasModifiers(int metaState, int modifiers){
    if ((modifiers & META_INVALID_MODIFIER_MASK) != 0) {
        LOGE("modifiers must not contain META_CAPS_LOCK_ON, META_NUM_LOCK_ON, META_SCROLL_LOCK_ON, "
                    "META_CAP_LOCKED, META_ALT_LOCKED, META_SYM_LOCKED,or META_SELECTING");
    }

    metaState = normalizeMetaState(metaState) & META_MODIFIER_MASK;
    metaState = metaStateFilterDirectionalModifiers(metaState, modifiers,
                META_SHIFT_ON, META_SHIFT_LEFT_ON, META_SHIFT_RIGHT_ON);
    metaState = metaStateFilterDirectionalModifiers(metaState, modifiers,
                META_ALT_ON, META_ALT_LEFT_ON, META_ALT_RIGHT_ON);
    metaState = metaStateFilterDirectionalModifiers(metaState, modifiers,
                META_CTRL_ON, META_CTRL_LEFT_ON, META_CTRL_RIGHT_ON);
    metaState = metaStateFilterDirectionalModifiers(metaState, modifiers,
                META_META_ON, META_META_LEFT_ON, META_META_RIGHT_ON);
    return metaState == modifiers;
}

int KeyEvent::normalizeMetaState(int metaState){
    if ((metaState & (META_SHIFT_LEFT_ON | META_SHIFT_RIGHT_ON)) != 0) {
         metaState |= META_SHIFT_ON;
    }
    if ((metaState & (META_ALT_LEFT_ON | META_ALT_RIGHT_ON)) != 0) {
        metaState |= META_ALT_ON;
    }
    if ((metaState & (META_CTRL_LEFT_ON | META_CTRL_RIGHT_ON)) != 0) {
        metaState |= META_CTRL_ON;
    }
    if ((metaState & (META_META_LEFT_ON | META_META_RIGHT_ON)) != 0) {
        metaState |= META_META_ON;
    }
    if ((metaState & META_CAP_LOCKED) != 0) {
        metaState |= META_CAPS_LOCK_ON;
    }
    if ((metaState & META_ALT_LOCKED) != 0) {
        metaState |= META_ALT_ON;
    }
    if ((metaState & META_SYM_LOCKED) != 0) {
        metaState |= META_SYM_ON;
    }
    return metaState & META_ALL_MASK;
}

KeyEvent::DispatcherState::DispatcherState(){
    reset();
}

void KeyEvent::DispatcherState::reset(){
    LOGV("Reset %p",this);
    mDownKeyCode = 0;
    mDownTarget = nullptr;
    mActiveLongPresses.clear();
}

void KeyEvent::DispatcherState::reset(void* target){
    if(mDownTarget==target){
        LOGV("Reset in %p:%p",target,this);
        mDownKeyCode=0;
        mDownTarget=nullptr;
    }
}

void KeyEvent::DispatcherState::startTracking(KeyEvent& event,void* target){
    if (event.getAction() != ACTION_DOWN) {
         throw "Can only start tracking on a down event";
    }
    LOGV("Start trackingt in %p:%p",target,this);
    mDownKeyCode = event.getKeyCode();
    mDownTarget = target;  
}

bool KeyEvent::DispatcherState::isTracking(KeyEvent& event){
    return mDownKeyCode == event.getKeyCode();
}

void KeyEvent::DispatcherState::performedLongPress(KeyEvent& event){
    mActiveLongPresses.put(event.getKeyCode(), 1);
}

void KeyEvent::DispatcherState::handleUpEvent(KeyEvent& event){
    int keyCode = event.getKeyCode();
    //LOGV("Handle key up %s:%p",event,this);
    int index = mActiveLongPresses.indexOfKey(keyCode);
    if (index >= 0) {
        LOGV("  Index: %d",index);
        event.mFlags |= FLAG_CANCELED | FLAG_CANCELED_LONG_PRESS;
        mActiveLongPresses.removeAt(index);
    }
    if (mDownKeyCode == keyCode) {
        LOGV("  Tracking!");
        event.mFlags |= FLAG_TRACKING;
        mDownKeyCode = 0;
        mDownTarget = nullptr;
    }
   
}

//-------------------MotionEvent------------
MotionEvent::MotionEvent(){
    mPointerProperties.clear();
    mSamplePointerCoords.clear();
    mSampleEventTimes.clear(); 
}

MotionEvent*MotionEvent::obtain(){
    return PooledInputEventFactory::getInstance().createMotionEvent();
}

MotionEvent*MotionEvent::obtain(nsecs_t downTime, nsecs_t eventTime,
            int action, int pointerCount, const PointerProperties* pointerProperties,
            const PointerCoords* pointerCoords, int metaState, int buttonState,
            float xPrecision, float yPrecision, int deviceId,
            int edgeFlags, int source, int flags){
    MotionEvent* ev = obtain();
    ev->initialize(deviceId, source, action,0/*actionbutton*/, flags, edgeFlags, metaState, buttonState,
                0, 0, xPrecision, yPrecision, downTime, eventTime,
                pointerCount, pointerProperties, pointerCoords);
    return ev;
}

MotionEvent* MotionEvent::obtain(nsecs_t downTime,nsecs_t eventTime, int action,
            float x, float y, float pressure, float size, int metaState,
            float xPrecision, float yPrecision, int deviceId, int edgeFlags){
    MotionEvent* ev = obtain();
    PointerProperties pp;
    pp.clear();
    pp.id = 0;

    PointerCoords pc;
    pc.clear();
    pc.setAxisValue(AXIS_X,x);
    pc.setAxisValue(AXIS_Y,y);
    pc.setAxisValue(AXIS_PRESSURE,pressure);
    pc.setAxisValue(AXIS_SIZE,size);
    ev->initialize(deviceId, InputDevice::SOURCE_UNKNOWN, action, 0/*actionButton*/,0/*flags*/, edgeFlags, metaState, 0,
            0, 0, xPrecision, yPrecision, downTime , eventTime, 1, &pp,&pc);
    return ev;
}

MotionEvent* MotionEvent::obtain(nsecs_t downTime, nsecs_t eventTime, int action, float x, float y, int metaState){
    return obtain(downTime, eventTime, action, x, y, 1.0f, 1.0f,
                metaState, 1.0f, 1.0f, 0, 0);
}

MotionEvent* MotionEvent::obtain(const MotionEvent& other) {
    MotionEvent* ev = obtain();
    ev->copyFrom(&other,true);
    return ev;
}

MotionEvent* MotionEvent::obtainNoHistory(MotionEvent& other){
    MotionEvent* ev = obtain();
    ev->copyFrom(&other,false);
    return ev;
}

void MotionEvent::initialize(
        int deviceId,
        int source,
        int action,
        int actionButton,
        int flags,
        int edgeFlags,
        int metaState,
        int buttonState,
        float xOffset,
        float yOffset,
        float xPrecision,
        float yPrecision,
        nsecs_t downTime,
        nsecs_t eventTime,
        size_t pointerCount,
        const PointerProperties* pointerProperties,
        const PointerCoords* pointerCoords) {
    InputEvent::initialize(deviceId, source);
    mAction = action;
    mActionButton = actionButton;
    mFlags = flags;
    mEdgeFlags = edgeFlags;
    mMetaState = metaState;
    mButtonState = buttonState;
    mXOffset = xOffset;
    mYOffset = yOffset;
    mXPrecision = xPrecision;
    mYPrecision = yPrecision;
    mDownTime = downTime;
	mEventTime= eventTime;
    mPointerProperties.clear();

    mSampleEventTimes.clear();
    mSamplePointerCoords.clear();
    for(int i=0;i<pointerCount;i++)
       addSample(eventTime,pointerProperties[i], pointerCoords[i]);
}

MotionEvent::MotionEvent(const MotionEvent&other){
    copyFrom(&other,false);
}

void MotionEvent::copyFrom(const MotionEvent* other, bool keepHistory) {
    InputEvent::initialize(other->mDeviceId, other->mSource);
    mAction = other->mAction;
    mActionButton = other->mActionButton;
    mFlags = other->mFlags;
    mEdgeFlags = other->mEdgeFlags;
    mMetaState = other->mMetaState;
    mButtonState = other->mButtonState;
    mXOffset = other->mXOffset;
    mYOffset = other->mYOffset;
    mXPrecision = other->mXPrecision;
    mYPrecision = other->mYPrecision;
    mDownTime = other->mDownTime;
	mEventTime= other->mEventTime;
    mPointerProperties = other->mPointerProperties;

    if (keepHistory) {
        mSampleEventTimes = other->mSampleEventTimes;
        mSamplePointerCoords = other->mSamplePointerCoords;
    } else {
        mSampleEventTimes.clear();
        mSampleEventTimes.push_back(other->getEventTime());
        mSamplePointerCoords.clear();
        size_t pointerCount = other->getPointerCount();
        size_t historySize = other->getHistorySize();

        mSamplePointerCoords.resize(pointerCount);    
        for(int i=0;i<pointerCount;i++)
            mSamplePointerCoords.push_back(other->mSamplePointerCoords.at(historySize*pointerCount+i));
    }
}

MotionEvent*MotionEvent::split(int idBits){
    MotionEvent*ev = obtain();
    int oldPointerCount = getPointerCount();
    PointerProperties pp[oldPointerCount];// = gSharedTempPointerProperties;
    PointerCoords pc[oldPointerCount];// = gSharedTempPointerCoords;
    int map[oldPointerCount];// = gSharedTempPointerIndexMap;

    int oldAction = getAction();
    int oldActionMasked = oldAction & ACTION_MASK;
    int oldActionPointerIndex = (oldAction & ACTION_POINTER_INDEX_MASK)
            >> ACTION_POINTER_INDEX_SHIFT;
    int newActionPointerIndex = -1;
    int newPointerCount = 0;
    int newIdBits = 0;
    for (int i = 0; i < oldPointerCount; i++) {
        getPointerProperties(i, &pp[newPointerCount]);
        int idBit = 1 << pp[newPointerCount].id;
        if ((idBit & idBits) != 0) {
            if (i == oldActionPointerIndex) {
                newActionPointerIndex = newPointerCount;
            }
            map[newPointerCount] = i;
            newPointerCount += 1;
            newIdBits |= idBit;
        }
    }

    if (newPointerCount == 0) {
        LOGE("idBits did not match any ids in the event");
    }

    int newAction;
    if (oldActionMasked == ACTION_POINTER_DOWN || oldActionMasked == ACTION_POINTER_UP) {
        if (newActionPointerIndex < 0) {   // An unrelated pointer changed.
            newAction = ACTION_MOVE;
        } else if (newPointerCount == 1) { // The first/last pointer went down/up.
            newAction = oldActionMasked == ACTION_POINTER_DOWN ? ACTION_DOWN : ACTION_UP;
        } else { // A secondary pointer went down/up.
            newAction = oldActionMasked | (newActionPointerIndex << ACTION_POINTER_INDEX_SHIFT);
        }
    } else {    // Simple up/down/cancel/move or other motion action.
        newAction = oldAction;
    }

    int historySize = getHistorySize();
    for (int h = 0; h <= historySize; h++) {
        int historyPos = h == historySize ? HISTORY_CURRENT : h;

        for (int i = 0; i < newPointerCount; i++) {
            //getPointerCoords(map[i], historyPos, &pc[i]);
            getHistoricalRawPointerCoords(map[i], historyPos, pc[i]);
        }

        long eventTimeNanos = getHistoricalEventTime(historyPos);
        if (h == 0) {
            ev->initialize( getDeviceId(),getSource(),  newAction, 0,
                    getFlags(),   getEdgeFlags(), getMetaState(),
                    getButtonState(), getXOffset(), getYOffset(),
                    getXPrecision(), getYPrecision(), mDownTime,
                    eventTimeNanos,  newPointerCount, pp, pc);
        } else {
            //nativeAddBatch(ev.mNativePtr, eventTimeNanos, pc, 0);
            ev->addSample(eventTimeNanos,pp[h],pc[h]);
        }
    }
    return ev;
}

bool MotionEvent::isButtonPressed(int button)const{
    return (button!=0)&&((getButtonState() & button) == button);
}

void MotionEvent::addSample(nsecs_t eventTime,const PointerProperties&prop, const PointerCoords&coord) {
    mSampleEventTimes.push_back(eventTime);
    mPointerProperties.push_back(prop);  
    mSamplePointerCoords.push_back(coord);
}

const PointerCoords* MotionEvent::getRawPointerCoords(size_t pointerIndex) const {
    return &mSamplePointerCoords[getHistorySize() * getPointerCount() + pointerIndex];
}

int MotionEvent::getPointerIdBits()const{
    int idBits = 0;
    int pointerCount = getPointerCount();
    for (int i = 0; i < pointerCount; i++) {
        idBits |= 1 << getPointerId(i);
    }
    return idBits;
}

float MotionEvent::getRawAxisValue(int32_t axis, size_t pointerIndex) const {
    return getRawPointerCoords(pointerIndex)->getAxisValue(axis);
}

float MotionEvent::getAxisValue(int axis)const {
    return getAxisValue(axis, 0);
}

float MotionEvent::getAxisValue(int32_t axis, size_t pointerIndex) const {
    float value =getRawPointerCoords(pointerIndex)->getAxisValue(axis);
    switch (axis) {
    case AXIS_X://AMOTION_EVENT_AXIS_X:
        return value + mXOffset;
    case AXIS_Y://AMOTION_EVENT_AXIS_Y:
        return value + mYOffset;
    }
    return value;
}

nsecs_t MotionEvent::getHistoricalEventTime(size_t historyPos) const{
    if(historyPos==HISTORY_CURRENT){
         return getEventTime();
    }else{
        const size_t historySize = getHistorySize();
        if(historyPos<0||historyPos>=historySize)return 0;
        return mSampleEventTimes[historyPos];
    }
}

nsecs_t MotionEvent::getHistoricalEventTimeNano(size_t historyPos) const{
    if(historyPos==HISTORY_CURRENT){
         return getEventTimeNano();
    }else{
        const size_t historySize = getHistorySize();
        if(historyPos<0||historyPos>=historySize)return 0;
        return mSampleEventTimes[historyPos]*NS_PER_MS;
    }
}

ssize_t MotionEvent::findPointerIndex(int32_t pointerId) const {
    size_t pointerCount = mPointerProperties.size();
    for (size_t i = 0; i < pointerCount; i++) {
        if (mPointerProperties[i].id == pointerId) {
            return i;
        }
    }
    return -1;
}

bool  MotionEvent::isTouchEvent(int32_t source, int32_t action){
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

bool MotionEvent::isTouchEvent()const{
    return isTouchEvent(mSource, mAction);
}

void MotionEvent::offsetLocation(float xOffset, float yOffset) {
    mXOffset += xOffset;
    mYOffset += yOffset;
}

void MotionEvent::setLocation(float x, float y){
    offsetLocation(x-getX(),y-getY());
}

void MotionEvent::scale(float scaleFactor) {
    mXOffset *= scaleFactor;
    mYOffset *= scaleFactor;
    mXPrecision *= scaleFactor;
    mYPrecision *= scaleFactor;

    size_t numSamples = mSamplePointerCoords.size();
    for (size_t i = 0; i < numSamples; i++) {
        mSamplePointerCoords[i].scale(scaleFactor);//editableitemat(i)
    }
}

const std::string MotionEvent::actionToString(int action){
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
    int index = (action & ACTION_POINTER_INDEX_MASK) >> ACTION_POINTER_INDEX_SHIFT;
    std::ostringstream oss;
    switch (action & ACTION_MASK) {
    case ACTION_POINTER_DOWN: oss<<"ACTION_POINTER_DOWN("<<index<<")";return oss.str();
    case ACTION_POINTER_UP  : oss<<"ACTION_POINTER_UP(" <<index<<")"; return oss.str();
    default: return std::to_string(action);
    }
}

static void transformPoint(const float matrix[9], float x, float y, float *outX, float *outY) {
    // Apply perspective transform like Skia.
    float newX = matrix[0] * x + matrix[1] * y + matrix[2];
    float newY = matrix[3] * x + matrix[4] * y + matrix[5];
    float newZ = matrix[6] * x + matrix[7] * y + matrix[8];
    if (newZ) 
        newZ = 1.0f / newZ;
    *outX = newX * newZ;
    *outY = newY * newZ;
}

static float transformAngle(const float matrix[9], float angleRadians,float originX, float originY) {
    // Construct and transform a vector oriented at the specified clockwise angle from vertical.
    // Coordinate system: down is increasing Y, right is increasing X.
    float x = sinf(angleRadians);
    float y = -cosf(angleRadians);
    transformPoint(matrix, x, y, &x, &y);
    x -= originX;
    y -= originY;

    // Derive the transformed vector's clockwise angle from vertical.
    float result = atan2f(x, -y);
    if (result < - M_PI_2) {
        result += M_PI;
    } else if (result > M_PI_2) {
        result -= M_PI;
    }
    return result;
}

void MotionEvent::transform(const float matrix[9]){
    float oldXOffset = mXOffset;
    float oldYOffset = mYOffset;
    float newX, newY;
    float rawX = getRawX(0);
    float rawY = getRawY(0);
    transformPoint(matrix, rawX + oldXOffset, rawY + oldYOffset, &newX, &newY);
    mXOffset = newX - rawX;
    mYOffset = newY - rawY;

    // Determine how the origin is transformed by the matrix so that we
    // can transform orientation vectors.
    float originX, originY;
    transformPoint(matrix, 0, 0, &originX, &originY);

    // Apply the transformation to all samples.
    size_t numSamples = mSamplePointerCoords.size();
    for (size_t i = 0; i < numSamples; i++) {
        PointerCoords& c = mSamplePointerCoords[i];
        float x = c.getAxisValue(AXIS_X) + oldXOffset;
        float y = c.getAxisValue(AXIS_Y) + oldYOffset;
        transformPoint(matrix, x, y, &x, &y);
        c.setAxisValue(AXIS_X, x - mXOffset);
        c.setAxisValue(AXIS_Y, y - mYOffset);

        float orientation = c.getAxisValue(AXIS_ORIENTATION);
        c.setAxisValue(AXIS_ORIENTATION,transformAngle(matrix, orientation, originX, originY));
    }   

}

static void Matrix2Float9(const Cairo::Matrix& m,float *f9){
    /*Matrix{
      double xx; double yx;
      double xy; double yy;
      double x0; double y0;}*/
    //x_new = xx * x + xy * y + x0;
    //y_new = yx * x + yy * y + y0;
    f9[0] = m.xx ; //scaleX
    f9[1] = m.xy ;  //skewX
    f9[2] = m.x0 ; //skewY

    f9[3] = m.yx ;
    f9[4] = m.yy ;
    f9[5] = m.y0 ;

    f9[6] = f9[7] =.0f;
    f9[8] = 1.f;
}

void MotionEvent::transform(const Cairo::Matrix& matrix){
    float f9[9];
    Matrix2Float9(matrix,f9);
    transform(f9);
}

void MotionEvent::getHistoricalRawPointerCoords(
        size_t pointerIndex, size_t historicalIndex,PointerCoords&out) const {
    const size_t pointerCount = getPointerCount();
    if(pointerIndex<0||pointerIndex>=pointerCount)return;
    if(historicalIndex==HISTORY_CURRENT){
        out = mSamplePointerCoords[pointerIndex];
    }else{
        out = mSamplePointerCoords[historicalIndex * getPointerCount() + pointerIndex];
    }
}

float MotionEvent::getHistoricalRawAxisValue(int32_t axis, size_t pointerIndex,
        size_t historicalIndex) const {
    PointerCoords pc;
    getHistoricalRawPointerCoords(pointerIndex,historicalIndex,pc);
    return pc.getAxisValue(axis);
}
// --- PooledInputEventFactory ---
PooledInputEventFactory*PooledInputEventFactory::mInst=nullptr;

PooledInputEventFactory& PooledInputEventFactory::getInstance(){
    if(nullptr==mInst)
	mInst=new PooledInputEventFactory(20);
    return *mInst;
}

PooledInputEventFactory::PooledInputEventFactory(size_t maxPoolSize) :
        mMaxPoolSize(maxPoolSize) {
}

PooledInputEventFactory::~PooledInputEventFactory() {
    for (size_t i = 0; i < mKeyEventPool.size(); i++) {
        delete mKeyEventPool.front();
        mKeyEventPool.pop();
    }
    for (size_t i = 0; i < mMotionEventPool.size(); i++) {
        delete mMotionEventPool.front();
        mMotionEventPool.pop();
    }
}

KeyEvent* PooledInputEventFactory::createKeyEvent() {
    if (mKeyEventPool.size()) {
        KeyEvent* event = mKeyEventPool.front();
        mKeyEventPool.pop();
        return event;
    }
    return new KeyEvent();
}

MotionEvent* PooledInputEventFactory::createMotionEvent() {
    if (mMotionEventPool.size()) {
        MotionEvent* event = mMotionEventPool.front();
        mMotionEventPool.pop();
        return event;
    }
    return new MotionEvent();
}

void PooledInputEventFactory::recycle(InputEvent* event) {
    switch (event->getType()) {
    case EV_KEY:
        if (mKeyEventPool.size() < mMaxPoolSize) {
            mKeyEventPool.push(static_cast<KeyEvent*>(event));
            return;
        }
        break;
    case EV_ABS:
        if (mMotionEventPool.size() < mMaxPoolSize) {
            mMotionEventPool.push(static_cast<MotionEvent*>(event));
            return;
        }
        break;
    }
    delete event;
}

}
