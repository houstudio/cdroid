#ifndef __UIEVENT_SOURCE_H__
#define __UIEVENT_SOURCE_H__
#include <core/looper.h>
#include <list>
#include <unordered_set>
#include <view/view.h>
namespace cdroid{

class UIEventSource:public EventHandler{
private:
    typedef struct{
        nsecs_t  time;
        bool removed;
        Runnable run;
    }RUNNER;
    std::list<RUNNER>mRunnables;
    std::function<void()> mLayoutRunner;
    ViewGroup*mAttachedView;
    bool hasDelayedRunners()const;
	void handleCompose();
	int handleRunnables();
public:
    UIEventSource(View*,std::function<void()>run);
    ~UIEventSource();
    bool processEvents();
    int checkEvents()override;
    int handleEvents()override;
    bool postDelayed(Runnable& run,uint32_t delay=0);
    int removeCallbacks(const Runnable& what);
};

}//end namespace

#endif
