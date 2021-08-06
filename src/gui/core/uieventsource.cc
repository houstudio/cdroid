#include <uieventsource.h>
#include <windowmanager.h>
#include <cdlog.h>
#include <systemclock.h>
#include <widget/view.h>
#include <list>


namespace cdroid{

UIEventSource::UIEventSource(View*v){
    mAttachedView=v;
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
    for(auto it=mRunnables.begin();it!=mRunnables.end();it++){
        LOGV_IF(it->removed,"remove %d",(int)it->run);
        if(it->removed)it=mRunnables.erase(it);
    }
    if(hasDelayedRunners()){
        RUNNER runner=mRunnables.front();
        runner.run();//maybe user will removed runnable itself in its runnable'proc,so we use removed flag to flag it
        LOGV("remove %d",(int)runner.run);
        mRunnables.pop_front(); 
    } 
    return 0;
}

void UIEventSource::post(const Runnable& run,uint32_t delayedtime){
    RUNNER runner;
    runner.removed=false;
    runner.time=SystemClock::uptimeMillis()+delayedtime;
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
    for(auto it=mRunnables.begin();it!=mRunnables.end();it++){ 
        if(it->run==what) it->removed=true;
    }
}

}//end namespace
