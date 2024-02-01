#include <uieventsource.h>
#include <windowmanager.h>
#include <cdlog.h>
#include <systemclock.h>
#include <list>

namespace cdroid{

UIEventSource::UIEventSource(View*v,std::function<void()>r){
    mAttachedView = dynamic_cast<ViewGroup*>(v);
    mLayoutRunner = r;
    setOwned(true);
}

UIEventSource::~UIEventSource(){
}

int UIEventSource::checkEvents(){
    return hasDelayedRunners()||(mAttachedView&&mAttachedView->isDirty())
           ||mAttachedView->isLayoutRequested()
           //||mAttachInfo->mViewRequestingLayout
           ||GraphDevice::getInstance().needCompose();
}

void UIEventSource::handleCompose(){
    GraphDevice::getInstance().lock();
#if COMPOSE_ASYNC
    if(GraphDevice::getInstance().needCompose())
        GraphDevice::getInstance().requestCompose();
#else
    GraphDevice::getInstance().composeSurfaces();
#endif

    GraphDevice::getInstance().unlock();
}

int UIEventSource::handleRunnables(){
    int count=0;
    GraphDevice::getInstance().lock();
    if ( ((mFlags&1)==0) && mAttachedView && mAttachedView->isAttachedToWindow()){
        if(mAttachedView->isLayoutRequested())
            mLayoutRunner();
        const nsecs_t nowms = SystemClock::uptimeMillis();
        //maybe user will removed runnable itself in its runnable'proc,so we use removed flag to flag it
        while(mRunnables.size() && ((mFlags&1)==0)){
            RUNNER runner = mRunnables.front();
            if(runner.time > nowms)break;
            mRunnables.pop_front();
            if(runner.run&&(runner.removed==false))runner.run();
            count++;
        }
        if(((mFlags&1)==0) && mAttachedView->isDirty() && mAttachedView->getVisibility()==View::VISIBLE){
            ((Window*)mAttachedView)->draw();
            GraphDevice::getInstance().flip();
        }
    }
    GraphDevice::getInstance().unlock();
    return count;
}

int UIEventSource::handleEvents(){
    handleRunnables();
    handleCompose();
    return 0;
}

#pragma GCC push_options
#pragma GCC optimize("O0")
//codes between pragma will crashed in ubuntu GCC V8.x,bus GCC V7 wroked well.
bool UIEventSource::postDelayed(Runnable& run,uint32_t delayedtime){
    RUNNER runner;
    runner.removed = false;
    runner.time = SystemClock::uptimeMillis() + delayedtime;
    runner.run = run;
	
    for(auto itr = mRunnables.begin();itr != mRunnables.end();itr++){
        if(runner.time < itr->time){
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
    nsecs_t nowms = SystemClock::uptimeMillis();
    RUNNER runner = mRunnables.front();
    return runner.time < nowms;
}

int UIEventSource::removeCallbacks(const Runnable& what){
    int count=0;
    for(auto it = mRunnables.begin();it != mRunnables.end();it++){
        if((it->run == what)&&(it->removed == false)){
            it->removed = true;
            count++;
        }
    }
    return count;
}

}//end namespace
