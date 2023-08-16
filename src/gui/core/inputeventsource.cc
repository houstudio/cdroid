#include <inputeventsource.h>
#include <uievents.h>
#include <cdlog.h>
#include <unordered_map>
#include <windowmanager.h>
#include <uievents.h>
#include <thread>
#include <chrono>
#include<tokenizer.h>
#include <systemclock.h>

namespace cdroid{


InputEventSource::InputEventSource(){
    InputInit();
    mScreenSaveTimeOut = -1;
    mIsPlayback = false;
    mIsScreenSaveActived = false;
    mLastEventTime=SystemClock::uptimeMillis();
    auto func=[this](){
        while(1){
            INPUTEVENT es[32];
            int count=InputGetEvents(es,32,10);
            std::lock_guard<std::mutex> lock(mtxEvents);
            LOGV_IF(count,"rcv %d rawEvents",count);
            for(int i=0;i<count;i++)
                mRawEvents.push(es[i]);
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

InputEventSource& InputEventSource::getInstance(){
    static InputEventSource* mInst = nullptr;
    if(mInst == nullptr)mInst=new InputEventSource();
    return *mInst;
}

void InputEventSource::setScreenSaver(ScreenSaver func,int timeout){
    mScreenSaver = func;
    mScreenSaveTimeOut = timeout;
}

std::shared_ptr<InputDevice>InputEventSource::getdevice(int fd){
    std::shared_ptr<InputDevice>dev;
    auto itr=devices.find(fd);
    if(itr==devices.end()){
        InputDevice tmpdev(fd);LOGD("device %d classes=%x",fd,tmpdev.getClasses());
        if(tmpdev.getClasses()&(INPUT_DEVICE_CLASS_TOUCH|INPUT_DEVICE_CLASS_TOUCH_MT)){
            dev.reset(new MouseDevice(fd));
            dev->setEventConsumeListener([&](const InputEvent&e){
               MotionEvent*mt=MotionEvent::obtain((MotionEvent&)e);
               mInputEvents.push(mt);
            });
        }else if(tmpdev.getClasses()&(INPUT_DEVICE_CLASS_KEYBOARD)){
            dev.reset(new KeyDevice(fd));
            dev->setEventConsumeListener([&](const InputEvent&e){
               KeyEvent*key=KeyEvent::obtain((KeyEvent&)e);
               mInputEvents.push(key);
            });
        }
        devices.emplace(fd,dev);
        return dev;
    }
    return itr->second;
}

int InputEventSource::checkEvents(){
    std::lock_guard<std::mutex> lock(mtxEvents);
    nsecs_t now = SystemClock::uptimeMillis();
    if( (now - mLastEventTime) > mScreenSaveTimeOut && (mScreenSaveTimeOut>0) && mScreenSaver){
	mScreenSaver(true);
	mIsScreenSaveActived = true;
    }
    process();
    if(mInputEvents.size()&& mIsScreenSaveActived){
	mIsScreenSaveActived = false;
	mScreenSaver(false);
    }
    return mInputEvents.size()>0;
}

int InputEventSource::handleEvents(){
    std::lock_guard<std::mutex> lock(mtxEvents);
    if(mInputEvents.size()==0)return false;
    while(mInputEvents.size()){
        InputEvent* e=mInputEvents.front();
        WindowManager::getInstance().processEvent(*e);
        if((!mIsPlayback)&& frecord.is_open() && dynamic_cast<KeyEvent*>(e) ){
            nsecs_t eventTime=SystemClock::uptimeMillis();
            KeyEvent*key=dynamic_cast<KeyEvent*>(e);
            frecord<<"delay("<<eventTime-mLastEventTime<<")"<<std::endl;
            frecord<<"key("<<KeyEvent::actionToString(key->getAction())<<","
                  <<KeyEvent::getLabel(key->getKeyCode())<<")"<<std::endl;
            mLastEventTime=eventTime;
        }
        e->recycle();
        mInputEvents.pop();
    }
    return 0; 
}

int  InputEventSource::process(){
    LOGV_IF(mRawEvents.size(),"%p  recv %d events ",this,mRawEvents.size());
    while(mRawEvents.size()){
        const INPUTEVENT e=mRawEvents.front();
        struct timeval tv={(time_t)e.tv_sec,e.tv_usec};
        std::shared_ptr<InputDevice>dev=getdevice(e.device);
        mRawEvents.pop();
        if(dev==nullptr){
            LOGD("%d,%d,%d device=%d ",e.type,e.code,e.value,e.device);
            continue;
        }
        dev->putRawEvent(tv,e.type,e.code,e.value);
    }
    return 0;
}

int InputEventSource::pushEvent(InputEvent*evt){
    std::lock_guard<std::mutex> lock(mtxEvents);
    mInputEvents.push(evt);
    return mInputEvents.size();
}

void InputEventSource::playback(const std::string&fname){
    #define DELIMITERS "(),"
    auto func=[this](const std::string&fname){
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

             std::string word=tok->nextToken(DELIMITERS);  
             tok->skipDelimiters(DELIMITERS);
             if(word.compare("delay")==0){
                 word=tok->nextToken(DELIMITERS);
                 std::chrono::milliseconds dur(strtoul(word.c_str(),&ps,10));
                 std::this_thread::sleep_for(dur);
             }else if(word.compare("key")==0){
                 word =tok->nextToken(DELIMITERS);  
                 int action =(word.find("DOWN")!=std::string::npos)?KeyEvent::ACTION_DOWN:KeyEvent::ACTION_UP;

                 tok->skipDelimiters(DELIMITERS);
                 word =tok->nextToken(DELIMITERS);
                 nsecs_t evttime=SystemClock::uptimeMillis();  
                 int keycode=KeyEvent::getKeyCodeFromLabel(word.c_str());
                 KeyEvent*key=KeyEvent::obtain(evttime,evttime,action,keycode,1,0/*metastate*/,
                       0/*deviceid*/,keycode/*scancode*/,0/*flags*/,0/*source*/);
                 mInputEvents.push(key);
             }
             if(in.gcount()==0){
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

