#include <view/inputevent.h>
#include <view/keyevent.h>
#include <view/motionevent.h>
#include <core/inputdevice.h>
// --- InputEvent ---
namespace cdroid{

InputEvent::InputEvent(){
   mSource = InputDevice::SOURCE_UNKNOWN;
   mDisplayId = 0;
}

InputEvent::~InputEvent(){
}

int InputEvent::getDisplayId()const{
    return mDisplayId;
}

void InputEvent::setDisplayId(int id){
    mDisplayId = id;
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

////////////////////////////////////////////////////////////////////////////////////////////////////

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
    case InputEvent::INPUT_EVENT_TYPE_KEY:
        if (mKeyEventPool.size() < mMaxPoolSize) {
            mKeyEventPool.push(static_cast<KeyEvent*>(event));
            return;
        }
        break;
    case InputEvent::INPUT_EVENT_TYPE_MOTION:
        if (mMotionEventPool.size() < mMaxPoolSize) {
            mMotionEventPool.push(static_cast<MotionEvent*>(event));
            return;
        }
        break;
    }
    delete event;
}

}/*endof namespace*/
