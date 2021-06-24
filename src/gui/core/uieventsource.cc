#include <uieventsource.h>
#include <windowmanager.h>
#include <cdlog.h>
#include <systemclock.h>
#include <widget/view.h>
#include <list>


namespace cdroid{

bool operator>(const UIMSG&m1,const UIMSG m2){
   return m1.time>m2.time;
}

bool operator==(const UIMSG&m1,const UIMSG m2){
   return m1.msgid==m2.msgid;
}

template<class T>
static const void * addr_of(T &&obj) noexcept{
   struct A {};
   return &reinterpret_cast<const A &>(obj);
}

UIEventSource::UIEventSource(View*v){
    mAttachedView=v;
}

UIEventSource::~UIEventSource(){
}

int UIEventSource::getEvents(){
    return normal_msgs.size()||hasDelayedMessage()||(mAttachedView&&mAttachedView->isDirty())||GraphDevice::getInstance().needCompose();
}

bool UIEventSource::processEvents(){
    bool rc=popMessage();//called by Dispatch,return false the  source will be killed
    if(GraphDevice::getInstance().needCompose())
       GraphDevice::getInstance().ComposeSurfaces();
    return rc;
}

bool UIEventSource::dispatch(EventHandler &func){ 
    if(func)return func(*this); 
    return processEvents();
}

void UIEventSource::reset(){
    while(!delayed_msgs.empty())delayed_msgs.pop_front();
    while(!normal_msgs.empty())normal_msgs.pop_front();
}

void UIEventSource::sendMessage(View*v,DWORD msgid,DWORD wp,ULONG lp,DWORD delayedtime){
    postMessage(v,msgid,wp,lp,nullptr,delayedtime);
}

void UIEventSource::postMessage(View*v,DWORD msgid,DWORD wp,ULONG lp,const Runnable action,DWORD delayedtime){
    UIMSG msg;
    msg.view=v;
    msg.msgid=msgid;
    msg.wParam=wp;    
    msg.lParam=lp;
    msg.runnable=action;
    if(delayedtime!=0||action!=nullptr){
        msg.time=SystemClock::uptimeMillis()+delayedtime;
        for(auto itr=delayed_msgs.begin();itr!=delayed_msgs.end();itr++){
           if(msg.time<itr->time){
               delayed_msgs.insert(itr,msg);
               return ;
           }
        }
        delayed_msgs.push_back(msg);
        return;
    }
    normal_msgs.push_back(msg);
}

void UIEventSource::post(View*v,const Runnable run,DWORD delayedtime){
    postMessage(v,0,0,0,run,delayedtime);
}

void UIEventSource::removeCallbacks(const Runnable what){
    normal_msgs.remove_if([&](const UIMSG & m)->bool{
        return (m.msgid==0) && (addr_of(m.runnable)==addr_of(what));
    });
    delayed_msgs.remove_if([&](const UIMSG& m)->bool{
            return (m.msgid==0) && (addr_of(m.runnable)==addr_of(what));
    });
}

bool UIEventSource::hasDelayedMessage(){
    UIMSG msg;
    if(delayed_msgs.empty())return false;
    ULONGLONG nowms=SystemClock::uptimeMillis();
    msg=delayed_msgs.front();
    return msg.time<nowms;
}

bool UIEventSource::popMessage(){
    UIMSG msg={0};
    if(normal_msgs.size()){
        msg=normal_msgs.front();
        normal_msgs.pop_front();
    }else if(hasDelayedMessage()){
        msg=delayed_msgs.front();
        delayed_msgs.pop_front();
    }
    if (mAttachedView && mAttachedView->isDirty()){
	mAttachedView->draw();
	GraphDevice::getInstance().flip();
    }
    if(msg.view){
        msg.view->onMessage(msg.msgid,msg.wParam,msg.lParam);
    }
    switch(msg.msgid){
    case View::WM_DESTROY:
        WindowManager::getInstance().removeWindow(static_cast<Window*>(msg.view));
        return false;//return false to tell Eventsource::Dispath,and EventLooper will remove the source. 
    case View::WM_ACTIVE:
        if(msg.wParam)dynamic_cast<Window*>(msg.view)->onActive();
        else dynamic_cast<Window*>(msg.view)->onDeactive(); 
        break;
    case 0:
        if(msg.runnable)msg.runnable();
    }
    return true;
}

bool UIEventSource::hasMessage(const View*v,DWORD msgid){
    const UIMSG msg={(View*)v,msgid};
    return delayed_msgs.end()==std::find(delayed_msgs.begin(),delayed_msgs.end(),msg);
}

void UIEventSource::removeMessage(const View*v,DWORD msgid){
    normal_msgs.remove_if([&](const UIMSG& m)->bool{
                return (m.msgid==msgid)&&(m.view==v);
            });
    delayed_msgs.remove_if([&](const UIMSG& m)->bool{
                return m.msgid==msgid&&m.view==v;
            });
}

}//end namespace
