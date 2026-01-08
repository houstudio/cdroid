#ifndef __INPUTEVENT_CONSISTENCYVERIFIER_H__
#define __INPUTEVENT_CONSISTENCYVERIFIER_H__
#include <view/keyevent.h>
#include <view/motionevent.h>
#include <sstream>
#include <stdarg.h>
namespace cdroid{
class Object;
class InputEventConsistencyVerifier {
private:
    class KeyState;
    // The number of recent events to log when a problem is detected.
    // Can be set to 0 to disable logging recent events but the runtime overhead of
    // this feature is negligible on current hardware.
    static constexpr int RECENT_EVENTS_TO_LOG = 5;

    // The object to which the verifier is attached.
    Object* mCaller;

    // Consistency verifier flags.
    int mFlags;

    // The most recently checked event and the nesting level at which it was checked.
    // This is only set when the verifier is called from a nesting level greater than 0
    // so that the verifier can detect when it has been asked to verify the same event twice.
    // It does not make sense to examine the contents of the last event since it may have
    // been recycled.
    int mLastEventSeq;
    std::string mLastEventType;
    int mLastNestingLevel;

    // Copy of the most recent events.
    std::vector<InputEvent*> mRecentEvents;
    std::vector<bool> mRecentEventsUnhandled;
    int mMostRecentEventIndex;

    // Current event and its type.
    InputEvent* mCurrentEvent;
    std::string mCurrentEventType;

    // Linked list of key state objects.
    KeyState* mKeyStateList;

    // Current state of the trackball.
    bool mTrackballDown;
    bool mTrackballUnhandled;

    // Bitfield of pointer ids that are currently down.
    // Assumes that the largest possible pointer id is 31, which is potentially subject to change.
    // (See MAX_POINTER_ID in frameworks/base/include/ui/Input.h)
    int mTouchEventStreamPointers;

    // The device id and source of the current stream of touch events.
    int mTouchEventStreamDeviceId = -1;
    int mTouchEventStreamSource;

    // Set to true when we discover that the touch event stream is inconsistent.
    // Reset on down or cancel.
    bool mTouchEventStreamIsTainted;

    // Set to true if the touch event stream is partially unhandled.
    bool mTouchEventStreamUnhandled;

    // Set to true if we received hover enter.
    bool mHoverEntered;

    // The bitset of buttons which we've received ACTION_BUTTON_PRESS for.
    int mButtonsPressed;

    // The current violation message.
    std::ostringstream mViolationMessage;

    /**
     * Indicates that the verifier is intended to act on raw device input event streams.
     * Disables certain checks for invariants that are established by the input dispatcher
     * itself as it delivers input events, such as key repeating behavior.
     */
private:
    void ensureMetaStateIsNormalized(int metaState);
    void ensurePointerCountIsOneForThisAction(MotionEvent& event);
    void ensureActionButtonIsNonZeroForThisAction(MotionEvent& event);
    void ensureHistorySizeIsZeroForThisAction(MotionEvent& event);
    bool startEvent(InputEvent& event, int nestingLevel, const std::string& eventType);
    void finishEvent();
    static void appendEvent(std::ostringstream& message, int index,const InputEvent& event, bool unhandled);
    void problem(const char* message,...);
    KeyState* findKeyState(int deviceId, int source, int keyCode, bool remove);
    void addKeyState(int deviceId, int source, int keyCode);
public:
    static const int FLAG_RAW_DEVICE_INPUT = 1 << 0;


    /**
     * Creates an input consistency verifier.
     * @param caller The object to which the verifier is attached.
     * @param flags Flags to the verifier, or 0 if none.
     * @param logTag Tag for logging. If null defaults to the short class name.
     */
    InputEventConsistencyVerifier(Object* caller, int flags);
    virtual ~InputEventConsistencyVerifier();
    /**
     * Determines whether the instrumentation should be enabled.
     * @return True if it should be enabled.
     */
    static bool isInstrumentationEnabled();
    void reset();

    /**
     * Checks an arbitrary input event.
     * @param event The event.
     * @param nestingLevel The nesting level: 0 if called from the base class,
     * or 1 from a subclass.  If the event was already checked by this consistency verifier
     * at a higher nesting level, it will not be checked again.  Used to handle the situation
     * where a subclass dispatching method delegates to its superclass's dispatching method
     * and both dispatching methods call into the consistency verifier.
     */
    void onInputEvent(InputEvent& event, int nestingLevel);
    /**
     * Checks a key event.
     * @param event The event.
     * @param nestingLevel The nesting level: 0 if called from the base class,
     * or 1 from a subclass.  If the event was already checked by this consistency verifier
     * at a higher nesting level, it will not be checked again.  Used to handle the situation
     * where a subclass dispatching method delegates to its superclass's dispatching method
     * and both dispatching methods call into the consistency verifier.
     */
    void onKeyEvent(KeyEvent& event, int nestingLevel);

    /**
     * Checks a trackball event.
     * @param event The event.
     * @param nestingLevel The nesting level: 0 if called from the base class,
     * or 1 from a subclass.  If the event was already checked by this consistency verifier
     * at a higher nesting level, it will not be checked again.  Used to handle the situation
     * where a subclass dispatching method delegates to its superclass's dispatching method
     * and both dispatching methods call into the consistency verifier.
     */
    void onTrackballEvent(MotionEvent& event, int nestingLevel);

    /**
     * Checks a touch event.
     * @param event The event.
     * @param nestingLevel The nesting level: 0 if called from the base class,
     * or 1 from a subclass.  If the event was already checked by this consistency verifier
     * at a higher nesting level, it will not be checked again.  Used to handle the situation
     * where a subclass dispatching method delegates to its superclass's dispatching method
     * and both dispatching methods call into the consistency verifier.
     */
    void onTouchEvent(MotionEvent& event, int nestingLevel);
    /**
     * Checks a generic motion event.
     * @param event The event.
     * @param nestingLevel The nesting level: 0 if called from the base class,
     * or 1 from a subclass.  If the event was already checked by this consistency verifier
     * at a higher nesting level, it will not be checked again.  Used to handle the situation
     * where a subclass dispatching method delegates to its superclass's dispatching method
     * and both dispatching methods call into the consistency verifier.
     */
    void onGenericMotionEvent(MotionEvent& event, int nestingLevel);

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
    void onUnhandledEvent(InputEvent& event, int nestingLevel);
};

class InputEventConsistencyVerifier::KeyState {
private:
    static KeyState* mRecycledList;
private:
    friend InputEventConsistencyVerifier;
    KeyState();
public:
    KeyState* next;
    int deviceId;
    int source;
    int keyCode;
    bool unhandled;
    static KeyState* obtain(int deviceId, int source, int keyCode);
    void recycle();
};
}/*endof namespace*/
#endif/*__INPUTEVENT_CONSISTENCYVERIFIER_H__*/
