#ifndef __CORE_HANDLER_H__
#define __CORE_HANDLER_H__
#include <core/looper.h>
namespace cdroid{

class Handler:public MessageHandler{
public:
    typedef std::function<bool(Message&)>Callback;
private:
    Looper*mLooper;
    Callback mCallback;
private:
    static void handleCallback(Message& message);
public:
    Handler();
    Handler(Callback callback);
    virtual ~Handler();
    void handleMessage(Message& msg)override;
    Looper* getLooper();
    void dispatchMessage(Message& msg);
    bool hasMessages(int what,void*object);
    bool hasCallbacks(Runnable r);
    bool sendMessage(Message& msg);
    bool sendEmptyMessage(int what);
    bool sendEmptyMessageDelayed(int what, long delayMillis);
    bool sendEmptyMessageAtTime(int what, long uptimeMillis);
    bool sendMessageDelayed(Message& msg, long delayMillis);
    bool sendMessageAtTime(Message& msg, long uptimeMillis);

};
}//endof namespace 
#endif
