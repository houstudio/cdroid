#include <inputeventsource.h>
#include <uievents.h>
#include <cdlog.h>
#include <unordered_map>
#include <windowmanager.h>
#include <uievents.h>
#include <thread>
#include <chrono>
#include <cstring>
#if ENABLED_GESTURE
#include <GRT/GRT.h>
#endif


namespace cdroid{

static int isplayingback=0;    
InputEventSource::InputEventSource(const std::string&file){
    InputInit();
    frecord.open(file);
    LOGD("%p Created",this);
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
        InputDevice tmpdev(fd);
        if(tmpdev.getSource()&(1<<EV_ABS)){
            dev.reset(new MouseDevice(fd));
            dev->setEventConsumeListener([&](const InputEvent&e){
               MotionEvent*mt=MotionEvent::obtain((MotionEvent&)e);
               events.push(mt);
            });
        }else if(tmpdev.getSource()&(1<<EV_KEY)){
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

bool InputEventSource::prepare(int& max_timeout){
    INPUTEVENT es[32];
    if(events.size()>0)return TRUE;
    int count=InputGetEvents(es,32,1);
    process(es,count);
    return count>0;
}

int  InputEventSource::process(const INPUTEVENT*inevents,int count){
    LOGV_IF(count,"%p  recv %d events ",this,count);
    for(int i=0;i<count;i++){
        const INPUTEVENT*e=inevents+i;
        std::shared_ptr<InputDevice>dev=getdevice(e->device);
        if(dev==nullptr){
            LOGD("%d,%d,%d device=%d ",e->type,e->code,e->value,e->device);
            continue;
        }
        dev->putrawdata(e);
    }
    return 0;
}

bool InputEventSource::processKey(){
    std::lock_guard<std::mutex> lock(mtxEvents);
    if(events.size()==0)return false;
    while(events.size()){
        InputEvent* e=events.front();
        WindowManager::getInstance().processEvent(*e);
        if((isplayingback==0)&& frecord.is_open() &&(e->getType()==EV_KEY) ){
            struct timespec ts;
            clock_gettime(CLOCK_MONOTONIC,&ts);
            frecord<<(ts.tv_sec*1000+ts.tv_nsec/1000000)<<",";
            frecord<<KeyEvent::getLabel(((KeyEvent*)e)->getKeyCode());
            frecord<<","<<((KeyEvent*)e)->getAction()<<std::endl;
            LOGD("%lld key:%s",(ts.tv_sec*1000+ts.tv_nsec/1000000),KeyEvent::getLabel(((KeyEvent*)e)->getKeyCode()));
        }
        e->recycle();
        events.pop();
    }
    return true;
}
int InputEventSource::pushEvent(InputEvent*evt){
    std::lock_guard<std::mutex> lock(mtxEvents);
    events.push(evt);
    return events.size();
}

void InputEventSource::playBack(const std::string&file,InputEventSource*es){
    std::ifstream fs;
    char line[256]={0};
    uint64_t last_event_time=0;
    fs.open(file);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    isplayingback=fs.good();
    LOGD("play macro %s  isplayingback=%d",file.c_str(),isplayingback);
    while(fs){
        char*ps=nullptr;
        fs.getline(line,255);
        uint64_t cur=strtoll(line,&ps,10);
        int keyCode=KeyEvent::getKeyCodeFromLabel(ps+1);
        ps=strchr(ps,',');
        int action=strtoll(ps+1,&ps,10);
        KeyEvent*evt=KeyEvent::obtain(cur,cur,action,keyCode,0,0,0,0,0,0);
        LOGV("eventtime=%lld  key:%s/%x",cur,ps+1,evt->getKeyCode());
        es->pushEvent(evt);
        if(last_event_time!=0){
            std::chrono::milliseconds dur(cur-last_event_time);
            std::this_thread::sleep_for(dur);
        }last_event_time=cur;
        if(fs.eof()){
            fs.close();
            fs.open(file);
            last_event_time=0;
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        }
    }
    fs.close();
}
void InputEventSource::play(const std::string&file){
    if(file.empty())return;
    std::thread th(playBack,std::ref(file),this);
    th.detach();
}

}//end namespace

