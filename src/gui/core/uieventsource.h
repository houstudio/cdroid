#ifndef __UIEVENT_SOURCE_H__
#define __UIEVENT_SOURCE_H__
#include <looper/looper.h>
#include <list>
#include <unordered_set>
#include <widget/view.h>
namespace cdroid{
class View;
typedef struct{
   View*view;
   DWORD msgid;
   DWORD wParam;
   ULONG lParam;
   Runnable runnable;
   ULONG time;
}UIMSG;
class UIEventSource:public EventSource{
private:
    std::list<UIMSG> normal_msgs;
    std::list<UIMSG> delayed_msgs;
    bool hasDelayedMessage();
    View*mAttachedView;
    bool popMessage();
    void postMessage(View*,DWORD msgid,DWORD wp,ULONG lp,const Runnable action,DWORD delayedtime=0);
public:
    UIEventSource(View*);
    ~UIEventSource();
    void reset();
    int getEvents();
    bool prepare(int&) override { return getEvents();}
    bool check(){
        return  getEvents();
    }
    bool dispatch(EventHandler &func)override;
    bool processEvents();
    bool hasMessage(const View*,DWORD msgid);
    void removeMessage(const View*,DWORD msgid);
    void sendMessage(View*,DWORD msgid,DWORD wp,ULONG lp,DWORD delayedtime=0);
    void post(View*v,const Runnable run,DWORD delay=0);
    void removeCallbacks(const Runnable what);
};

}//end namespace

#endif
