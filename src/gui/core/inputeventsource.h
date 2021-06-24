#ifndef __INPUT_EVENT_SOURCE_H__
#define __INPUT_EVENT_SOURCE_H__
#include <cdinput.h>
#include <looper/looper.h>
#include <queue>
#include <string>
#include <fstream>
#include <uievents.h>
#include <inputdevice.h>
#include <unordered_map>
#include <mutex>

#if ENABLED_GESTURE
namespace GRT{
   class GestureRecognitionPipeline;
}
#endif

namespace cdroid{

class InputEventSource:public EventSource{
protected:
    std::mutex mtxEvents;
    std::ofstream frecord;
    std::queue<InputEvent*>events;
#if ENABLED_GESTURE
    std::unique_ptr<GRT::GestureRecognitionPipeline>pipeline;
#endif
    std::unordered_map<int,std::shared_ptr<InputDevice>>devices;
    std::shared_ptr<InputDevice>getdevice(int fd);
    int pushEvent(InputEvent*evt);
    static void playBack(const std::string&fname,InputEventSource*es);
public:
    InputEventSource(const std::string&recordfile=std::string() );
    ~InputEventSource();
    bool initGesture(const std::string&fname);
    void play(const std::string&);
    bool prepare(int& max_timeout);
    bool check(){return events.size()>0;}
    bool dispatch(EventHandler &func) { return func(*this); }
    bool is_file_source() const override final { return false; }
    int process(const INPUTEVENT*es,int count);
    bool processKey();
};
}
#endif
