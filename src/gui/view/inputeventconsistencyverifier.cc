#include <view/inputeventconsistencyverifier.h>
#include <core/inputdevice.h>
#include <porting/cdlog.h>
#include <cstdarg>

namespace cdroid{
static const char* EVENT_TYPE_KEY = "KeyEvent";
static const char* EVENT_TYPE_TRACKBALL = "TrackballEvent";
static const char* EVENT_TYPE_TOUCH = "TouchEvent";
static const char* EVENT_TYPE_GENERIC_MOTION = "GenericMotionEvent";	

InputEventConsistencyVerifier::InputEventConsistencyVerifier(Object* caller, int flags) {
    mCaller = caller;
    mFlags = flags;
    mMostRecentEventIndex = 0;
    mCurrentEvent = nullptr;
    mKeyStateList = nullptr;
}

InputEventConsistencyVerifier::~InputEventConsistencyVerifier(){
    KeyState*keyState = mKeyStateList;
    while(keyState){
        KeyState* tmp = keyState;
        keyState = tmp->next;
        delete tmp;
    }
}

/**
 * Determines whether the instrumentation should be enabled.
 * @return True if it should be enabled.
 */
bool InputEventConsistencyVerifier::isInstrumentationEnabled() {
#ifndef _DEBUG
    return true;//IS_ENG_BUILD;
#else
    return false;
#endif
}

void InputEventConsistencyVerifier::reset() {
    mLastEventSeq = -1;
    mLastNestingLevel = 0;
    mTrackballDown = false;
    mTrackballUnhandled = false;
    mTouchEventStreamPointers = 0;
    mTouchEventStreamIsTainted = false;
    mTouchEventStreamUnhandled = false;
    mHoverEntered = false;
    mButtonsPressed = 0;

    while (mKeyStateList != nullptr) {
        KeyState* state = mKeyStateList;
        mKeyStateList = state->next;
        state->recycle();
    }
}

/**
 * Checks an arbitrary input event.
 * @param event The event.
 * @param nestingLevel The nesting level: 0 if called from the base class,
 * or 1 from a subclass.  If the event was already checked by this consistency verifier
 * at a higher nesting level, it will not be checked again.  Used to handle the situation
 * where a subclass dispatching method delegates to its superclass's dispatching method
 * and both dispatching methods call into the consistency verifier.
 */
void InputEventConsistencyVerifier::onInputEvent(InputEvent& event, int nestingLevel) {
    if (dynamic_cast<KeyEvent*>(&event)) {
        KeyEvent& keyEvent = (KeyEvent&)event;
        onKeyEvent(keyEvent, nestingLevel);
    } else {
        MotionEvent& motionEvent = (MotionEvent&)event;
        if (motionEvent.isTouchEvent()) {
            onTouchEvent(motionEvent, nestingLevel);
        } else if ((motionEvent.getSource() & InputDevice::SOURCE_CLASS_TRACKBALL) != 0) {
            onTrackballEvent(motionEvent, nestingLevel);
        } else {
            onGenericMotionEvent(motionEvent, nestingLevel);
        }
    }
}

/**
 * Checks a key event.
 * @param event The event.
 * @param nestingLevel The nesting level: 0 if called from the base class,
 * or 1 from a subclass.  If the event was already checked by this consistency verifier
 * at a higher nesting level, it will not be checked again.  Used to handle the situation
 * where a subclass dispatching method delegates to its superclass's dispatching method
 * and both dispatching methods call into the consistency verifier.
 */
void InputEventConsistencyVerifier::onKeyEvent(KeyEvent& event, int nestingLevel) {
    if (!startEvent(event, nestingLevel, EVENT_TYPE_KEY)) {
        return;
    }

    ensureMetaStateIsNormalized(event.getMetaState());

    const int action = event.getAction();
    const int deviceId = event.getDeviceId();
    const int source = event.getSource();
    const int keyCode = event.getKeyCode();
    switch (action) {
    case KeyEvent::ACTION_DOWN: {
        KeyState* state = findKeyState(deviceId, source, keyCode, /*remove*/ false);
        if (state != nullptr) {
            // If the key is already down, ensure it is a repeat.
            // We don't perform this check when processing raw device input
            // because the input dispatcher itself is responsible for setting
            // the key repeat count before it delivers input events.
            if (state->unhandled) {
                state->unhandled = false;
            } else if ((mFlags & FLAG_RAW_DEVICE_INPUT) == 0
                    && event.getRepeatCount() == 0) {
                problem("ACTION_DOWN but key is already down and this event is not a key repeat.");
            }
        } else {
            addKeyState(deviceId, source, keyCode);
        }
        break;
    }
    case KeyEvent::ACTION_UP: {
        KeyState* state = findKeyState(deviceId, source, keyCode, /*remove*/ true);
        if (state == nullptr) {
            problem("ACTION_UP but key was not down.");
        } else {
            state->recycle();
        }
        break;
        }
    case KeyEvent::ACTION_MULTIPLE:
        break;
    default:
        problem("Invalid action %s for key event" ,KeyEvent::actionToString(action).c_str());
            break;
    }
    finishEvent();
}

/**
 * Checks a trackball event.
 * @param event The event.
 * @param nestingLevel The nesting level: 0 if called from the base class,
 * or 1 from a subclass.  If the event was already checked by this consistency verifier
 * at a higher nesting level, it will not be checked again.  Used to handle the situation
 * where a subclass dispatching method delegates to its superclass's dispatching method
 * and both dispatching methods call into the consistency verifier.
 */
void InputEventConsistencyVerifier::onTrackballEvent(MotionEvent& event, int nestingLevel) {
    if (!startEvent(event, nestingLevel, EVENT_TYPE_TRACKBALL)) {
        return;
    }

    ensureMetaStateIsNormalized(event.getMetaState());

    const int action = event.getAction();
    const int source = event.getSource();
    if ((source & InputDevice::SOURCE_CLASS_TRACKBALL) != 0) {
        switch (action) {
        case MotionEvent::ACTION_DOWN:
            if (mTrackballDown && !mTrackballUnhandled) {
                problem("ACTION_DOWN but trackball is already down.");
            } else {
                mTrackballDown = true;
                mTrackballUnhandled = false;
            }
            ensureHistorySizeIsZeroForThisAction(event);
            ensurePointerCountIsOneForThisAction(event);
            break;
	case MotionEvent::ACTION_UP:
            if (!mTrackballDown) {
                problem("ACTION_UP but trackball is not down.");
            } else {
                mTrackballDown = false;
                mTrackballUnhandled = false;
            }
            ensureHistorySizeIsZeroForThisAction(event);
            ensurePointerCountIsOneForThisAction(event);
            break;
	case MotionEvent::ACTION_MOVE:
            ensurePointerCountIsOneForThisAction(event);
            break;
        default:
            problem("Invalid action " + MotionEvent::actionToString(action)
                    + " for trackball event.");
            break;
        }

        if (mTrackballDown && event.getPressure(0) <= 0.f) {
            problem("Trackball is down but pressure is not greater than 0.");
        } else if (!mTrackballDown && event.getPressure(0) != 0) {
            problem("Trackball is up but pressure is not equal to 0.");
        }
    } else {
        problem("Source was not SOURCE_CLASS_TRACKBALL.");
    }
    finishEvent();
}

static int bitCount(int number) {
    int count = 0;
    while (number) {
        count += number & 1;
        number >>= 1;
    }
    return count;
}
/**
 * Checks a touch event.
 * @param event The event.
 * @param nestingLevel The nesting level: 0 if called from the base class,
 * or 1 from a subclass.  If the event was already checked by this consistency verifier
 * at a higher nesting level, it will not be checked again.  Used to handle the situation
 * where a subclass dispatching method delegates to its superclass's dispatching method
 * and both dispatching methods call into the consistency verifier.
 */
void InputEventConsistencyVerifier::onTouchEvent(MotionEvent& event, int nestingLevel) {
    if (!startEvent(event, nestingLevel, EVENT_TYPE_TOUCH)) {
        return;
    }

    const int action = event.getAction();
    const bool newStream = action == MotionEvent::ACTION_DOWN
            || action == MotionEvent::ACTION_CANCEL || action == MotionEvent::ACTION_OUTSIDE;
    if (newStream && (mTouchEventStreamIsTainted || mTouchEventStreamUnhandled)) {
        mTouchEventStreamIsTainted = false;
        mTouchEventStreamUnhandled = false;
        mTouchEventStreamPointers = 0;
    }
    if (mTouchEventStreamIsTainted) {
        event.setTainted(true);
    }

    ensureMetaStateIsNormalized(event.getMetaState());

    const int deviceId = event.getDeviceId();
    const int source = event.getSource();

    if (!newStream && mTouchEventStreamDeviceId != -1
            && (mTouchEventStreamDeviceId != deviceId
                    || mTouchEventStreamSource != source)) {
        problem("Touch event stream contains events from multiple sources: previous device id %d"
                ", previous source %x, new device id %d, new source %x",
    	   mTouchEventStreamDeviceId,mTouchEventStreamSource,deviceId,source);
    }
    mTouchEventStreamDeviceId = deviceId;
    mTouchEventStreamSource = source;

    const int pointerCount = (int)event.getPointerCount();
    if ((source & InputDevice::SOURCE_CLASS_POINTER) != 0) {
        switch (action) {
        case MotionEvent::ACTION_DOWN:
            if (mTouchEventStreamPointers != 0) {
                problem("ACTION_DOWN but pointers are already down.  "
                        "Probably missing ACTION_UP from previous gesture.");
            }
            ensureHistorySizeIsZeroForThisAction(event);
            ensurePointerCountIsOneForThisAction(event);
            mTouchEventStreamPointers = 1 << event.getPointerId(0);
            break;
	    case MotionEvent::ACTION_UP:
            ensureHistorySizeIsZeroForThisAction(event);
            ensurePointerCountIsOneForThisAction(event);
            mTouchEventStreamPointers = 0;
            mTouchEventStreamIsTainted = false;
            break;
	    case MotionEvent::ACTION_MOVE: {
            const int expectedPointerCount = bitCount(mTouchEventStreamPointers);
            if (pointerCount != expectedPointerCount) {
                problem("ACTION_MOVE contained %d pointers but there are currently %d pointers down.",pointerCount,expectedPointerCount);
                mTouchEventStreamIsTainted = true;
            }
            break;
        }
	    case MotionEvent::ACTION_CANCEL:
            mTouchEventStreamPointers = 0;
            mTouchEventStreamIsTainted = false;
            break;
	    case MotionEvent::ACTION_OUTSIDE:
            if (mTouchEventStreamPointers != 0) {
                problem("ACTION_OUTSIDE but pointers are still down.");
            }
            ensureHistorySizeIsZeroForThisAction(event);
            ensurePointerCountIsOneForThisAction(event);
            mTouchEventStreamIsTainted = false;
            break;
        default: {
                const int actionMasked = event.getActionMasked();
                const int actionIndex = event.getActionIndex();
                if (actionMasked == MotionEvent::ACTION_POINTER_DOWN) {
                    if (mTouchEventStreamPointers == 0) {
                        problem("ACTION_POINTER_DOWN but no other pointers were down.");
                        mTouchEventStreamIsTainted = true;
                    }
                    if (actionIndex < 0 || actionIndex >= pointerCount) {
                        problem("ACTION_POINTER_DOWN index is %d but the pointer count is %d.",
    		         actionIndex,pointerCount);
                        mTouchEventStreamIsTainted = true;
                    } else {
                        const int id = event.getPointerId(actionIndex);
                        const int idBit = 1 << id;
                        if ((mTouchEventStreamPointers & idBit) != 0) {
                            problem("ACTION_POINTER_DOWN specified pointer id %d which is already down.",id);
                            mTouchEventStreamIsTainted = true;
                        } else {
                            mTouchEventStreamPointers |= idBit;
                        }
                    }
                    ensureHistorySizeIsZeroForThisAction(event);
                } else if (actionMasked == MotionEvent::ACTION_POINTER_UP) {
                    if (actionIndex < 0 || actionIndex >= pointerCount) {
                        problem("ACTION_POINTER_UP index is % but the pointer count is %d.",actionIndex,pointerCount);
                        mTouchEventStreamIsTainted = true;
                    } else {
                        const int id = event.getPointerId(actionIndex);
                        const int idBit = 1 << id;
                        if ((mTouchEventStreamPointers & idBit) == 0) {
                            problem("ACTION_POINTER_UP specified pointer id %d which is not currently down.",id);
                            mTouchEventStreamIsTainted = true;
                        } else {
                            mTouchEventStreamPointers &= ~idBit;
                        }
                    }
                    ensureHistorySizeIsZeroForThisAction(event);
                } else {
                    problem("Invalid action %s for touch event.",MotionEvent::actionToString(action).c_str());
                }
                break;
            }
        }
    } else {
        problem("Source %d was not SOURCE_CLASS_POINTER.",source);
    }
    finishEvent();
}

/**
 * Checks a generic motion event.
 * @param event The event.
 * @param nestingLevel The nesting level: 0 if called from the base class,
 * or 1 from a subclass.  If the event was already checked by this consistency verifier
 * at a higher nesting level, it will not be checked again.  Used to handle the situation
 * where a subclass dispatching method delegates to its superclass's dispatching method
 * and both dispatching methods call into the consistency verifier.
 */
void InputEventConsistencyVerifier::onGenericMotionEvent(MotionEvent& event, int nestingLevel) {
    if (!startEvent(event, nestingLevel, EVENT_TYPE_GENERIC_MOTION)) {
        return;
    }

    ensureMetaStateIsNormalized(event.getMetaState());

    const int action = event.getAction();
    const int source = event.getSource();
    const int buttonState = event.getButtonState();
    const int actionButton = event.getActionButton();
    if ((source & InputDevice::SOURCE_CLASS_POINTER) != 0) {
        switch (action) {
        case MotionEvent::ACTION_HOVER_ENTER:
            ensurePointerCountIsOneForThisAction(event);
            mHoverEntered = true;
            break;
	case MotionEvent::ACTION_HOVER_MOVE:
            ensurePointerCountIsOneForThisAction(event);
            break;
	case MotionEvent::ACTION_HOVER_EXIT:
            ensurePointerCountIsOneForThisAction(event);
            if (!mHoverEntered) {
                problem("ACTION_HOVER_EXIT without prior ACTION_HOVER_ENTER");
            }
            mHoverEntered = false;
            break;
	case MotionEvent::ACTION_SCROLL:
            ensureHistorySizeIsZeroForThisAction(event);
            ensurePointerCountIsOneForThisAction(event);
            break;
	case MotionEvent::ACTION_BUTTON_PRESS:
            ensureActionButtonIsNonZeroForThisAction(event);
            if ((mButtonsPressed & actionButton) != 0) {
                problem("Action button for ACTION_BUTTON_PRESS event is %d"
                        ", but it has already been pressed and "
                        "has yet to be released.",actionButton);
            }

            mButtonsPressed |= actionButton;
            // The system will automatically mirror the stylus buttons onto the button
            // state as the old set of generic buttons for apps targeting pre-M. If
            // it looks this has happened, go ahead and set the generic buttons as
            // pressed to prevent spurious errors.
            if (actionButton == MotionEvent::BUTTON_STYLUS_PRIMARY &&
                    (buttonState & MotionEvent::BUTTON_SECONDARY) != 0) {
                mButtonsPressed |= MotionEvent::BUTTON_SECONDARY;
            } else if (actionButton == MotionEvent::BUTTON_STYLUS_SECONDARY &&
                    (buttonState & MotionEvent::BUTTON_TERTIARY) != 0) {
                mButtonsPressed |= MotionEvent::BUTTON_TERTIARY;
            }

            if (mButtonsPressed != buttonState) {
                problem("Reported button state differs from expect button state "
    		 "based on press and release events. Is 0x%08x but expected 0x%08x.",
                     buttonState, mButtonsPressed);
            }
            break;
	case MotionEvent::ACTION_BUTTON_RELEASE:
            ensureActionButtonIsNonZeroForThisAction(event);
            if ((mButtonsPressed & actionButton) != actionButton) {
                problem("Action button for ACTION_BUTTON_RELEASE event is %d"
                        ", but it was either never pressed or has "
                        "already been released.",actionButton);
            }

            mButtonsPressed &= ~actionButton;
            // The system will automatically mirror the stylus buttons onto the button
            // state as the old set of generic buttons for apps targeting pre-M. If
            // it looks this has happened, go ahead and set the generic buttons as
            // released to prevent spurious errors.
            if (actionButton == MotionEvent::BUTTON_STYLUS_PRIMARY &&
                    (buttonState & MotionEvent::BUTTON_SECONDARY) == 0) {
                mButtonsPressed &= ~MotionEvent::BUTTON_SECONDARY;
            } else if (actionButton == MotionEvent::BUTTON_STYLUS_SECONDARY &&
                    (buttonState & MotionEvent::BUTTON_TERTIARY) == 0) {
                mButtonsPressed &= ~MotionEvent::BUTTON_TERTIARY;
            }

            if (mButtonsPressed != buttonState) {
                problem("Reported button state differs from "
                        "expected button state based on press and release events. "
                        "Is 0x%08x but expected 0x%08x.",
                        buttonState, mButtonsPressed);
            }
            break;
        default:
            problem("Invalid action for generic pointer event.");
            break;
        }
    } else if ((source & InputDevice::SOURCE_CLASS_JOYSTICK) != 0) {
        switch (action) {
        case MotionEvent::ACTION_MOVE:
            ensurePointerCountIsOneForThisAction(event);
            break;
        default:
            problem("Invalid action for generic joystick event.");
            break;
        }
    }
    finishEvent();
}

/**
 * Notifies the verifier that a given event was unhandled and the rest of the
 * trace for the event should be ignored.
 * This method should only be called if the event was previously checked by
 * the consistency verifier using {@link #onInputEvent} and other methods.
 * @param event The event.
 * @param nestingLevel The nesting level: 0 if called from the base class,
 * or 1 from a subclass.  If the event was already checked by this consistency verifier
 * at a higher nesting level, it will not be checked again.  Used to handle the situation
 * where a subclass dispatching method delegates to its superclass's dispatching method
 * and both dispatching methods call into the consistency verifier.
 */
void InputEventConsistencyVerifier::onUnhandledEvent(InputEvent& event, int nestingLevel) {
    if (nestingLevel != mLastNestingLevel) {
        return;
    }

    if (mRecentEventsUnhandled.size()) {
        mRecentEventsUnhandled[mMostRecentEventIndex] = true;
    }

    if (dynamic_cast<KeyEvent*>(&event)) {
        const KeyEvent& keyEvent = (KeyEvent&)event;
        const int deviceId = keyEvent.getDeviceId();
        const int source = keyEvent.getSource();
        const int keyCode = keyEvent.getKeyCode();
        KeyState* state = findKeyState(deviceId, source, keyCode, /*remove*/ false);
        if (state != nullptr) {
            state->unhandled = true;
        }
    } else {
        MotionEvent& motionEvent = (MotionEvent&)event;
        if (motionEvent.isTouchEvent()) {
            mTouchEventStreamUnhandled = true;
        } else if ((motionEvent.getSource() & InputDevice::SOURCE_CLASS_TRACKBALL) != 0) {
            if (mTrackballDown) {
                mTrackballUnhandled = true;
            }
        }
    }
}

void InputEventConsistencyVerifier::ensureMetaStateIsNormalized(int metaState) {
    const int normalizedMetaState = KeyEvent::normalizeMetaState(metaState);
    if (normalizedMetaState != metaState) {
        problem("Metastate not normalized.  Was 0x%08x but expected 0x%08x.",
                metaState, normalizedMetaState);
    }
}

void InputEventConsistencyVerifier::ensurePointerCountIsOneForThisAction(MotionEvent& event) {
    const size_t pointerCount = event.getPointerCount();
    if (pointerCount != 1) {
        problem("Pointer count is %d but it should always be 1 for %s",
		pointerCount, MotionEvent::actionToString(event.getAction()).c_str());
    }
}

void InputEventConsistencyVerifier::ensureActionButtonIsNonZeroForThisAction(MotionEvent& event) {
    const int actionButton = event.getActionButton();
    if (actionButton == 0) {
        problem("No action button set. Action button should always be non-zero for %s",
                MotionEvent::actionToString(event.getAction()).c_str());

    }
}

void InputEventConsistencyVerifier::ensureHistorySizeIsZeroForThisAction(MotionEvent& event) {
    const size_t historySize = event.getHistorySize();
    if (historySize != 0) {
        problem("History size is %d but it should always be 0 for %s",
		historySize,MotionEvent::actionToString(event.getAction()).c_str());
    }
}

bool InputEventConsistencyVerifier::startEvent(InputEvent& event, int nestingLevel,const std::string& eventType) {
    // Ignore the event if we already checked it at a higher nesting level.
    const int seq = event.getSequenceNumber();
    if (seq == mLastEventSeq && nestingLevel < mLastNestingLevel
            && eventType == mLastEventType) {
        return false;
    }

    if (nestingLevel > 0) {
        mLastEventSeq = seq;
        mLastEventType = eventType;
        mLastNestingLevel = nestingLevel;
    } else {
        mLastEventSeq = -1;
        mLastEventType.clear();// = nullptr;
        mLastNestingLevel = 0;
    }

    mCurrentEvent = &event;
    mCurrentEventType = eventType;
    return true;
}

void InputEventConsistencyVerifier::finishEvent() {
    if (mViolationMessage.str().length() != 0) {
        if (!mCurrentEvent->isTainted()) {
            // Write a log message only if the event was not already tainted.
            //mViolationMessage.append("\n  in ").append(mCaller);
            mViolationMessage<<("\n  ");
            appendEvent(mViolationMessage, 0, *mCurrentEvent, false);

            if (RECENT_EVENTS_TO_LOG != 0 && mRecentEvents.size()) {
                mViolationMessage<<("\n  -- recent events --");
                for (int i = 0; i < RECENT_EVENTS_TO_LOG; i++) {
                    const int index = (mMostRecentEventIndex + RECENT_EVENTS_TO_LOG - i)
                            % RECENT_EVENTS_TO_LOG;
                    InputEvent* event = mRecentEvents[index];
                    if (event == nullptr) {
                        break;
                    }
                    mViolationMessage<<("\n  ");
                    appendEvent(mViolationMessage, i + 1, *event, mRecentEventsUnhandled[index]);
                }
            }

            LOGV("%s",mViolationMessage.str().c_str());

            // Taint the event so that we do not generate additional violations from it
            // further downstream.
            mCurrentEvent->setTainted(true);
        }
        mViolationMessage.clear();//setLength(0);
    }

    if (RECENT_EVENTS_TO_LOG != 0) {
        if (mRecentEvents.empty()) {
            mRecentEvents.resize(RECENT_EVENTS_TO_LOG);
            mRecentEventsUnhandled.resize(RECENT_EVENTS_TO_LOG);
        }
        const int index = (mMostRecentEventIndex + 1) % RECENT_EVENTS_TO_LOG;
        mMostRecentEventIndex = index;
        if (mRecentEvents[index] != nullptr) {
            mRecentEvents[index]->recycle();
        }
        mRecentEvents[index] = mCurrentEvent->copy();
        mRecentEventsUnhandled[index] = false;
    }

    mCurrentEvent = nullptr;
    mCurrentEventType.clear();
}

void InputEventConsistencyVerifier::appendEvent(std::ostringstream& message, int index,
        const InputEvent& event, bool unhandled) {
    message<<index<<": sent at "<<(event.getEventTimeNanos());
    message<<", ";
    if (unhandled) {
        message<<"(unhandled) ";
    }
    message<<event;
}

void InputEventConsistencyVerifier::problem(const std::string message,...) {
    va_list args;
    char buffer [512];
    va_start(args, message);
    vsnprintf(buffer,sizeof(buffer),message.c_str(),args);
    va_end(args);
    if (mViolationMessage.str().length() == 0) {
        mViolationMessage<<mCurrentEventType<<": ";
    } else {
        mViolationMessage<<"\n  ";
    }
    mViolationMessage<<buffer;
}

InputEventConsistencyVerifier::KeyState* InputEventConsistencyVerifier::findKeyState(int deviceId, int source, int keyCode, bool remove) {
    KeyState* last = nullptr;
    KeyState* state = mKeyStateList;
    while (state != nullptr) {
        if (state->deviceId == deviceId && state->source == source
                && state->keyCode == keyCode) {
            if (remove) {
                if (last != nullptr) {
                    last->next = state->next;
                } else {
                    mKeyStateList = state->next;
                }
                state->next = nullptr;
            }
            return state;
        }
        last = state;
        state = state->next;
    }
    return nullptr;
}

void InputEventConsistencyVerifier::addKeyState(int deviceId, int source, int keyCode) {
    KeyState* state = KeyState::obtain(deviceId, source, keyCode);
    state->next = mKeyStateList;
    mKeyStateList = state;
}

InputEventConsistencyVerifier::KeyState* InputEventConsistencyVerifier::KeyState::mRecycledList = nullptr;
InputEventConsistencyVerifier::KeyState::KeyState() {
}

InputEventConsistencyVerifier::KeyState* InputEventConsistencyVerifier::KeyState::obtain(int deviceId, int source, int keyCode) {
    KeyState* state;
    state = mRecycledList;
    if (state != nullptr) {
        mRecycledList = state->next;
    } else {
        state = new KeyState();
    }
    state->deviceId = deviceId;
    state->source = source;
    state->keyCode = keyCode;
    state->unhandled = false;
    return state;
}

void InputEventConsistencyVerifier::KeyState::recycle() {
    next = mRecycledList;
    mRecycledList = next;
}

}/*endof namespace*/
