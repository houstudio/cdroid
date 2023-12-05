#include <inputeventsource.h>
#include <uievents.h>
#include <cdlog.h>
#include <unordered_map>
#include <windowmanager.h>
#include <uievents.h>
#include <thread>
#include <chrono>
#include <tokenizer.h>
#include <systemclock.h>

namespace cdroid{


InputEventSource::InputEventSource(){
    InputInit();
    mScreenSaveTimeOut = -1;
    mRunning = false;
    mIsPlayback = false;
    mIsScreenSaveActived = false;
    mLastPlaybackEventTime = SystemClock::uptimeMillis();
    mLastInputEventTime = mLastPlaybackEventTime;
    auto func=[this](){
        INPUTEVENT es[128];
        while(1){
            if(mRunning==false){
                std::chrono::milliseconds dur(50);
                std::this_thread::sleep_for(dur);
                continue;
            }
            const int count = InputGetEvents(es,sizeof(es)/sizeof(INPUTEVENT),20);
            std::lock_guard<std::mutex> lock(mtxEvents);
            if(count)mLastInputEventTime = SystemClock::uptimeMillis();
            LOGV_IF(count,"rcv %d rawEvents",count);
            for(int i = 0 ; i < count ; i ++){
                if(es[i].type > EV_CNT){
                    onDeviceChanged(es+i);
                    continue;
                }
                mRawEvents.push(es[i]);
            }
        }
    };
    std::thread th(func);
    th.detach();
}

InputEventSource::~InputEventSource(){
    if(frecord.is_open())
       frecord.close();
    LOGD("%p Destroied",this);
}

void InputEventSource::onDeviceChanged(const INPUTEVENT*es){
    auto itr = mDevices.find(es->device);
    std::shared_ptr<InputDevice>dev = nullptr;
    switch(es->type){
    case EV_ADD:
        /*noting todo*/
        LOGI("device %d is added",es->device);
        for(auto d:mDevices)LOGI("device %d %s",d.first,d.second->getName().c_str());
        getdevice(es->device);
        for(auto d:mDevices)LOGI("::device %d classes=%x %s",d.first,d.second->getClasses(),d.second->getName().c_str());
        break;
    case EV_REMOVE:
        if(itr!=mDevices.end())dev = itr->second;
        LOGI("device %s:%d/%d is removed", dev->getName().c_str(), es->device,dev->getId());
        mDevices.erase(itr);
	break;
    }
}

InputEventSource& InputEventSource::getInstance(){
    static InputEventSource* mInst = nullptr;
    if(mInst == nullptr)mInst = new InputEventSource();
    return *mInst;
}

void InputEventSource::setScreenSaver(ScreenSaver func,int timeout){
    mScreenSaver = func;
    mScreenSaveTimeOut = timeout;
}

std::shared_ptr<InputDevice>InputEventSource::getdevice(int fd){
    std::shared_ptr<InputDevice>dev;
    auto itr = mDevices.find(fd);
    if(itr == mDevices.end()){
        InputDevice tmpdev(fd);
        if(tmpdev.getClasses()&(INPUT_DEVICE_CLASS_TOUCH|INPUT_DEVICE_CLASS_TOUCH_MT)){
            dev.reset(new MouseDevice(fd));
            dev->setEventConsumeListener([&](const InputEvent&e){
               MotionEvent*mt=MotionEvent::obtain((MotionEvent&)e);
               mMotionEvents.push(mt);
            });
        }else if(tmpdev.getClasses()&(INPUT_DEVICE_CLASS_KEYBOARD)){
            dev.reset(new KeyDevice(fd));
            dev->setEventConsumeListener([&](const InputEvent&e){
               KeyEvent*key=KeyEvent::obtain((KeyEvent&)e);
               mKeyEvents.push_back(key);
            });
        }
        mDevices.emplace(fd,dev);
        return dev;
    }
    return itr->second;
}

int InputEventSource::checkEvents(){
    std::lock_guard<std::mutex> lock(mtxEvents);
    nsecs_t now = SystemClock::uptimeMillis();
    if(mRunning==false)
        mRunning = true;
    if( ((now - mLastInputEventTime) > mScreenSaveTimeOut) && (mScreenSaveTimeOut>0)
            && ( mIsScreenSaveActived == false ) && mScreenSaver){
        mScreenSaver(true);
        mIsScreenSaveActived = true;
    }
    process();
    if(mMotionEvents.size() && mIsScreenSaveActived && mScreenSaver){
        mScreenSaver(false);
        mLastInputEventTime = now;
    }
    return mMotionEvents.size()+mKeyEvents.size();
}

void InputEventSource::closeScreenSaver(){
    mIsScreenSaveActived = false;
}

bool InputEventSource::isScreenSaverActived()const{
    return mIsScreenSaveActived;
}

int InputEventSource::handleEvents(){
    std::lock_guard<std::mutex> lock(mtxEvents);
    int ret = mKeyEvents.size() + mMotionEvents.size();
    while(mKeyEvents.size()){
        int lastDev = -1,lastKey = -1;
        int numDown = 0 ,numUp = 0;
        for(KeyEvent*e:mKeyEvents){
            const int dev = e->getDeviceId();
            const int key = e->getKeyCode();
            const int act = e->getAction();
	    LOGV("dev=%d key=%d act=%d %d",dev,key,act,mKeyEvents.size());
            if( (lastKey !=-1) || (lastKey==key) ){
                numDown+= (act==KeyEvent::ACTION_DOWN);
                numUp  += (act==KeyEvent::ACTION_UP);
                lastKey = dev;  lastKey = key;
            }else{
                numDown = numUp =0;
                lastKey = -1;   lastDev = -1;
                break;
            }
        }
        if((numDown>1)&&(numUp>1)){
            int repeat = std::min(numDown,numUp);
            KeyEvent*e = mKeyEvents.front();
            KeyEvent*kMulti = KeyEvent::obtain(e->getDownTime(),e->getEventTime(),KeyEvent::ACTION_MULTIPLE,
            e->getKeyCode(),repeat,e->getMetaState(),e->getDeviceId(),
            e->getScanCode(),e->getFlags(),e->getSource());
            LOGD("key %d repeat %d",e->getKeyCode(),repeat);
            WindowManager::getInstance().processEvent(*kMulti);
            kMulti->recycle();
            repeat<<=1;
            for(int i=0;i<repeat;i++){
                e = mKeyEvents.front();
                e->recycle();
                mKeyEvents.erase(mKeyEvents.begin());
            }
            continue;
        }
        KeyEvent*e1 = mKeyEvents.front();
        WindowManager::getInstance().processEvent(*e1);
        e1->recycle();
        LOGV("key %d  repeat=%d/%d",e1->getKeyCode(),numDown,numUp);
        mKeyEvents.erase(mKeyEvents.begin());
    }
    while(mMotionEvents.size()){
        MotionEvent* e = mMotionEvents.front();
        WindowManager::getInstance().processEvent(*e);
        if((!mIsPlayback)&& frecord.is_open() && dynamic_cast<KeyEvent*>(e) ){
            nsecs_t eventTime=SystemClock::uptimeMillis();
            KeyEvent*key=dynamic_cast<KeyEvent*>(e);
            frecord<<"delay("<<eventTime - mLastPlaybackEventTime<<")"<<std::endl;
            frecord<<"key("<<KeyEvent::actionToString(key->getAction())<<","
                  <<KeyEvent::getLabel(key->getKeyCode())<<")"<<std::endl;
            mLastPlaybackEventTime = eventTime;
        }
        e->recycle();
        mMotionEvents.pop();
    }
    return ret;
}

int  InputEventSource::process(){
    LOGV_IF(mRawEvents.size(),"%p  recv %d events ",this,mRawEvents.size());
    while(mRawEvents.size()){
        const INPUTEVENT e = mRawEvents.front();
        struct timeval  tv = {(time_t)e.tv_sec,e.tv_usec};
        std::shared_ptr<InputDevice>dev = getdevice(e.device);
        mRawEvents.pop();
        if(dev == nullptr){
            LOGD("%d,%d,%d device=%d ",e.type,e.code,e.value,e.device);
            continue;
        }
        dev->putRawEvent(tv,e.type,e.code,e.value);
    }
    return 0;
}

int InputEventSource::pushEvent(InputEvent*evt){
    std::lock_guard<std::mutex> lock(mtxEvents);
    if(evt->getType()==InputEvent::EVENT_TYPE_MOTION){
        mMotionEvents.push((MotionEvent*)evt);
    }else if(evt->getType()==InputEvent::EVENT_TYPE_KEY){
        mKeyEvents.push_back((KeyEvent*)evt);
    }
    return mMotionEvents.size() + mKeyEvents.size();
}

void InputEventSource::playback(const std::string&fname){
    #define DELIMITERS "(),"
    auto func = [this](const std::string&fname){
         std::fstream in(fname);
         std::this_thread::sleep_for(std::chrono::milliseconds(10000));
         LOGD_IF(in.good(),"play key from %s",fname.c_str());
         if(!in.good())return ;
         mIsPlayback = true;
         while(1){
             char *ps= nullptr;
             char line[256] = { 0 };
             Tokenizer *tok;
             in.getline(line,255);
             Tokenizer::fromContents("",line,&tok);

             std::string word = tok->nextToken(DELIMITERS);
             tok->skipDelimiters(DELIMITERS);
             if(word.compare("delay")==0){
                 word = tok->nextToken(DELIMITERS);
                 std::chrono::milliseconds dur(strtoul(word.c_str(),&ps,10));
                 std::this_thread::sleep_for(dur);
             }else if(word.compare("key")==0){
                 word = tok->nextToken(DELIMITERS);
                 int action = (word.find("DOWN")!=std::string::npos)?KeyEvent::ACTION_DOWN:KeyEvent::ACTION_UP;

                 tok->skipDelimiters(DELIMITERS);
                 word = tok->nextToken(DELIMITERS);
                 nsecs_t evttime = SystemClock::uptimeMillis();
                 int keycode = KeyEvent::getKeyCodeFromLabel(word.c_str());
                 KeyEvent*key= KeyEvent::obtain(evttime,evttime,action,keycode,1,0/*metastate*/,
                       0/*deviceid*/,keycode/*scancode*/,0/*flags*/,0/*source*/);
                 mKeyEvents.push_back(key);
             }
             if(in.gcount() == 0){
                 in.close();
                 in.open(fname);
             }
             delete tok;
         }
    };
    std::thread th(func,fname);
    th.detach();
}
}//end namespace

