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
    if (mAttachedView && mAttachedView->isAttachedToWindow() && mAttachedView->isDirty() 
           && mAttachedView->getVisibility()==View::VISIBLE){
        ((Window*)mAttachedView)->draw();
        GraphDevice::getInstance().flip();
    }
    if(GraphDevice::getInstance().needCompose())
        GraphDevice::getInstance().composeSurfaces();

    mRunnables.remove_if([](const RUNNER&r)->bool{
         return r.removed;
    });
    if(hasDelayedRunners()){
        RUNNER runner=mRunnables.front();
        if(runner.run)runner.run();//maybe user will removed runnable itself in its runnable'proc,so we use removed flag to flag it
        mRunnables.pop_front(); 
    } 
    return 0;
}

#pragma GCC push_options
#pragma GCC optimize("O0")
//codes between pragma will crashed in ubuntu GCC V8.x,bus GCC V7 wroked well.
bool UIEventSource::post(Runnable& run,uint32_t delayedtime){
    RUNNER runner;
    runner.removed=false;
    runner.time=SystemClock::uptimeMillis()+delayedtime;
    run.newInstance();
    runner.run=run;
	
    for(auto itr=mRunnables.begin();itr!=mRunnables.end();itr++){
        if(runner.time<itr->time){
            mRunnables.insert(itr,runner);
            return true;
        }
    }
    mRunnables.push_back(runner);
    return true;
}
#pragma GCC pop_options

bool UIEventSource::hasDelayedRunners()const{
    if(mRunnables.empty())return false;
    nsecs_t nowms=SystemClock::uptimeMillis();
    RUNNER runner=mRunnables.front();
    return runner.time<nowms;
}

bool UIEventSource::removeCallbacks(const Runnable& what){
    for(auto it=mRunnables.begin();it!=mRunnables.end();it++){ 
        if(it->run==what){
            it->removed=true;
            return true;
        }
    }
    return false;
}

}//end namespace
