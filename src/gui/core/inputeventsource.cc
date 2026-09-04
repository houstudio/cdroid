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
#include <core/inputeventsource.h>
#include <core/windowmanager.h>
#include <core/systemclock.h>
#include <porting/cdlog.h>
#include <unordered_map>
#include <gui_features.h>
#include <thread>
#include <chrono>
#include <mutex>
#include <pthread.h>
#if HAVE_SYS_PRCTL_H
#include <sys/prctl.h>
#elif HAVE_LINUX_PRCTL_H
#include <linux/prctl.h>
#endif

namespace cdroid{
InputEventSource::InputEventSource(){
    LOGD("InputEventSource %p",this);
    mScreenSaveTimeOut = -1;
    mRunning = false;
    mInited = false;
    mIsScreenSaveActived = false;
    mLastInputEventTime = SystemClock::uptimeMillis();
}

void InputEventSource::doEventsConsume(){
    INPUTEVENT es[128];
#if HAVE_PRCTL
    prctl(PR_SET_NAME,"InputThread",0,0,0);
#elif HAVE_PTHREAD_SETNAME_NP
    pthread_setname_np(pthread_self(), "InputThread");
#endif
    LOGI("InputEventSource'thread on %d",sched_getcpu());
    mRunning = true;
    InputInit();
    while(mRunning){
        const int count = InputGetEvents(es,sizeof(es)/sizeof(INPUTEVENT),20);
        std::lock_guard<std::recursive_mutex> lock(mtxEvents);
        if(count)mLastInputEventTime = SystemClock::uptimeMillis();
        for(int i = 0 ; i < count ; i ++){
            const INPUTEVENT*e = es+i;
            auto it = mDevices.find(e->device);
            if(es[i].type >= EV_ADD){
                onDeviceChanged(es+i);
                continue;
            }
            if(it==mDevices.end()){
                getDevice(es->device)->putEvent(e->tv_sec,e->tv_usec,e->type,e->code,e->value);
                continue;
            }
            it->second->putEvent(e->tv_sec,e->tv_usec,e->type,e->code,e->value);
        }
        if(count) Looper::getMainLooper()->wake();
    }
}

InputEventSource::~InputEventSource(){
    mRunning = false;
    // Join before the object dies: the input thread parks in InputGetEvents
    // with a 20ms timeout, so it observes the flag and exits within one poll;
    // after this, mtxEvents/mDevices outlive their only reader.
    if (mInputThread.joinable()) {
        mInputThread.join();
    }
    Looper::getMainLooper()->removeEventHandler(this);
    LOGD("%p Destroied",this);
}

void InputEventSource::onDeviceChanged(const INPUTEVENT*es){
    auto itr = mDevices.find(es->device);
    std::shared_ptr<InputDevice>dev = nullptr;
    switch(es->type){
    case EV_ADD:/*noting todo*/
        dev = getDevice(es->device);
        LOGI("device %s %d is added",(dev?dev->getName().c_str():""),es->device);
        break;
    case EV_REMOVE:
        if(itr!=mDevices.end())dev = itr->second;
        LOGI_IF(dev,"device %s:%d/%d is removed", dev->getName().c_str(), es->device,dev->getId());
        LOGI_IF(dev==nullptr,"remove unknwon dev %d",es->device);
        if(itr!=mDevices.end())mDevices.erase(itr);
        break;
    default:
        LOGI("dev %d unknown event %d",es->device,es->type);
        break;
    }
}

InputEventSource& InputEventSource::getInstance(){
    static InputEventSource* mInstance = nullptr;
    static std::once_flag flag;
    std::call_once(flag, []() {
        mInstance = new InputEventSource();
    });
    return *mInstance;
}

void InputEventSource::setScreenSaver(ScreenSaver func,int timeout){
    if(mScreenSaver)mScreenSaver(false);
    mScreenSaver = func;
    mScreenSaveTimeOut = timeout;
}

std::shared_ptr<InputDevice>InputEventSource::getDevice(int fd){
    std::shared_ptr<InputDevice>dev;
    auto itr = mDevices.find(fd);
    if(itr == mDevices.end()){
        InputDevice tmpdev(fd);
        if(tmpdev.getClasses()&(INPUT_DEVICE_CLASS_TOUCH|INPUT_DEVICE_CLASS_TOUCH_MT)){
            dev.reset(new TouchDevice(fd));
        }else if(tmpdev.getClasses()&INPUT_DEVICE_CLASS_CURSOR){
            dev.reset(new MouseDevice(fd));
        }else if(tmpdev.getClasses()&(INPUT_DEVICE_CLASS_KEYBOARD)){
            dev.reset(new KeyDevice(fd));
        }else if(tmpdev.getClasses()&(INPUT_DEVICE_CLASS_JOYSTICK|INPUT_DEVICE_CLASS_GAMEPAD)){
            LOGI("[%d]%s IS NOT SUPPORTED",fd,tmpdev.getName().c_str());
        }else {
            LOGI("[%d]%s IS NOT SUPPORTED",fd,tmpdev.getName().c_str());
            dev.reset(new InputDevice(fd));
        }
        mDevices.emplace(fd,dev);
        return dev;
    }
    return itr->second;
}

InputDevice* InputEventSource::getInputDevice(int id) {
    // Pure lookup (no auto-create, unlike getDevice) — KeyEvent resolves chars
    // from any thread, so take the recursive mutex ourselves.
    std::lock_guard<std::recursive_mutex> lock(mtxEvents);
    auto it = mDevices.find(id);
    return (it != mDevices.end()) ? it->second.get() : nullptr;
}

bool InputEventSource::needCancel(InputDevice*dev){
    int32_t action;
    nsecs_t etime;
    Point pos;
    const nsecs_t now = SystemClock::uptimeMillis();
    dev->getLastEvent(action,etime,&pos);
    TouchDevice*tdev= dynamic_cast<TouchDevice*>(dev);
    const int edges = tdev ? tdev->checkPointEdges(pos):0;
    if( (action == MotionEvent::ACTION_MOVE) && (now - etime>500) && (tdev != nullptr) && edges){
        MotionEvent*e = MotionEvent::obtain(now, now, MotionEvent::ACTION_CANCEL, 0, 0, 0);
        e->setSource(InputDevice::SOURCE_TOUCHSCREEN);
        dev->pushEvent(e);
    }
    return false;
}

void setThreadAffinity(std::thread& t, int coreId) {
    if (!t.joinable()) {
        LOGE("Cannot set affinity: thread is not joinable.");
    }
#if __linux__
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(coreId, &cpuset);

    const int rc = pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
        LOGW("Failed to set thread affinity");
    }
#endif
}

int InputEventSource::checkEvents(){
    if(!mInited){
        const auto numCore = std::thread::hardware_concurrency();
        auto coreId= sched_getcpu();
        auto func = std::bind(&InputEventSource::doEventsConsume,this);
        mInputThread = std::thread(func);
        if(numCore>1){
            setThreadAffinity(mInputThread,coreId-1>=0?coreId-1:coreId+1);
        }
        LOGI("MainLoop on %d/%d",coreId,numCore);
        mInited = true;
    }
    std::lock_guard<std::recursive_mutex> lock(mtxEvents);
    const nsecs_t now = SystemClock::uptimeMillis();
    int count = 0;
    count += (int)mInjectedEvents.size();
    for(auto item:mDevices){
        auto dev = item.second;
        const int devEvents= dev->getEventCount();
        count += devEvents;
        if((devEvents==0)&&(dev->getClasses()&(INPUT_DEVICE_CLASS_TOUCH|INPUT_DEVICE_CLASS_TOUCH_MT)))
            needCancel(dev.get());
    }
    if( ((now - mLastInputEventTime) > mScreenSaveTimeOut) && (mScreenSaveTimeOut>0)
            && ( mIsScreenSaveActived == false ) && mScreenSaver){
        mScreenSaver(true);
        mIsScreenSaveActived = true;
    }
    if(count && mIsScreenSaveActived){
        if(mScreenSaver)
            mScreenSaver(false);
        mIsScreenSaveActived= false;
        mLastInputEventTime = now;
    }
    return count;
}

void InputEventSource::openScreenSaver(){
    mIsScreenSaveActived = true;
    mLastInputEventTime = SystemClock::uptimeMillis();
}

void InputEventSource::closeScreenSaver(){
    if(mIsScreenSaveActived){
        mIsScreenSaveActived = false;
        if(mScreenSaver)mScreenSaver(false);
    }
    mLastInputEventTime = SystemClock::uptimeMillis();
}

bool InputEventSource::isScreenSaverActived()const{
    return mIsScreenSaveActived;
}

int InputEventSource::handleEvents(){
    int ret = 0;
    std::vector<InputEvent*>events;
    WindowManager& wm = WindowManager::getInstance();
    std::lock_guard<std::recursive_mutex> lock(mtxEvents);
    for(auto it:mDevices){
        const auto eventCount = it.second->drainEvents(events);
        if(eventCount==0) continue;
        ret += eventCount;
        std::for_each(events.begin(),events.end(),[&wm](InputEvent*e){
            wm.processEvent(*e);
            e->recycle();
        });
    }
    /*Injected events ride the same drain — process then recycle, exactly
      like device-queued ones (the InputDispatcher injection-entry analog).*/
    while(!mInjectedEvents.empty()){
        InputEvent*e = mInjectedEvents.front();
        mInjectedEvents.pop();
        wm.processEvent(*e);
        e->recycle();
        ret++;
    }
    return ret;
}

void InputEventSource::clearEvents(){
    // Signal the input thread to stop so it cannot keep filling queues while
    // we drain them (mRunning is its loop condition; see doEventsConsume).
    mRunning = false;
    std::vector<InputEvent*> events;
    std::lock_guard<std::recursive_mutex> lock(mtxEvents);
    for (auto& it : mDevices) {
        it.second->drainEvents(events);
    }
    // The injection queue too — InputEventSource is never destroyed, so
    // undelivered injected events would otherwise leak at exit.
    while (!mInjectedEvents.empty()) {
        events.push_back(mInjectedEvents.front());
        mInjectedEvents.pop();
    }
    for (InputEvent* e : events) {
        e->recycle();
    }
}

void InputEventSource::sendEvent(InputEvent&event){
    WindowManager::getInstance().processEvent(event);
}

bool InputEventSource::injectInputEvent(InputEvent&event,int mode){
    /*AOSP IInputManager.injectInputEvent queues an injection entry on the
      InputDispatcher and dispatches it through the normal channel path —
      an injected event is indistinguishable from a device event and never
      short-circuits into the window. Mirror that here: park a copy on the
      drain queue; handleEvents() delivers it on the main looper and recycles
      it. mode is kept for API fidelity — a blocking WAIT_* mode cannot work
      on the delivery thread itself, so every mode enqueues and returns.*/
    InputEvent* injected = event.copy();
    std::lock_guard<std::recursive_mutex> lock(mtxEvents);
    mInjectedEvents.push(injected);
    LOGV("injectInputEvent(mode=%d) queued", mode);
    return true;
}

int32_t InputEventSource::getGlobalMetaState()const{
    std::lock_guard<std::recursive_mutex> lock(mtxEvents);
    int32_t global = 0;
    for(const auto&item:mDevices){
        const KeyDevice*kd = dynamic_cast<const KeyDevice*>(item.second.get());
        if(kd) global |= kd->getMetaState();
    }
    return global;
}

}//end namespace

