#include <core/handler.h>
#include <core/systemclock.h>

namespace cdroid{

Handler::Handler(){
    mLooper = Looper::getDefault();
}

Handler::Handler(Callback callback):Handler(){
    mCallback = callback;
}

Handler::~Handler(){
    mLooper->removeMessages(this);
}

void Handler::handleMessage(Message& msg) {
}

bool Handler::hasMessages(int what,void*object){
    return false;
}

bool Handler::hasCallbacks(Runnable r){
    return false;
}

Looper* Handler::getLooper(){
    return mLooper;
}

void Handler::handleCallback(Message& message) {
    message.callback();
}

void Handler::dispatchMessage(Message& msg) {
    if (msg.callback != nullptr) {
        handleCallback(msg);
    } else {
        if (mCallback != nullptr) {
            if (mCallback(msg)){//mCallbackack.handleMessage(msg)) {
                return;
            }
        }
        handleMessage(msg);
    }
}

bool Handler::sendMessage(Message& msg){
    return sendMessageDelayed(msg, 0);
}

bool Handler::sendEmptyMessage(int what){
    return sendEmptyMessageDelayed(what, 0);
}

bool Handler::sendEmptyMessageDelayed(int what, long delayMillis) {
    Message msg;// = Message.obtain();
    msg.what = what;
    return sendMessageDelayed(msg, delayMillis);
}

bool Handler::sendEmptyMessageAtTime(int what, long uptimeMillis) {
    Message msg;// = Message.obtain();
    msg.what = what;
    return sendMessageAtTime(msg, uptimeMillis);
}

bool Handler::sendMessageDelayed(Message& msg, long delayMillis){
    if (delayMillis < 0) {
        delayMillis = 0;
    }
    return sendMessageAtTime(msg, SystemClock::uptimeMillis() + delayMillis);
}

bool Handler::sendMessageAtTime(Message& msg, long uptimeMillis) {
#if 0
    MessageQueue queue = mQueue;
    if (queue == null) {
        RuntimeException e = new RuntimeException(
            this + " sendMessageAtTime() called with no mQueue");
        Log.w("Looper", e.getMessage(), e);
        return false;
    }
    return enqueueMessage(queue, msg, uptimeMillis);
#else
    mLooper->sendMessageAtTime(uptimeMillis,this,msg);
#endif
    return true;
}

}
