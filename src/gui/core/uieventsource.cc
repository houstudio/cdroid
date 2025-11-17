/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
#include <uieventsource.h>
#include <windowmanager.h>
#include <cdlog.h>
#include <systemclock.h>
#include <list>

namespace cdroid{

UIEventSource::UIEventSource(View*v,const Runnable&r):mLayoutRunner(r){
    mAttachedView = dynamic_cast<ViewGroup*>(v);
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
        //if(mAttachedView->isLayoutRequested())
        //    mLayoutRunner();
        const nsecs_t nowms = SystemClock::uptimeMillis()-1;/*-1 to prevent postDelay(mRunable,0)in some Runnable*/
        //maybe user will removed runnable itself in its runnable'proc,so we use removed flag to flag it
        while(mRunnables.size() && ((mFlags&1)==0)){
            RUNNER runner = mRunnables.front();
            if(runner.time > nowms)break;
            mRunnables.pop_front();
            if(runner.run)runner.run();
            count++;
        }
        if(((mFlags&1)==0)&&mAttachedView->isLayoutRequested())
            mLayoutRunner();
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
bool UIEventSource::postDelayed(const Runnable& run,long delayedtime){
    RUNNER runner;
    runner.run = run;
    runner.time = SystemClock::uptimeMillis() + delayedtime;

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
    for(auto it = mRunnables.begin();it != mRunnables.end();){
        if(it->run == what){
            it = mRunnables.erase(it);
            count++;
        }else it++;
    }
    return count;
}

}//end namespace
