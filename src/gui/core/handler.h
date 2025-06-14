#ifndef __CORE_HANDLER_H__
#define __CORE_HANDLER_H__
#include <core/looper.h>
#include <core/callbackbase.h>
namespace cdroid{

class Handler:public MessageHandler{
public:
    typedef std::function<bool(Message&)>Callback;
private:
    Looper*mLooper;
    Callback mCallback;
private:
    void handleCallback(Message& message);
    Message getPostMessage(const Runnable& r);
public:
    Handler();
    Handler(Callback callback);
    Handler(Looper*looper,Callback callback);
    virtual ~Handler();
    void handleMessage(Message& msg)override;
    void handleIdle()override;
    Looper* getLooper();
    void dispatchMessage(Message& msg)override;

    bool hasMessages(int what);
    bool hasMessages(int what,void*object);
    void removeMessages(int what);
    void removeMessages(int what,void*object);

    bool sendMessage(const Message& msg);
    bool sendEmptyMessage(int what);
    bool sendEmptyMessageDelayed(int what, long delayMillis);
    bool sendEmptyMessageAtTime(int what, int64_t uptimeMillis);
    bool sendMessageDelayed(const Message& msg, long delayMillis);
    bool sendMessageAtTime(const Message& msg, int64_t uptimeMillis);

    Message obtainMessage();
    Message obtainMessage(int what);
    Message obtainMessage(int what,int arg1,int arg2);
    Message obtainMessage(int what,int arg1,int arg2,void*obj);
    bool hasCallbacks(const Runnable& r);
    void removeCallbacks(const Runnable& r);
    bool post(const Runnable& r);
    bool postAtTime(const Runnable& r, int64_t uptimeMillis);
    bool postDelayed(const Runnable& r, long delayMillis);
};
}//endof namespace 
#endif
