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
    Message getPostMessage(Runnable& r);
public:
    Handler();
    Handler(Callback callback);
    Handler(Looper*looper,Callback callback);
    virtual ~Handler();
    void handleMessage(Message& msg)override;
    void handleIdle()override;
    Looper* getLooper();
    void dispatchMessage(Message& msg)override;

    bool hasMessages(int what,void*object);
    void removeMessages(int what);
    void removeMessages(int what,void*object);

    bool sendMessage(Message& msg);
    bool sendEmptyMessage(int what);
    bool sendEmptyMessageDelayed(int what, long delayMillis);
    bool sendEmptyMessageAtTime(int what, int64_t uptimeMillis);
    bool sendMessageDelayed(Message& msg, long delayMillis);
    bool sendMessageAtTime(Message& msg, int64_t uptimeMillis);

    Message obtainMessage();
    Message obtainMessage(int what);
    Message obtainMessage(int what,int arg1,int arg2);
    Message obtainMessage(int what,int arg1,int arg2,void*obj);
    bool hasCallbacks(Runnable r);
    void removeCallbacks(const Runnable& r);
    bool post(Runnable r);
    bool postAtTime(Runnable r, int64_t uptimeMillis);
    bool postDelayed(Runnable r, long delayMillis);
};
}//endof namespace 
#endif
