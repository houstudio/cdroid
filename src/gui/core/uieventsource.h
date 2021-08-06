#ifndef __UIEVENT_SOURCE_H__
#define __UIEVENT_SOURCE_H__
#include <core/looper.h>
#include <list>
#include <unordered_set>
#include <widget/view.h>
namespace cdroid{

class UIEventSource:public EventHandler{
private:
    typedef struct{
        nsecs_t  time;
        bool removed;
        Runnable run;
    }RUNNER;
    std::list<RUNNER>mRunnables;
    View*mAttachedView;
    bool hasDelayedRunners()const;
public:
    UIEventSource(View*);
    ~UIEventSource();
    bool processEvents();
    int checkEvents()override;
    int handleEvents()override;
    void post(const Runnable& run,uint32_t delay=0);
    void removeCallbacks(const Runnable& what);
};

}//end namespace

#endif
