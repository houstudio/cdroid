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
#include <thread>

namespace cdroid{

class InputEventSource:public EventHandler{
public:
    typedef std::function<void(bool)>ScreenSaver;
private:
    mutable std::recursive_mutex mtxEvents;
    ScreenSaver mScreenSaver;
    int mScreenSaveTimeOut;
    bool mInited;
    bool mRunning;
    bool mIsScreenSaveActived;
    /*The reader thread (spawned lazily in checkEvents), kept JOINABLE so the
      dtor can wait it out: it was detached before, and ~InputEventSource
      returning while the thread was still inside its 20ms select() left it
      locking mtxEvents / reading mRunning on freed memory (valgrind: invalid
      read/write in pthread_mutex_lock from InputThread at every app exit).*/
    std::thread mInputThread;
    nsecs_t mLastInputEventTime;/*for screensaver*/
    std::unordered_map<int,std::shared_ptr<InputDevice>>mDevices;
    /*Injected events (injectInputEvent) waiting for the main-looper drain —
      the InputDispatcher injection queue analog; guarded by mtxEvents.*/
    std::queue<InputEvent*> mInjectedEvents;
private:
    std::shared_ptr<InputDevice>getDevice(int fd);
    void doEventsConsume();
    bool needCancel(InputDevice*dev);
protected:
    InputEventSource();
    void onDeviceChanged(const INPUTEVENT*es);
public:
    static InputEventSource& getInstance();
    ~InputEventSource()override;
    void openScreenSaver();
    void setScreenSaver(ScreenSaver func,int timeout);
    void closeScreenSaver();
    bool isScreenSaverActived()const;
    int checkEvents()override;
    int handleEvents()override;
    /*Drain every device's pending event queue and return the events to the
      pool. Called on shutdown to reclaim MotionEvent/KeyEvent still queued
      after the main loop stopped consuming them: InputEventSource is a process
      singleton that is never destroyed, so its device queues would otherwise
      stay populated and leak on exit. Also stops the input thread so it cannot
      keep refilling the queues while we drain.*/
    void clearEvents();
    void sendEvent(InputEvent&);
    /*android.view.InputManager INJECT_INPUT_EVENT_MODE_* — the injection
      modes AOSP's IInputManager.injectInputEvent takes (hidden API surface).*/
    static constexpr int INJECT_INPUT_EVENT_MODE_ASYNC = 0;
    static constexpr int INJECT_INPUT_EVENT_MODE_WAIT_FOR_FINISH = 1;
    static constexpr int INJECT_INPUT_EVENT_MODE_WAIT_FOR_RESULT = 2;
    /**AOSP IInputManager.injectInputEvent: an injected event must enter the
       SAME pipeline as hardware input (AOSP: InputDispatcher queues it behind
       real events and dispatches it normally; it is indistinguishable from a
       device event). The in-process pipeline entry is the queue handleEvents()
       drains on the main looper → WindowManager::processEvent → recycle, so
       enqueue a copy there — never a direct processEvent shortcut. The caller
       keeps ownership of its event. The WAIT_* modes cannot block here:
       delivery happens on this same looper during a later drain, so every
       mode enqueues and returns (AOSP blocks the caller's thread instead —
       CDROID drivers run on the UI thread and wait by pumping the looper,
       e.g. UiAutomation::executeAndWaitForEvent).*/
    bool injectInputEvent(InputEvent& event, int mode);
    /*Lookup an input device by id (== fd) WITHOUT creating it — a pure registry
      query, the equivalent of Android InputManagerGlobal.getInputDevice(id).
      Returns nullptr if no device with that id is registered (e.g. a synthetic
      KeyEvent with deviceId < 0, or a hot-removed device). Thread-safe: takes
      mtxEvents (KeyEvent resolves chars from any thread, not just the input one).*/
    InputDevice* getInputDevice(int id);
    /*OR of every KeyDevice's local meta state. Mirrors Android
      InputReader::getGlobalMetaState — the global modifier view attached to
      KeyEvents/MotionEvents from any device. Safe to call from putEvent
      (mtxEvents is recursive and already held on the input thread).*/
    int32_t getGlobalMetaState()const;
};
}
#endif
