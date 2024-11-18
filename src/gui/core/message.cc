#include <core/message.h>
#include <core/handler.h>
namespace cdroid{
std::mutex Message::mLock;
Message* Message::sPool = nullptr;
int Message::sPoolSize =0;
Message::Message(int what){
   this->what=what;
}

void Message::sendToTarget(){
    target->sendMessage(*this);
}

Message*Message::obtain(){
    std::lock_guard<std::mutex>_l(mLock);
    if (sPool != nullptr) {
        Message* m = sPool;
        sPool   = m->next;
        m->next = nullptr;
        m->flags = 0; // clear in-use flag
        m->target= nullptr;
        sPoolSize--;
        return m;
    }
    return new Message();
}

Message*Message::obtain(const Message&orig){
    Message* m = obtain();
    m->what = orig.what;
    m->arg1 = orig.arg1;
    m->arg2 = orig.arg2;
    m->obj = orig.obj;
    m->target = orig.target;
    m->callback = orig.callback;
    /*m->replyTo = orig.replyTo;
    m->sendingUid = orig.sendingUid;
    if (orig.data != nullptr) {
        m.data = new Bundle(orig.data);
    }*/
    return m;
}

Message* Message::obtain(Handler* h) {
    Message* m = obtain();
    m->target = h;
    return m;
}

void Message::recycle(){
    if(isInUse())return;
    flags = FLAG_IN_USE;
    what = 0;
    arg1 = 0;
    arg2 = 0;
    obj = nullptr;
    when = 0;
    target = nullptr;
    callback = nullptr;
    /*replyTo = nullptr;
    sendingUid = -1;
    data = nullptr;*/

    std::lock_guard<std::mutex>_l(mLock);
    if (sPoolSize < MAX_POOL_SIZE) {
        next = sPool;
        sPool = this;
        sPoolSize++;
    }
}

void Message::recycleUnchecked(){

}

}
