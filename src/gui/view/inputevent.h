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
#ifndef __INPUT_EVENT_H__
#define __INPUT_EVENT_H__
#include <stdint.h>
#include <string>
#include <vector>
#include <queue>
typedef int64_t nsecs_t;
namespace cdroid{
/*
 * Flags that flow alongside events in the input dispatch system to help with certain
 * policy decisions such as waking from device sleep.
 *
 * These flags are also defined in frameworks/base/core/java/android/view/WindowManagerPolicy.java.
 */
enum {
    /* These flags originate in RawEvents and are generally set in the key map.
     * NOTE: If you want a flag to be able to set in a keylayout file, then you must add it to
     * InputEventLabels.h as well. */

    // Indicates that the event should wake the device.
    POLICY_FLAG_WAKE = 0x00000001,

    // Indicates that the key is virtual, such as a capacitive button, and should
    // generate haptic feedback.  Virtual keys may be suppressed for some time
    // after a recent touch to prevent accidental activation of virtual keys adjacent
    // to the touch screen during an edge swipe.
    POLICY_FLAG_VIRTUAL = 0x00000002,

    // Indicates that the key is the special function modifier.
    POLICY_FLAG_FUNCTION = 0x00000004,

    // Indicates that the key represents a special gesture that has been detected by
    // the touch firmware or driver.  Causes touch events from the same device to be canceled.
    POLICY_FLAG_GESTURE = 0x00000008,

    POLICY_FLAG_RAW_MASK = 0x0000ffff,

    POLICY_FLAG_INJECTED_FROM_ACCESSIBILITY = 0x20000,

    /* These flags are set by the input dispatcher. */

    // Indicates that the input event was injected.
    POLICY_FLAG_INJECTED = 0x01000000,

    // Indicates that the input event is from a trusted source such as a directly attached
    // input device or an application with system-wide event injection permission.
    POLICY_FLAG_TRUSTED = 0x02000000,

    // Indicates that the input event has passed through an input filter.
    POLICY_FLAG_FILTERED = 0x04000000,

    // Disables automatic key repeating behavior.
    POLICY_FLAG_DISABLE_KEY_REPEAT = 0x08000000,

    /* These flags are set by the input reader policy as it intercepts each event. */

    // Indicates that the device was in an interactive state when the
    // event was intercepted.
    POLICY_FLAG_INTERACTIVE = 0x20000000,

    // Indicates that the event should be dispatched to applications.
    // The input event should still be sent to the InputDispatcher so that it can see all
    // input events received include those that it will not deliver.
    POLICY_FLAG_PASS_TO_USER = 0x40000000,
};

class InputEvent{
protected:
    static constexpr long NS_PER_MS = 1000000;
    int32_t mDeviceId;
    int32_t mDisplayId;
    int32_t mSource;
    int32_t mId;
    int32_t mAction;
    long mSeq;
    static int mNextSeq;
    nsecs_t mEventTime;
protected:
    void prepareForReuse();
public:
    enum InputEventType{
        INPUT_EVENT_TYPE_KEY = 1,
        INPUT_EVENT_TYPE_MOTION = 2,
        INPUT_EVENT_TYPE_FOCUS = 3,
        INPUT_EVENT_TYPE_CAPTURE =4,
        INPUT_EVENT_TYPE_DRAG=5,
        INPUT_EVENT_TYPE_TOUCH_MODE=6
    };
    InputEvent();
    virtual ~InputEvent();
    int getAction()const{return mAction;}
    int getDeviceId()const{return mDeviceId;}
    int getId()const{return mId;}
    int getDisplayId()const;
    void setDisplayId(int);
    virtual int getType()const=0;
    virtual InputEvent*copy()const=0;
    void initialize(int32_t id,int32_t deviceId, int32_t source,int32_t displayId);
    void initialize(const InputEvent& from);
    int32_t getSource()const{return mSource;}
    bool isFromSource(int32_t s)const;
    long getSequenceNumber()const{return mSeq;}
    virtual void setSource(int32_t source);
    virtual bool isTainted()const=0;
    virtual void setTainted(bool)=0;
    virtual nsecs_t getEventTimeNanos() const { return mEventTime*NS_PER_MS; }
    virtual nsecs_t getEventTime()const{ return mEventTime;}
    virtual void recycle();/*only obtained event can call recycle*/
    static int32_t nextId();
    static std::string sourceToString(int32_t source);
};

std::ostream& operator<<(std::ostream& out, const InputEvent& event);

class KeyEvent;
class MotionEvent;
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
    std::queue<KeyEvent*> mKeyEventPool;
    std::queue<MotionEvent*> mMotionEventPool;
};

class IdGenerator {
private:
    static constexpr uint32_t SOURCE_SHIFT = 30;

public:
    // Used to divide integer space to ensure no conflict among these sources./
    enum class Source : int32_t {
        INPUT_READER = static_cast<int32_t>(0x0u << SOURCE_SHIFT),
        INPUT_DISPATCHER = static_cast<int32_t>(0x1u << SOURCE_SHIFT),
        OTHER = static_cast<int32_t>(0x3u << SOURCE_SHIFT), // E.g. app injected events
    };
    IdGenerator(Source source);

    int32_t nextId() const;

    // Extract source from given id.
    static inline Source getSource(int32_t id) { return static_cast<Source>(SOURCE_MASK & id); }

private:
    const Source mSource;

    static constexpr int32_t SOURCE_MASK = static_cast<int32_t>(0x3u << SOURCE_SHIFT);
};

}/*endof namespace*/
#endif/*__INPUT_EVENT_H__*/
