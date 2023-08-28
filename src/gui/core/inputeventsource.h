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
public:	
    typedef std::function<bool(bool)>ScreenSaver;
private:
    std::mutex mtxEvents;
    ScreenSaver mScreenSaver;
    int mScreenSaveTimeOut;
    bool mIsPlayback;
    bool mIsScreenSaveActived;
    nsecs_t mLastEventTime;
    std::ofstream frecord;
    std::queue<InputEvent*>mInputEvents;
    std::queue<INPUTEVENT>mRawEvents;
    std::unordered_map<int,std::shared_ptr<InputDevice>>devices;
    std::shared_ptr<InputDevice>getdevice(int fd);
protected:
    int pushEvent(InputEvent*evt);
    int process();
    InputEventSource();
public:
    static InputEventSource& getInstance();
    ~InputEventSource();
    void setScreenSaver(ScreenSaver func,int timeout);
    void playback(const std::string&fname);
    int checkEvents()override;
    int handleEvents()override;
};
}
#endif
