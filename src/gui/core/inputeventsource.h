#ifndef __INPUT_EVENT_SOURCE_H__
#define __INPUT_EVENT_SOURCE_H__
#include <cdinput.h>
#include <queue>
#include <string>
#include <fstream>
#include <core/looper.h>
#include <core/uievents.h>
#include <core/inputdevice.h>
#include <unordered_map>
#include <mutex>

namespace cdroid{

class InputEventSource:public EventHandler{
public:	
    typedef std::function<void(bool)>ScreenSaver;
private:
    std::mutex mtxEvents;
    ScreenSaver mScreenSaver;
    int mScreenSaveTimeOut;
    bool mRunning;
    bool mIsPlayback;
    bool mIsScreenSaveActived;
    nsecs_t mLastPlaybackEventTime;/*for event record and playback*/
    nsecs_t mLastInputEventTime;/*for screensaver*/
    std::ofstream frecord;
    std::queue<InputEvent*>mInputEvents;
    std::queue<INPUTEVENT> mRawEvents;
    std::unordered_map<int,std::shared_ptr<InputDevice>>mDevices;
    std::shared_ptr<InputDevice>getdevice(int fd);
protected:
    InputEventSource();
    int pushEvent(InputEvent*evt);
    int process();
    void onDeviceChanged(const INPUTEVENT*es);
public:
    static InputEventSource& getInstance();
    ~InputEventSource();
    void setScreenSaver(ScreenSaver func,int timeout);
    void closeScreenSaver();
    bool isScreenSaverActived()const;
    void playback(const std::string&fname);
    int checkEvents()override;
    int handleEvents()override;
};
}
#endif
