#include <uieventsource.h>
#include <windowmanager.h>
#include <cdlog.h>
#include <systemclock.h>
#include <widget/view.h>
#include <list>


namespace cdroid{

UIEventSource::UIEventSource(View*v){
    mAttachedView=v;
    mID=0;
}

UIEventSource::~UIEventSource(){
}

int UIEventSource::checkEvents(){
    int rc= hasDelayedRunners()+(mAttachedView&&mAttachedView->isDirty())+GraphDevice::getInstance().needCompose();
    return rc;
}

int UIEventSource::handleEvents(){
    if (mAttachedView && mAttachedView->isDirty()){
        mAttachedView->draw();
        GraphDevice::getInstance().flip();
    }
    if(GraphDevice::getInstance().needCompose())
        GraphDevice::getInstance().ComposeSurfaces();
    if(hasDelayedRunners()){
        RUNNER runner=mRunnables.front();
        runner.run();
        mRunnables.pop_front(); 
    } 
    return 0;
}

void UIEventSource::post(Runnable& run,DWORD delayedtime){
    RUNNER runner;
    runner.time=SystemClock::uptimeMillis()+delayedtime;
    run.ID=mID++;
    runner.run=run;
    for(auto itr=mRunnables.begin();itr!=mRunnables.end();itr++){
        if(runner.time<itr->time){
            mRunnables.insert(itr,runner);
            return;
        }
    }
    mRunnables.push_back(runner);
}

bool UIEventSource::hasDelayedRunners()const{
    if(mRunnables.empty())return false;
    nsecs_t nowms=SystemClock::uptimeMillis();
    RUNNER runner=mRunnables.front();
    return runner.time<nowms;
}

void UIEventSource::removeCallbacks(const Runnable& what){
    mRunnables.remove_if([&](const RUNNER& m)->bool{
        return (m.run.ID==what.ID);
    });
}

}//end namespace
