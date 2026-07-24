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
#ifndef __UIEVENT_SOURCE_H__
#define __UIEVENT_SOURCE_H__
#include <core/looper.h>
#include <view/view.h>
namespace cdroid{

// UIEventSource 现为纯帧驱动 EventHandler: 仅负责 layout/draw/compose (checkEvents/handleEvents)。
// View::post/postDelayed 的延迟 runnable 不再经此对象, 改由 AttachInfo::mHandler 直接投到主 looper 的
// MessageQueue, 使阻塞主循环 (Looper::loopOnce -> MessageQueue::next) 能按 msg.when 计时唤醒并派发。
class UIEventSource:public EventHandler{
private:
    Runnable mLayoutRunner;
    ViewGroup*mAttachedView;
    void handleCompose();
    int handleRunnables();
public:
    UIEventSource(View*,const Runnable& run);
    ~UIEventSource()override;
    void cleanUp();
    bool processEvents();
    int checkEvents()override;
    int handleEvents()override;
};

}//end namespace

#endif
