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
#include <core/message.h>      // cdroid::Message (引用语义, 对齐 android.os.Message)
#include <core/callbackbase.h>
namespace cdroid{

/**
 * 移植自 android.os.Handler (蓝本: android-36 Handler.java)。
 * 以 cdroid::MessageQueue 为后端、引用语义 cdroid::Message*、Java 模型 dispatchMessage。
 *
 * CDROID 取舍:
 *  - 仍 `: public MessageHandler` —— 保留 handleIdle (UIEventHandler 帧驱动) + mHandlers 注册。
 *  - 统一类型: cdroid::Message 已与池化 Message 合并 (无 struct/os:: 之分)。
 *  - handleMessage(Message&) 是单一虚函数, apps 与库内消费者统一 override 它。
 *  - 两个 dispatchMessage 入口: (Message&) 由 libutils mMessageEnvelopes 路径调用;
 *    (Message*) 由 MessageQueue 排空路径 (Looper::drainMessageQueue) 调用。两者最终都走 handleMessage(Message&)。
 *  - mQueue 取自 Looper::getQueue() (CDROID 扩展, Looper 拥有 Java MessageQueue)。
 */
class Handler:public MessageHandler{
public:
    using Callback = std::function<bool(Message&)>;  // android.os.Handler.Callback
private:
    Looper*mLooper;
    MessageQueue* mQueue;   // :1000 (取自 mLooper->getQueue())
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

    // 单一虚函数: apps (153 处) 与库内消费者均 override 此。Message 已统一为池化类型。
    virtual void handleMessage(Message& msg) override {}
    // libutils 路径派发 (apps 经 looper->sendMessage → mMessageEnvelopes → 此处)
    void dispatchMessage(Message& msg) override;
    // MessageQueue 路径派发 (Looper::drainMessageQueue 排空到期消息 → 此处) → handleMessage(*msg)
    virtual void dispatchMessage(Message* msg);

    void handleIdle()override;
    Looper* getLooper();
    MessageQueue* getQueue();

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
};
}//endof namespace
#endif
