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
#ifndef __INPUT_EVENT_SOURCE_H__
#define __INPUT_EVENT_SOURCE_H__
#include <cdinput.h>
#include <queue>
#include <string>
#include <fstream>
#include <core/looper.h>
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
    std::unordered_map<int,std::shared_ptr<InputDevice>>mDevices;
    static std::unique_ptr<InputEventSource>mInst;
private:
    std::shared_ptr<InputDevice>getDevice(int fd);
    void doEventsConsume();
    bool needCancel(InputDevice*dev);
    void recordEvent(InputEvent&);
    InputEvent*parseEvent(const char*);
protected:
    InputEventSource();
    void onDeviceChanged(const INPUTEVENT*es);
public:
    static InputEventSource& getInstance();
    ~InputEventSource();
    void openScreenSaver();
    void setScreenSaver(ScreenSaver func,int timeout);
    void closeScreenSaver();
    bool isScreenSaverActived()const;
    void record(const std::string&fname);
    void playback(const std::string&fname);
    int checkEvents()override;
    int handleEvents()override;
    void sendEvent(InputEvent&);
};
}
#endif
