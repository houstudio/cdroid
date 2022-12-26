#ifndef __INPUT_EVENT_SOURCE_H__
#define __INPUT_EVENT_SOURCE_H__
#include <cdinput.h>
#include <core/looper.h>
#include <queue>
#include <string>
#include <fstream>
#include <uievents.h>
#include <inputdevice.h>
#include <unordered_map>
#include <mutex>

namespace cdroid{

class InputEventSource:public EventHandler{
protected:
    std::mutex mtxEvents;
    bool isplayback;
    nsecs_t lasteventTime;
    std::ofstream frecord;
    std::queue<InputEvent*>mInputEvents;
    std::queue<INPUTEVENT>mRawEvents;
    std::unordered_map<int,std::shared_ptr<InputDevice>>devices;
    std::shared_ptr<InputDevice>getdevice(int fd);
    int pushEvent(InputEvent*evt);
    int process();
public:
    InputEventSource(const std::string&recordfile=std::string() );
    ~InputEventSource();
    bool initGesture(const std::string&fname);
    void playback(const std::string&fname);
    int checkEvents()override;
    int handleEvents()override;
};
}
#endif
