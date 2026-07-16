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

namespace cdroid{

UIEventSource::UIEventSource(View*v,const Runnable&r):mLayoutRunner(r){
    mAttachedView = dynamic_cast<ViewGroup*>(v);
    setOwned(true);
}

UIEventSource::~UIEventSource(){
}

void UIEventSource::cleanUp(){
    // 延迟 runnable 已归 AttachInfo::mHandler (主 looper MessageQueue) 承载, 本对象不再持有待派发队列。
}

int UIEventSource::checkEvents(){
    return (mAttachedView&&mAttachedView->isDirty())
           ||mAttachedView->isLayoutRequested()
           ||GraphDevice::getInstance().needCompose();
}

void UIEventSource::handleCompose(){
#if COMPOSE_ASYNC
    GraphDevice::getInstance().lock();
    if(GraphDevice::getInstance().needCompose())
        GraphDevice::getInstance().requestCompose();
    GraphDevice::getInstance().unlock();
#else
    GraphDevice::getInstance().composeSurfaces();
#endif
}

int UIEventSource::handleRunnables(){
    int count=0;
    GraphDevice::getInstance().lock();
    if ( ((mFlags&1)==0) && mAttachedView && mAttachedView->isAttachedToWindow()){
        // 延迟 runnable 由 AttachInfo::mHandler 经 MessageQueue 派发 (Looper::loopOnce->next 排空);
        // 本处只负责 layout/draw 帧管线 (pollInner->doEventHandlers 调到这里)。
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

}//end namespace
