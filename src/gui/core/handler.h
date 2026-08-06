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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02101-1301  USA
 *********************************************************************************/
#ifndef __CORE_HANDLER_H__
#define __CORE_HANDLER_H__
#include <core/looper.h>
#include <core/message.h>      // cdroid::Message (reference semantics, aligning with android.os.Message)
#include <core/callbackbase.h>
namespace cdroid{

/**
 * Ported from android.os.Handler (reference: android-36 Handler.java).
 * Backed by cdroid::MessageQueue, reference-semantics cdroid::Message*, Java-model dispatchMessage.
 *
 * CDROID trade-offs:
 *  - Still `: public MessageHandler` — keeps handleIdle (UIEventHandler frame driving) + mHandlers registration.
 *  - Unified type: cdroid::Message is merged with the pooled Message (no struct/os:: distinction).
 *  - handleMessage(Message&) is the single virtual function; both apps and in-library consumers override it.
 *  - Two dispatchMessage entry points: (Message&) is called from the libutils mMessageEnvelopes path;
 *    (Message*) is called from the MessageQueue drain path (Looper::drainMessageQueue). Both end up in handleMessage(Message&).
 *  - mQueue comes from Looper::getQueue() (CDROID extension; the Looper owns the Java MessageQueue).
 */
class Handler:public MessageHandler{
public:
    using Callback = std::function<bool(Message&)>;  // android.os.Handler.Callback
private:
    Looper*mLooper;
    MessageQueue* mQueue;   // :1000 (from mLooper->getQueue())
    Callback mCallback;
    bool mAsynchronous;     // :1003
private:
    static void handleCallback(Message* message);                 // :994
    static Message* getPostMessage(const Runnable& r);            // :980
    static Message* getPostMessage(const Runnable& r,void*token); // :986
    bool enqueueMessage(MessageQueue* queue, Message* msg, int64_t uptimeMillis); // :782
public:
    Handler();
    Handler(Callback callback);
    Handler(Looper*looper);
    Handler(Looper*looper,Callback callback);
    virtual ~Handler();

    // Single virtual function: apps (153 sites) and in-library consumers override this. Message is unified to the pooled type.
    virtual void handleMessage(Message& msg) override {}
    // libutils-path dispatch (apps via looper->sendMessage -> mMessageEnvelopes -> here)
    void dispatchMessage(Message& msg) override;
    // MessageQueue-path dispatch (Looper::drainMessageQueue draining due messages -> here) -> handleMessage(*msg)
    virtual void dispatchMessage(Message* msg);

    void handleIdle()override;
    Looper* getLooper()const;
    MessageQueue* getQueue()const;

    bool hasMessages(int what);
    bool hasMessages(int what,void*object);
    bool hasCallbacks(const Runnable& r);
    void removeMessages(int what);
    void removeMessages(int what,void*object);
    void removeCallbacks(const Runnable& r);
    void removeCallbacksAndMessages(void*token);

    bool sendMessage(Message* msg);
    bool sendEmptyMessage(int what);
    bool sendEmptyMessageDelayed(int what, long delayMillis);
    bool sendEmptyMessageAtTime(int what, int64_t uptimeMillis);
    bool sendMessageDelayed(Message* msg, long delayMillis);
    bool sendMessageAtTime(Message* msg, int64_t uptimeMillis);
    bool sendMessageAtFrontOfQueue(Message* msg);

    Message* obtainMessage();
    Message* obtainMessage(int what);
    Message* obtainMessage(int what,void*obj);
    Message* obtainMessage(int what,int arg1,int arg2);
    Message* obtainMessage(int what,int arg1,int arg2,void*obj);

    bool post(const Runnable& r);
    bool postAtTime(const Runnable& r, int64_t uptimeMillis);
    bool postDelayed(const Runnable& r, long delayMillis);
    bool postAtFrontOfQueue(const Runnable&);
};
}//endof namespace
#endif
