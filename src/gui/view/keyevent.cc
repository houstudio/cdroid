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
#include <view/keyevent.h>
#include <private/inputeventlabels.h>
#include <porting/cdlog.h>
#include <sstream>
namespace cdroid{

KeyEvent* KeyEvent::obtain(){
    KeyEvent*ev = PooledInputEventFactory::getInstance().createKeyEvent();
    ev->prepareForReuse();
    return ev;
}

KeyEvent* KeyEvent::obtain(nsecs_t downTime, nsecs_t eventTime, int action,int code, int repeat, int metaState,
          int deviceId, int scancode, int flags, int32_t source,int displayId/*, std::string characters*/){
    KeyEvent* ev = obtain();
    ev->mId = InputEvent::nextId();
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
    ev->mDisplayId = displayId;
    //ev->mCharacters = characters;
    return ev;
}

KeyEvent* KeyEvent::obtain(const KeyEvent& other){
    KeyEvent* ev = obtain();
    ev->mId = other.mId;
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
    ev->mDisplayId = other.mDisplayId;
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
        if (mKeyCode != KEYCODE_UNKNOWN) {
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

const std::string KeyEvent::keyCodeToString(int keyCode){
    std::string symbolicName = getLabel(keyCode);
    if(symbolicName.empty())
        return std::string("KEYCODE_")+symbolicName;
    return std::to_string(keyCode);
}

int KeyEvent::keyCodeFromString(const std::string& symbolicName){
    const int base =( ((symbolicName.length()>2) &&
                (symbolicName[1]=='x'||symbolicName[1]=='X')) ||(symbolicName[0]=='#'))?16:10;
    int keyCode = std::strtol(symbolicName.c_str(),nullptr,base);
    if(keyCodeIsValid(keyCode)){
        return keyCode;
    }
    keyCode = KeyEvent::getKeyCodeFromLabel(symbolicName.c_str());
    if(keyCodeIsValid(keyCode)) return keyCode;
    return KEYCODE_UNKNOWN;
}

const char* KeyEvent::getLabel(int keyCode) {
    return  InputEventLookup::getLabelByKeyCode(keyCode);//getLabelByKeyCode(keyCode);
}

int32_t KeyEvent::getKeyCodeFromLabel(const char* label) {
    return  InputEventLookup::getKeyCodeByLabel(label);//getKeyCodeByLabel(label);
}

bool KeyEvent::isModifierKey(int keyCode){
    switch (keyCode) {
    case KEYCODE_SHIFT_LEFT:
    case KEYCODE_SHIFT_RIGHT:
    case KEYCODE_ALT_LEFT:
    case KEYCODE_ALT_RIGHT:
    case KEYCODE_CTRL_LEFT:
    case KEYCODE_CTRL_RIGHT:
    case KEYCODE_META_LEFT:
    case KEYCODE_META_RIGHT:
    case KEYCODE_SYM:
    case KEYCODE_NUM:
    case KEYCODE_FUNCTION:return true;
    default:return false;
    }
}

bool KeyEvent::isConfirmKey(int keyCode){
    switch(keyCode){
    case KEYCODE_ENTER:
    case KEYCODE_DPAD_CENTER:
    case KEYCODE_NUMPAD_ENTER:
        return true;
    default:return false;
    }
}

void KeyEvent::initialize(int32_t deviceId, int32_t source,int32_t displayId,int32_t action, int32_t flags, int32_t keyCode,
        int32_t scanCode, int32_t metaState, int32_t repeatCount, nsecs_t downTime, nsecs_t eventTime) {
    InputEvent::initialize(nextId(),deviceId, source,displayId);
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
    mDisplayId = from.mDisplayId;
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

bool KeyEvent::keyCodeIsValid(int keyCode){
    return (keyCode >= KEYCODE_UNKNOWN) && (keyCode <= LAST_KEYCODE);
}

const std::string KeyEvent::metaStateToString(int metaState){
    std::ostringstream result;
    int i = 0;
    if(metaState==0) return "0";
    while (metaState != 0) {
        const bool isSet = (metaState & 1) != 0;
        metaState >>= 1; // unsigned shift!
        if (isSet) {
            std::string name = META_SYMBOLIC_NAMES[i];
            if (result.str().empty()) {
                if (metaState == 0) {
                    return name;
                }
                result << name;
            } else {
                result << "|"<< name;
            }
        }
        i += 1;
    }
    return result.str();
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
         throw std::runtime_error("Can only start tracking on a down event");
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

std::ostream& operator<<(std::ostream& out, const KeyEvent& event) {
    out << "KeyEvent { action=" << KeyEvent::actionToString(event.getAction());

    out << ", keycode=" << event.getKeyCode() << "(" << KeyEvent::getLabel(event.getKeyCode())
        << ")";

    if (event.getMetaState() != 0) {
        out << ", metaState=" << event.getMetaState();
    }

    out << ", eventTime=" << event.getEventTime();
    out << ", downTime=" << event.getDownTime();
    out << ", flags=" << std::hex << event.getFlags() << std::dec;
    out << ", repeatCount=" << event.getRepeatCount();
    out << ", deviceId=" << event.getDeviceId();
    out << ", source=" << InputEvent::sourceToString(event.getSource());
    out << ", displayId=" << event.getDisplayId();
    out << ", eventId=0x" << std::hex << event.getId() << std::dec;
    out << "}";
    return out;
}

}
