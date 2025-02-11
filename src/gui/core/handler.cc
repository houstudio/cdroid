#include <core/handler.h>
#include <core/systemclock.h>

namespace cdroid{

Handler::Handler():Handler(nullptr){
}

Handler::Handler(Callback callback)
    :Handler(Looper::getMainLooper(),callback){
}

Handler::Handler(Looper*looper,Callback callback):
    mLooper(looper),mCallback(callback){
    mLooper->addHandler(this);
}

Handler::~Handler(){
    mLooper->removeMessages(this);
}

Message* Handler::getPostMessage(Runnable& r){
    Message* m = Message::obtain();
    m->callback = r;
    return m;
}

void Handler::handleMessage(Message& msg) {
}

void Handler::handleIdle(){
}

bool Handler::hasMessages(int what,void*object){
    return false;
}

void Handler::removeMessages(int what){
    mLooper->removeMessages(this,what);
}

void Handler::removeMessages(int what,void*object){
    mLooper->removeMessages(this,what);
}

bool Handler::hasCallbacks(Runnable r){
    return true;
}

void Handler::removeCallbacks(const Runnable& r){
    mLooper->removeCallbacks(this,r);
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

Message*Handler::obtainMessage(){
    return Message::obtain();
}

Message*Handler::obtainMessage(int what){
    return Message::obtain(what);
}

Message*Handler::obtainMessage(int what,int arg1,int arg2){
    return Message::obtain(what,arg1,arg2);
}

Message*Handler::obtainMessage(int what,int arg1,int arg2,void*obj){
    return Message::obtain(what,arg1,arg2,obj);
}

bool Handler::sendMessage(Message& msg){
    return sendMessageDelayed(msg, 0);
}

bool Handler::sendEmptyMessage(int what){
    return sendEmptyMessageDelayed(what, 0);
}

bool Handler::sendEmptyMessageDelayed(int what, long delayMillis) {
    Message* msg = Message::obtain();
    msg->what = what;
    return sendMessageDelayed(*msg, delayMillis);
}

bool Handler::sendEmptyMessageAtTime(int what, int64_t uptimeMillis) {
    Message* msg = Message::obtain();
    msg->what = what;
    return sendMessageAtTime(*msg, uptimeMillis);
}

bool Handler::sendMessageDelayed(Message& msg, long delayMillis){
    if (delayMillis < 0) {
        delayMillis = 0;
    }
    return sendMessageAtTime(msg, SystemClock::uptimeMillis() + delayMillis);
}

bool Handler::sendMessageAtTime(Message& msg, int64_t uptimeMillis) {
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

bool Handler::post(Runnable r){
    Message* msg = getPostMessage(r);
    return sendMessageDelayed(*msg, 0);
}

bool Handler::postAtTime(Runnable r, int64_t uptimeMillis){
    Message* msg = getPostMessage(r);
    return sendMessageAtTime(*msg, uptimeMillis);
}

bool Handler::postDelayed(Runnable r, long delayMillis){
    Message* msg = getPostMessage(r);
    return sendMessageDelayed(*msg, delayMillis);
}

}
