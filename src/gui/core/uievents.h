#ifndef __CDROID_UIEVENTS_H__
#define __CDROID_UIEVENTS_H__
#include <string>
#include <core/eventcodes.h>
#include <core/sparsearray.h>
#include <core/bitset.h>
#include <stdint.h>
#include <vector>
#include <queue>

typedef __int64_t nsecs_t;

namespace cdroid{

struct PointerCoords {
    enum { MAX_AXES = 30 }; // 30 so that sizeof(PointerCoords) == 128

    // Bitfield of axes that are present in this structure.
    uint64_t bits __attribute__((aligned(8)));

    // Values of axes that are stored in this structure packed in order by axis id
    // for each axis that is present in the structure according to 'bits'.
    float values[MAX_AXES];

    inline void clear() {
        BitSet64::clear(bits);
    }

    bool isEmpty() const {
        return BitSet64::isEmpty(bits);
    }

    float getAxisValue(int32_t axis) const;
    int setAxisValue(int32_t axis, float value);

    void scale(float scale);
    void applyOffset(float xOffset, float yOffset);

    inline float getX() const {
        return getAxisValue(ABS_X);
    }

    inline float getY() const {
        return getAxisValue(ABS_Y);
    }

    bool operator==(const PointerCoords& other) const;
    inline bool operator!=(const PointerCoords& other) const {
        return !(*this == other);
    }

    void copyFrom(const PointerCoords& other);

private:
    void tooManyAxes(int axis);
};

/* Pointer property data.*/
struct PointerProperties {
    // The id of the pointer.
    int32_t id;

    // The pointer tool type.
    int32_t toolType;

    inline void clear() {
        id = -1;
        toolType = 0;
    }

    bool operator==(const PointerProperties& other) const;
    inline bool operator!=(const PointerProperties& other) const {
        return !(*this == other);
    }

    void copyFrom(const PointerProperties& other);
};

class InputEvent{
protected:
   int mDeviceId;
   int mSource;
   long mSeq;
   nsecs_t mEventTime;//SystemClock#uptimeMicros
public:   
   enum{
     EVENT_TYPE_KEY = 1,
     EVENT_TYPE_MOTION = 2,
     EVENT_TYPE_FOCUS = 3
   };
   enum{
       SOURCE_CLASS_MASK   = 0x000000ff,
       SOURCE_CLASS_NONE   = 0x00000000,
       SOURCE_CLASS_BUTTON = 0x00000001,
       SOURCE_CLASS_POINTER= 0x00000002,
       SOURCE_CLASS_TRACKBALL= 0x00000004,
       SOURCE_CLASS_POSITION = 0x00000008,
       SOURCE_CLASS_JOYSTICK = 0x00000010,
       SOURCE_UNKNOWN  = 0x00000000,
       SOURCE_KEYBOARD = 0x00000100 | SOURCE_CLASS_BUTTON,
       SOURCE_DPAD    = 0x00000200 | SOURCE_CLASS_BUTTON,
       SOURCE_GAMEPAD = 0x00000400 | SOURCE_CLASS_BUTTON,
       SOURCE_TOUCHSCREEN = 0x00001000 | SOURCE_CLASS_POINTER,
       SOURCE_MOUSE  = 0x00002000 | SOURCE_CLASS_POINTER,
       SOURCE_STYLUS = 0x00004000 | SOURCE_CLASS_POINTER,
       SOURCE_BLUETOOTH_STYLUS = 0x00008000 | SOURCE_STYLUS,
       SOURCE_TRACKBALL = 0x00010000 | SOURCE_CLASS_TRACKBALL,
       SOURCE_MOUSE_RELATIVE = 0x00020000 | SOURCE_CLASS_TRACKBALL,
       SOURCE_TOUCHPAD = 0x00100000 | SOURCE_CLASS_POSITION,
       SOURCE_TOUCH_NAVIGATION = 0x00200000 | SOURCE_CLASS_NONE,
       SOURCE_ROTARY_ENCODER = 0x00400000 | SOURCE_CLASS_NONE,
       SOURCE_JOYSTICK = 0x01000000 | SOURCE_CLASS_JOYSTICK,
       SOURCE_HDMI = 0x02000000 | SOURCE_CLASS_BUTTON,
       SOURCE_ANY = 0xffffff00
   };

   virtual int getType()=0;
   void initialize(int32_t deviceId, int32_t source);
   void initialize(const InputEvent& from);
   void setSource(int source){mSource=source;}
   int getSource()const{return mSource;}
   bool isFromSource(int s)const{return mSource==s;}
   int getDeviceId(){return mDeviceId;}
   long getSequenceNumber()const{return mSeq;}
   virtual nsecs_t getEventTime() const { return mEventTime; }
   virtual void recycle();/*only obtained event can call recycle*/
};

class KeyEvent:public InputEvent{
private:
    int32_t mAction;
    int32_t mFlags;
    int32_t mKeyCode;
    int32_t mScanCode;
    int32_t mMetaState;
    int32_t mRepeatCount;
    nsecs_t mDownTime;//SystemClock#uptimeNanos
    static int metaStateFilterDirectionalModifiers(int metaState,int modifiers, int basic, int left, int right);
    static KeyEvent*obtain();
public:
    enum{
        ACTION_DOWN = 0,
        ACTION_UP = 1,
        ACTION_MULTIPLE = 2
    };
    enum{
        STATE_UNKNOWN = -1,
        STATE_UP = 0,
        STATE_DOWN = 1,
        STATE_VIRTUAL = 2
    };
   enum{
       META_SHIFT_ON      =0x01,
       META_SHIFT_LEFT_ON =0x40,
       META_SHIFT_RIGHT_ON=0x80,
       META_SHIFT_MASK    = META_SHIFT_ON | META_SHIFT_LEFT_ON | META_SHIFT_RIGHT_ON,

       META_ALT_ON      =0x02,
       META_ALT_LEFT_ON =0x10,
       META_ALT_RIGHT_ON=0x20,
       META_ALT_MASK    = META_ALT_ON | META_ALT_LEFT_ON | META_ALT_RIGHT_ON,

       META_SYM_ON      = 0x4,
       META_FUNCTION_ON =0x08,
       META_CAP_LOCKED = 0x100,
       META_ALT_LOCKED = 0x200,
       META_SYM_LOCKED = 0x400,
       META_SELECTING  = 0x800,

       META_CTRL_ON      =0x1000,
       META_CTRL_LEFT_ON =0x2000,
       META_CTRL_RIGHT_ON=0x4000,
       META_CTRL_MASK   =META_CTRL_ON | META_CTRL_LEFT_ON | META_CTRL_RIGHT_ON,

       META_META_ON      = 0x10000,
       META_META_LEFT_ON =0x20000,
       META_META_RIGHT_ON=0x40000,
       META_META_MASK    = META_META_ON | META_META_LEFT_ON | META_META_RIGHT_ON,

       META_CAPS_LOCK_ON = 0x100000,
       META_NUM_LOCK_ON  = 0x200000,
       META_SCROLL_LOCK_ON = 0x400000,
       META_LOCK_MASK =   META_CAPS_LOCK_ON | META_NUM_LOCK_ON | META_SCROLL_LOCK_ON,

       META_SYNTHETIC_MASK = META_CAP_LOCKED | META_ALT_LOCKED | META_SYM_LOCKED | META_SELECTING,
       META_INVALID_MODIFIER_MASK = META_LOCK_MASK | META_SYNTHETIC_MASK,

       META_MODIFIER_MASK =  META_SHIFT_ON | META_SHIFT_LEFT_ON | META_SHIFT_RIGHT_ON
            | META_ALT_ON | META_ALT_LEFT_ON | META_ALT_RIGHT_ON | META_CTRL_ON 
            | META_CTRL_LEFT_ON | META_CTRL_RIGHT_ON | META_META_ON | META_META_LEFT_ON 
            | META_META_RIGHT_ON | META_SYM_ON | META_FUNCTION_ON,
       META_ALL_MASK = META_MODIFIER_MASK | META_LOCK_MASK,
   };
   enum{
       FLAG_WOKE_HERE       = 0x1,
       FLAG_SOFT_KEYBOARD   = 0x2,
       FLAG_KEEP_TOUCH_MODE = 0x4,
       FLAG_FROM_SYSTEM     = 0x8,
       FLAG_EDITOR_ACTION   = 0x10,
       FLAG_CANCELED        = 0x20,
       FLAG_VIRTUAL_HARD_KEY= 0x40,
       FLAG_LONG_PRESS      = 0x80,
       FLAG_CANCELED_LONG_PRESS = 0x100,
       FLAG_TRACKING       = 0x200,
       FLAG_FALLBACK       = 0x400,
       FLAG_PREDISPATCH    = 0x20000000,
       FLAG_START_TRACKING = 0x40000000,
       FLAG_TAINTED        = 0x80000000,	   
   };
   class Callback{
   public:
       virtual bool onKeyDown(int keyCode, KeyEvent& event)=0;
       virtual bool onKeyLongPress(int keyCode, KeyEvent& event)=0;
       virtual bool onKeyUp(int keyCode, KeyEvent& event)=0;
       virtual bool onKeyMultiple(int keyCode, int count, KeyEvent& event)=0;
   };
   class DispatcherState{
   protected:
       int mDownKeyCode;
       void* mDownTarget;
       SparseIntArray  mActiveLongPresses;
   public:
       DispatcherState();
       void reset();
       void reset(void* target);
       void startTracking(KeyEvent& event,void* target);
       bool isTracking(KeyEvent& event);
       void performedLongPress(KeyEvent& event);
       void handleUpEvent(KeyEvent& event);
   };
public:
   void initialize(
            int32_t deviceId,
            int32_t source,
            int32_t action,
            int32_t flags,
            int32_t keyCode,
            int32_t scanCode,
            int32_t metaState,
            int32_t repeatCount,
            nsecs_t downTime,
            nsecs_t eventTime);
    void initialize(const KeyEvent& from);
    static KeyEvent* obtain(nsecs_t downTime, nsecs_t eventTime, int action,int code, int repeat, int metaState,
                   int deviceId, int scancode, int flags, int source/*,std::string characters*/);
    static KeyEvent* obtain(const KeyEvent& other);
    virtual int getType(){return EV_KEY;}
    int getKeyCode() {return mKeyCode;}
    void setKeyCode(int k){mKeyCode=k;}
    int getFlags(){return mFlags;}
    inline int32_t getScanCode() const { return mScanCode; }
    inline int32_t getMetaState() const { return mMetaState; } 
    int getAction(){return mAction;}//key up-->0 down-->1
    int getRepeatCount(){return mRepeatCount;}
    inline nsecs_t getDownTime() const { return mDownTime; }
    bool hasNoModifiers()const;
    bool hasModifiers(int modifiers)const;
    bool dispatch(Callback* caller,DispatcherState*state,void*target);
    bool isAltPressed () const{ return (mMetaState & META_ALT_ON) != 0;  }
    bool isShiftPressed()const{ return (mMetaState & META_SHIFT_ON) != 0;}
    bool isCtrlPressed() const{ return (mMetaState & META_CTRL_ON) != 0; }
    bool isMetaPressed() const{ return (mMetaState & META_META_ON) != 0; }
    bool isFunctionPressed()const{return (mMetaState & META_FUNCTION_ON) != 0; } 
    bool isCapsLockOn() const{  return (mMetaState & META_CAPS_LOCK_ON) != 0;  }
    bool isNumLockOn() const{   return (mMetaState & META_NUM_LOCK_ON) != 0;   }
    bool isScrollLockOn() const{return (mMetaState & META_SCROLL_LOCK_ON) != 0;}
    bool isCanceled()const { return (mFlags&FLAG_CANCELED) != 0; }
    void cancel() { mFlags |= FLAG_CANCELED;}
    void startTracking() { mFlags |= FLAG_START_TRACKING; }
    bool isTracking()const{ return (mFlags&FLAG_TRACKING) != 0;}
    bool isLongPress() { return (mFlags&FLAG_LONG_PRESS) != 0; }
    const char*getLabel(){return getLabel(mKeyCode);}
    static const char*getLabel(int key);
    static int getKeyCodeFromLabel(const char*label);
    static bool isModifierKey(int keyCode);
    static bool isConfirmKey(int keyCode);
    static int normalizeMetaState(int metaState);
    static bool metaStateHasNoModifiers(int metaState);
    static bool metaStateHasModifiers(int metaState, int modifiers);
    static const std::string metaStateToString(int metaState);
    static const std::string actionToString(int action);
};

class MotionEvent:public InputEvent{
public:
    enum{
        ACTION_MASK = 0xff,
        ACTION_POINTER_INDEX_MASK = 0xff00,
        ACTION_POINTER_INDEX_SHIFT=8, 
        ACTION_DOWN = 0,
        ACTION_UP = 1,
        ACTION_MOVE = 2,
        ACTION_CANCEL = 3,
        ACTION_OUTSIDE = 4,
        ACTION_POINTER_DOWN = 5,
        ACTION_POINTER_UP = 6,
        ACTION_HOVER_MOVE = 7,
        ACTION_SCROLL = 8,
        ACTION_HOVER_ENTER = 9,
        ACTION_HOVER_EXIT = 10,
        ACTION_BUTTON_PRESS = 11,
        ACTION_BUTTON_RELEASE = 12
    };
    /*Flag indicating the motion event intersected the top edge of the screen.*/
    enum{
        EDGE_TOP   = 0x0001,
        EDGE_BOTTOM= 0x0002,
        EDGE_LEFT  = 0x0004,
        EDGE_RIGHT = 0x0008
    };
    enum{
        AXIS_X = 0,
        AXIS_Y = 1,
        AXIS_PRESSURE = 2,
        AXIS_SIZE = 3,
        AXIS_TOUCH_MAJOR = 4,
        AXIS_TOUCH_MINOR = 5,
        AXIS_TOOL_MAJOR = 6,
        AXIS_TOOL_MINOR = 7,
        AXIS_ORIENTATION = 8,
        AXIS_VSCROLL = 9,
        AXIS_HSCROLL = 10,
        AXIS_Z = 11,
        AXIS_RX = 12,
        AXIS_RY = 13,
        AXIS_RZ = 14,
        AXIS_HAT_X = 15,
        AXIS_HAT_Y = 16,
        AXIS_LTRIGGER = 17,
        AXIS_RTRIGGER = 18,
        AXIS_THROTTLE = 19,
        AXIS_RUDDER = 20,
        AXIS_WHEEL = 21,
        AXIS_GAS = 22,
        AXIS_BRAKE = 23,
        AXIS_DISTANCE = 24,
        AXIS_TILT = 25,
        AXIS_SCROLL = 26,
        AXIS_RELATIVE_X = 27,
        AXIS_RELATIVE_Y = 28,
        AXIS_GENERIC_1 = 32,
        AXIS_GENERIC_2 = 33,
        AXIS_GENERIC_3 = 34,
        AXIS_GENERIC_4 = 35,
        AXIS_GENERIC_5 = 36,
        AXIS_GENERIC_6 = 37,
        AXIS_GENERIC_7 = 38,
        AXIS_GENERIC_8 = 39,
        AXIS_GENERIC_9 = 40,
        AXIS_GENERIC_10 = 41,
        AXIS_GENERIC_11 = 42,
        AXIS_GENERIC_12 = 43,
        AXIS_GENERIC_13 = 44,
        AXIS_GENERIC_14 = 45,
        AXIS_GENERIC_15 = 46,
        AXIS_GENERIC_16 = 47
    };
    enum{
        BUTTON_PRIMARY = 1 << 0,
        BUTTON_SECONDARY = 1 << 1,
        BUTTON_TERTIARY = 1 << 2,
        BUTTON_BACK = 1 << 3,
        BUTTON_FORWARD = 1 << 4,
        BUTTON_STYLUS_PRIMARY = 1 << 5,
        BUTTON_STYLUS_SECONDARY = 1 << 6
   };
   enum{
       FLAG_WINDOW_IS_OBSCURED =0x01,
       FLAG_WINDOW_IS_PARTIALLY_OBSCURED = 0x2,
       FLAG_HOVER_EXIT_PENDING = 0x4,
       FLAG_IS_GENERATED_GESTURE = 0x8,
       FLAG_TAINTED = 0x80000000,
       FLAG_TARGET_ACCESSIBILITY_FOCUS = 0x40000000,
   };
private:
   static const int HISTORY_CURRENT = -0x80000000;
   static MotionEvent*obtain();
protected:
   int32_t mAction;
   int32_t mActionButton;
   int32_t mFlags;
   int32_t mEdgeFlags;
   int32_t mMetaState;
   int32_t mButtonState;
   float mXOffset;
   float mYOffset;
   float mXPrecision;
   float mYPrecision;
   nsecs_t mDownTime;
   std::vector<PointerProperties> mPointerProperties;
   std::vector< nsecs_t > mSampleEventTimes;
   std::vector<PointerCoords> mSamplePointerCoords;
public:
   MotionEvent();
   MotionEvent(const MotionEvent&m);
   
   void initialize(int deviceId,int source,int action,int actionButton,
            int flags, int edgeFlags,int metaState,   int buttonState,
            float xOffset, float yOffset, float xPrecision, float yPrecision,
            nsecs_t downTime, nsecs_t eventTime, size_t pointerCount,
            const PointerProperties* pointerProperties,const PointerCoords* pointerCoords);
   
   static MotionEvent*obtain(nsecs_t downTime, nsecs_t eventTime, int action, 
            int pointerCount, const PointerProperties* pointerProperties,const PointerCoords* pointerCoords,
	    int metaState, int buttonState, float xPrecision, float yPrecision, int deviceId,
            int edgeFlags, int source, int flags);
   
   static MotionEvent* obtain(nsecs_t downTime, nsecs_t eventTime, int action,
            float x, float y, float pressure, float size, int metaState,
            float xPrecision, float yPrecision, int deviceId, int edgeFlags);

   static MotionEvent* obtain(nsecs_t downTime, nsecs_t eventTime, int action, float x, float y, int metaState);
   static MotionEvent* obtain(const MotionEvent& other);
   void copyFrom(const MotionEvent* other, bool keepHistory);
   MotionEvent*split(int idBits);
   virtual int getType(){return EV_ABS;}
   inline int32_t getAction() const { return mAction;}  
   inline void setAction(int32_t action) { mAction = action; }
   inline int32_t getActionMasked() const { return mAction &ACTION_MASK; }
   int getActionIndex()const{
      return (mAction & ACTION_POINTER_INDEX_MASK) >>  ACTION_POINTER_INDEX_SHIFT;
   }
   bool isTouchEvent()const;
   inline nsecs_t getDownTime()const{return mDownTime;}
   inline int32_t getFlags() const { return mFlags; }
   inline void setFlags(int32_t flags) { mFlags = flags; }
   inline int32_t getEdgeFlags() const { return mEdgeFlags; }
   inline void setEdgeFlags(int32_t edgeFlags) { mEdgeFlags = edgeFlags; }
   inline int32_t getMetaState() const { return mMetaState; }
   inline void setMetaState(int32_t metaState) { mMetaState = metaState; }
   inline int32_t getButtonState() const { return mButtonState; }
   inline void setButtonState(int32_t buttonState) { mButtonState = buttonState; }
   bool isButtonPressed(int button)const;
   inline int32_t getActionButton() const { return mActionButton; }
   inline void setActionButton(int32_t button) { mActionButton = button; }
   inline float getXOffset() const { return mXOffset; }
   inline float getYOffset() const { return mYOffset; }
   inline float getXPrecision() const { return mXPrecision; }
   inline float getYPrecision() const { return mYPrecision; }
   inline size_t getHistorySize() const { return mSampleEventTimes.size() - 1; }
   inline nsecs_t getHistoricalEventTime(size_t historicalIndex) const {
       return mSampleEventTimes[historicalIndex];
   }
   void getPointerCoords(int pointerIndex, PointerCoords* outPointerCoords){
       getHistoricalRawPointerCoords(pointerIndex,HISTORY_CURRENT,outPointerCoords);
   }
   void getHistoricalRawPointerCoords(size_t pointerIndex, size_t historicalIndex,PointerCoords* outPointerCoords) const;
   float getHistoricalRawAxisValue(int32_t axis, size_t pointerIndex,size_t historicalIndex) const;
   inline float getHistoricalRawX(size_t pointerIndex, size_t historicalIndex) const {
        return getHistoricalRawAxisValue(AXIS_X, pointerIndex, historicalIndex);
   }

   inline float getHistoricalRawY(size_t pointerIndex, size_t historicalIndex) const {
        return getHistoricalRawAxisValue(AXIS_Y, pointerIndex, historicalIndex);
   }

   inline size_t getPointerCount() const { return mPointerProperties.size(); }
   inline void getPointerProperties(size_t pointerIndex,PointerProperties*out) const {
        *out=mPointerProperties[pointerIndex];
   }
   void addSample(nsecs_t eventTime,const PointerProperties&, const PointerCoords&);
   void offsetLocation(float xOffset, float yOffset);
   void setLocation(float x,float y);
   void scale(float scaleFactor);

    // Apply 3x3 perspective matrix transformation.
    // Matrix is in row-major form and compatible with SkMatrix.
   void transform(const float matrix[9]);
   inline int32_t getPointerId(size_t pointerIndex) const {
        return mPointerProperties[pointerIndex].id;
   }
   int getPointerIdBits()const;
   inline int32_t getToolType(size_t pointerIndex) const {
        return mPointerProperties[pointerIndex].toolType;
   }
   inline nsecs_t getEventTime() const override{ return mSampleEventTimes[getHistorySize()]; }
   const PointerCoords* getRawPointerCoords(size_t pointerIndex) const;
   float getRawAxisValue(int32_t axis, size_t pointerIndex) const;
   inline float getRawX(size_t pointerIndex) const {
       return getRawAxisValue(ABS_X, pointerIndex);
   }

   inline float getRawY(size_t pointerIndex) const {
       return getRawAxisValue(ABS_Y, pointerIndex);
   }
   float getAxisValue(int32_t axis, size_t pointerIndex)const;
   float getX(size_t pointer=0)const{return getAxisValue(ABS_X,pointer);}
   float getY(size_t pointer=0)const{return getAxisValue(ABS_Y,pointer);}
   inline float getPressure(size_t pointerIndex) const {
       return getAxisValue(AXIS_PRESSURE, pointerIndex);
   }
   inline float getSize(size_t pointerIndex) const {
       return getAxisValue(AXIS_SIZE, pointerIndex);
   }
   inline float getTouchMajor(size_t pointerIndex) const {
       return getAxisValue(AXIS_TOUCH_MAJOR, pointerIndex);
   }
   inline float getTouchMinor(size_t pointerIndex) const {
       return getAxisValue(AXIS_TOUCH_MINOR, pointerIndex);
   }
   inline float getToolMajor(size_t pointerIndex) const {
       return getAxisValue(AXIS_TOOL_MAJOR, pointerIndex);
   }
   inline float getToolMinor(size_t pointerIndex) const {
       return getAxisValue(AXIS_TOOL_MINOR, pointerIndex);
   }
   inline float getOrientation(size_t pointerIndex) const {
       return getAxisValue(AXIS_ORIENTATION, pointerIndex);
   }
   ssize_t findPointerIndex(int32_t pointerId) const;
   inline bool isTargetAccessibilityFocus()const{
       return (mFlags & FLAG_TARGET_ACCESSIBILITY_FOCUS) != 0;
   }
   inline void setTargetAccessibilityFocus(bool targetsFocus){
       if(targetsFocus)
           mFlags|=FLAG_TARGET_ACCESSIBILITY_FOCUS;
       else
           mFlags&=~FLAG_TARGET_ACCESSIBILITY_FOCUS; 
   }
   static const std::string actionToString(int action);
};
/*
 * Input event factory.
 */
class InputEventFactoryInterface {
protected:
    virtual ~InputEventFactoryInterface() { }

public:
    InputEventFactoryInterface() { }

    virtual KeyEvent* createKeyEvent() = 0;
    virtual MotionEvent* createMotionEvent() = 0;
};

/* A simple input event factory implementation that uses a single preallocated instance
 * of each type of input event that are reused for each request.*/
class PreallocatedInputEventFactory : public InputEventFactoryInterface {
public:
    PreallocatedInputEventFactory() { }
    virtual ~PreallocatedInputEventFactory() { }

    virtual KeyEvent* createKeyEvent() { return & mKeyEvent; }
    virtual MotionEvent* createMotionEvent() { return & mMotionEvent; }

private:
    KeyEvent mKeyEvent;
    MotionEvent mMotionEvent;
};

/*
 * An input event factory implementation that maintains a pool of input events.
 */
class PooledInputEventFactory : public InputEventFactoryInterface {
public:
    PooledInputEventFactory(size_t maxPoolSize = 20);
    virtual ~PooledInputEventFactory();

    virtual KeyEvent* createKeyEvent();
    virtual MotionEvent* createMotionEvent();

    void recycle(InputEvent* event);
    static PooledInputEventFactory& getInstance();
private:
    const size_t mMaxPoolSize;
    static PooledInputEventFactory*mInst;
    std::queue<KeyEvent*> mKeyEventPool;
    std::queue<MotionEvent*> mMotionEventPool;
};
}
#endif
