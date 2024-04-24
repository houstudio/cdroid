#ifndef __INPUT_EVENT_H__
#define __INPUT_EVENT_H__
#include <stdint.h>
#include <string>
#include <vector>
#include <queue>
typedef __int64_t nsecs_t;
namespace cdroid{

class InputEvent{
protected:
    static constexpr long NS_PER_MS = 1000000;
    int mDeviceId;
    int mDisplayId;
    int mSource;
    long mSeq;
    nsecs_t mEventTime;//SystemClock#uptimeMillis
public:
    enum{
        INPUT_EVENT_TYPE_KEY = 1,
        INPUT_EVENT_TYPE_MOTION = 2,
        INPUT_EVENT_TYPE_FOCUS = 3
    };
    InputEvent();
    virtual ~InputEvent();
    int getDeviceId()const{return mDeviceId;}
    int getDisplayId()const;
    void setDisplayId(int);
    virtual int getType()const=0;
    virtual InputEvent*copy()const=0;
    void initialize(int32_t deviceId, int32_t source);
    void initialize(const InputEvent& from);
    void setSource(int source){mSource=source;}
    int getSource()const{return mSource;}
    bool isFromSource(int s)const;
    long getSequenceNumber()const{return mSeq;}
    virtual bool isTainted()const=0;
    virtual void setTainted(bool)=0;
    virtual nsecs_t getEventTimeNanos() const { return mEventTime*NS_PER_MS; }
    virtual nsecs_t getEventTime()const{ return mEventTime;}
    virtual void recycle();/*only obtained event can call recycle*/
    virtual void toStream(std::ostream& os)const=0;
    friend std::ostream& operator<<( std::ostream&,const InputEvent&);
};

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
    static PooledInputEventFactory*mInst;
    std::queue<KeyEvent*> mKeyEventPool;
    std::queue<MotionEvent*> mMotionEventPool;
};

}/*endof namespace*/
#endif/*__INPUT_EVENT_H__*/
