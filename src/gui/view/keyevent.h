#ifndef __KEY_EVENT_H__
#define __KEY_EVENT_H__
#include <view/inputevent.h>
#include <core/eventcodes.h>
#include <core/sparsearray.h>
namespace cdroid{

class KeyEvent:public InputEvent{
private:
    int32_t mAction;
    int32_t mFlags;
    int32_t mKeyCode;
    int32_t mScanCode;
    int32_t mMetaState;
    int32_t mRepeatCount;
    nsecs_t mDownTime;//SystemClock#uptimeMillis
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
               int deviceId, int scancode, int flags, int source,int displayId=0/*,std::string characters*/);
    static KeyEvent* obtain(const KeyEvent& other);
    virtual int getType()const {return INPUT_EVENT_TYPE_KEY;}
    KeyEvent*copy()const override{return obtain(*this);}
    int getKeyCode()const {return mKeyCode;}
    void setKeyCode(int k){mKeyCode=k;}
    int getFlags()const{return mFlags;}
    inline bool isTainted()const{return (mFlags&FLAG_TAINTED)!=0;}
    inline void setTainted(bool tainted){
        if(tainted)mFlags|=FLAG_TAINTED;
        else mFlags&=~FLAG_TAINTED;
    }
    inline int32_t getScanCode() const { return mScanCode; }
    inline int32_t getMetaState() const { return mMetaState; }
    int getAction()const{return mAction;}//key up-->0 down-->1
    int getRepeatCount()const {return mRepeatCount;}
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
    void toStream(std::ostream& os)const override;
};
}/*endof namespace*/

#endif/*__KEY_EVENT_H__*/
