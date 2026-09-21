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
    /*Reader backend. Thread (default) = the dedicated evdev reader thread
      blocking in InputGetEvents(20ms) and waking the main looper; the stock
      behavior. Choreographer = no reader thread: a self-reposting
      Choreographer CALLBACK_INPUT poll calls InputGetEvents with a 0ms
      timeout (non-blocking probe) at frame cadence and dispatches on the
      spot, so input precedes the frame's ANIMATION/TRAVERSAL phases
      (Android frame order). Trade-offs: input latency is bounded by the
      frame delay (default 33ms), and the main loop keeps waking at frame
      cadence even when idle. Fixed before exec(); no runtime switch.*/
    enum Mode{ Thread, Choreographer };
    /*Select the reader mode BEFORE the main loop starts (App parses
      --input-mode ahead of getInstance()); a later call is ignored once
      the reader is up (checkEvents' lazy init reads it exactly once).*/
    static void setMode(Mode mode);
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
    /*Choreographer mode: the self-reposting CALLBACK_INPUT runner. Held as a
      member so removeCallbacks gets a stable action pointer to match on
      (records match by action address + token, see CallbackRecord::compare).*/
    Runnable mFramePoll;
    nsecs_t mLastInputEventTime;/*for screensaver*/
    std::unordered_map<int,std::shared_ptr<InputDevice>>mDevices;
    /*Injected events (injectInputEvent) waiting for the main-looper drain —
      the InputDispatcher injection queue analog; guarded by mtxEvents.*/
    std::queue<InputEvent*> mInjectedEvents;
private:
    std::shared_ptr<InputDevice>getDevice(int fd);
    void doEventsConsume();
    /*Queue-fill shared by both reader backends: stash a raw InputGetEvents
      batch into the per-device queues (device add/remove included). Returns
      count, exactly what the reader consumed. The caller holds no lock:
      this takes mtxEvents itself, like the old inline loop body did.*/
    int consumeRawEvents(const INPUTEVENT*es,int count);
    /*Choreographer-mode reader: post the first CALLBACK_INPUT poll (called
      from checkEvents' lazy init instead of spawning the thread) / drop any
      pending poll record (shutdown; no-op in Thread mode).*/
    void startFramePolling();
    void stopFramePolling();
    void onFramePoll();
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
