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
#include <view/inputevent.h>
#include <view/keyevent.h>
#include <view/motionevent.h>
#include <core/inputdevice.h>
#include <utils/atexit.h>
#include <utils/neverdestroyed.h>
#include <porting/cdlog.h>
// --- InputEvent ---
namespace cdroid{

int InputEvent::mNextSeq=0;

InputEvent::InputEvent(){
   mSource = InputDevice::SOURCE_UNKNOWN;
   mDisplayId = 0;
   mSeq = mNextSeq++;
}

InputEvent::~InputEvent(){
}

void InputEvent::prepareForReuse(){
   mSeq = mNextSeq++;
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

void InputEvent::setSource(int source){
    mSource = source;
}

bool InputEvent::isFromSource(int source)const{
    return (getSource() & source) == source;
}

void InputEvent::initialize(const InputEvent& from) {
    mDeviceId = from.mDeviceId;
    mSource = from.mSource;
    mId = from.mId;
}

void InputEvent::recycle(){
    PooledInputEventFactory::getInstance().recycle(this);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

// --- PooledInputEventFactory ---
static NeverDestroyed<PooledInputEventFactory>mInst(20);
PooledInputEventFactory& PooledInputEventFactory::getInstance(){
    return *mInst.get();
}

PooledInputEventFactory::PooledInputEventFactory(size_t maxPoolSize)
    :mMaxPoolSize(maxPoolSize) {
}

PooledInputEventFactory::~PooledInputEventFactory() {
    while(!mKeyEventPool.empty()) {
        delete mKeyEventPool.front();
        mKeyEventPool.pop();
    }
    while(!mMotionEventPool.empty()) {
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
        LOGD("outOf KeyPool %d/%d",mKeyEventPool.size(),mMaxPoolSize);
        break;
    case InputEvent::INPUT_EVENT_TYPE_MOTION:
        if (mMotionEventPool.size() < mMaxPoolSize) {
            mMotionEventPool.push(static_cast<MotionEvent*>(event));
            return;
        }
        LOGD("outOf MotionPool %d/%d",mMotionEventPool.size(),mMaxPoolSize);
        break;
    }
    delete event;
}

}/*endof namespace*/
