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
#if ENABLED_GESTURE
#include <GRT/GRT.h>
#endif


namespace cdroid{


InputEventSource::InputEventSource(const std::string&file){
    InputInit();
    frecord.open(file);
    isplayback=false;
    lasteventTime=SystemClock::uptimeMillis();
}

InputEventSource::~InputEventSource(){
    if(frecord.is_open())
       frecord.close();
    LOGD("%p Destroied",this);
}

bool InputEventSource::initGesture(const std::string&fname){
#if ENABLED_GESTURE
    GRT::ClassificationData trainingData;
    if(!trainingData.load(fname))return false;
    trainingData.printStats();
    pipeline.reset(new GRT::GestureRecognitionPipeline);//std::make_unique< GRT::GestureRecognitionPipeline >();
    *pipeline << GRT::ANBC();
    if( !pipeline->train( trainingData ) ){
       LOGD("ERROR: Failed to train the pipeline!");
    } 
    return true;
#else
    return false;
#endif
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
               events.push(mt);
            });
        }else if(tmpdev.getClasses()&(INPUT_DEVICE_CLASS_KEYBOARD)){
            dev.reset(new KeyDevice(fd));
            dev->setEventConsumeListener([&](const InputEvent&e){
               KeyEvent*key=KeyEvent::obtain((KeyEvent&)e);
               events.push(key);
            });
        }
        devices.emplace(fd,dev);
        return dev;
    }
    return itr->second;
}

int InputEventSource::checkEvents(){
    INPUTEVENT es[32];
    if(events.size()>0)return TRUE;
    int count=InputGetEvents(es,32,1);
    process(es,count);
    return count;
}

int InputEventSource::handleEvents(){
    std::lock_guard<std::mutex> lock(mtxEvents);
    if(events.size()==0)return false;
    while(events.size()){
        InputEvent* e=events.front();
        WindowManager::getInstance().processEvent(*e);
        if((!isplayback)&& frecord.is_open() && dynamic_cast<KeyEvent*>(e) ){
            nsecs_t eventTime=SystemClock::uptimeMillis();
            const char*actname[]={"down","up","multi"};
            KeyEvent*key=dynamic_cast<KeyEvent*>(e);
            frecord<<"delay("<<eventTime-lasteventTime<<")"<<std::endl;
            frecord<<"key("<<actname[key->getAction()]<<","
                  <<KeyEvent::getLabel(key->getKeyCode())<<")"<<std::endl;
            lasteventTime=eventTime;
        }
        e->recycle();
        events.pop();
    }
    return 0; 
}

int  InputEventSource::process(const INPUTEVENT*inevents,int count){
    LOGV_IF(count,"%p  recv %d events ",this,count);
    for(int i=0;i<count;i++){
        const INPUTEVENT*e=inevents+i;
        struct timeval tv={e->tv_sec,e->tv_usec};
        std::shared_ptr<InputDevice>dev=getdevice(e->device);
        if(dev==nullptr){
            LOGD("%d,%d,%d device=%d ",e->type,e->code,e->value,e->device);
            continue;
        }
        dev->putRawEvent(tv,e->type,e->code,e->value);
    }
    return 0;
}

int InputEventSource::pushEvent(InputEvent*evt){
    std::lock_guard<std::mutex> lock(mtxEvents);
    events.push(evt);
    return events.size();
}

void InputEventSource::playback(const std::string&fname){
    #define DELIMITERS "(),"
    auto func=[this](const std::string&fname){
         std::fstream in(fname);
         std::this_thread::sleep_for(std::chrono::milliseconds(10000));
         LOGD_IF(in.good(),"play key from %s",fname.c_str());
         if(!in.good())return ;
         isplayback=true;
         while(1){
             char *ps=nullptr;
             char line[256]={0};
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
                 int action =(word.find("down")!=std::string::npos)?KeyEvent::ACTION_DOWN:KeyEvent::ACTION_UP;

                 tok->skipDelimiters(DELIMITERS);
                 word =tok->nextToken(DELIMITERS);
                 nsecs_t evttime=SystemClock::uptimeMillis();  
                 int keycode=KeyEvent::getKeyCodeFromLabel(word.c_str());
                 KeyEvent*key=KeyEvent::obtain(evttime,evttime,action,keycode,1,0/*metastate*/,
                       0/*deviceid*/,keycode/*scancode*/,0/*flags*/,0/*source*/);
                 events.push(key);
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

