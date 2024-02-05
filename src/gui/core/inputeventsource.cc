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
    auto func = std::bind(&InputEventSource::doEventsConsume,this);
    std::thread th(func);
    th.detach();
}

void InputEventSource::doEventsConsume(){
    INPUTEVENT es[128];
    while(1){
        if(mRunning == false){
            std::chrono::milliseconds dur(50);
            std::this_thread::sleep_for(dur);
            continue;
        }
        const int count = InputGetEvents(es,sizeof(es)/sizeof(INPUTEVENT),20);
        std::lock_guard<std::mutex> lock(mtxEvents);
        if(count)mLastInputEventTime = SystemClock::uptimeMillis();
        LOGV_IF(count,"rcv %d rawEvents",count);
        for(int i = 0 ; i < count ; i ++){
            const INPUTEVENT*e = es+i;
            struct timeval  tv = {(time_t)e->tv_sec,e->tv_usec};
            auto it = mDevices.find(e->device);
            if(es[i].type > EV_CNT){
                onDeviceChanged(es+i);
                continue;
            }
            if(it==mDevices.end()){
                getdevice(es->device)->putRawEvent(tv,e->type,e->code,e->value);
		continue;
	    }
	    it->second->putRawEvent(tv,e->type,e->code,e->value);
        }
    }
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
        getdevice(es->device);
        break;
    case EV_REMOVE:
        if(itr!=mDevices.end())dev = itr->second;
        LOGI_IF(dev,"device %s:%d/%d is removed", dev->getName().c_str(), es->device,dev->getId());
        LOGI_IF(dev==nullptr,"remove unknwon dev %s:%d",dev->getName().c_str(), es->device);
        if(itr!=mDevices.end())mDevices.erase(itr);
	break;
    default:LOGI("dev %d unknown event %d",es->device,es->type);break;
    }
}

static NeverDestroyed<InputEventSource>mInst;
InputEventSource& InputEventSource::getInstance(){
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
        }else if(tmpdev.getClasses()&(INPUT_DEVICE_CLASS_KEYBOARD)){
            dev.reset(new KeyDevice(fd));
        }
        mDevices.emplace(fd,dev);
        return dev;
    }
    return itr->second;
}

int InputEventSource::checkEvents(){
    std::lock_guard<std::mutex> lock(mtxEvents);
    nsecs_t now = SystemClock::uptimeMillis();
	int count = 0;
	for(auto dev:mDevices){
        count += dev.second->getEventCount();
	}
    if(mRunning==false)
        mRunning = true;
    if( ((now - mLastInputEventTime) > mScreenSaveTimeOut) && (mScreenSaveTimeOut>0)
            && ( mIsScreenSaveActived == false ) && mScreenSaver){
        mScreenSaver(true);
        mIsScreenSaveActived = true;
    }
    if(count && mIsScreenSaveActived && mScreenSaver){
        mScreenSaver(false);
        mLastInputEventTime = now;
    }
    return count;
}

void InputEventSource::closeScreenSaver(){
    mIsScreenSaveActived = false;
}

bool InputEventSource::isScreenSaverActived()const{
    return mIsScreenSaveActived;
}

int InputEventSource::handleEvents(){
    std::lock_guard<std::mutex> lock(mtxEvents);
    int ret = 0;
	for(auto it:mDevices){
	   auto dev = it.second;
	   if(dev->getEventCount()==0)continue;
	   ret += dev->getEventCount();
	   if(dev->getClasses()&(INPUT_DEVICE_CLASS_TOUCH|INPUT_DEVICE_CLASS_TOUCH_MT)){
	       while(dev->getEventCount()){
	          MotionEvent*e = (MotionEvent*)dev->popEvent();
	          WindowManager::getInstance().processEvent(*e);
			  e->recycle();
		   }
	   }else if(dev->getClasses()&INPUT_DEVICE_CLASS_KEYBOARD){
	       while(dev->getEventCount()){
	          KeyEvent*e = (KeyEvent*)dev->popEvent();
	          WindowManager::getInstance().processEvent(*e);
			  e->recycle();
		   }	       
	   }
	}
    return ret;
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
                       0/*deviceid*/,keycode/*scancode*/,0/*flags*/,0/*source*/,0/*displayId*/);
                 //mKeyEvents.push_back(key);
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

