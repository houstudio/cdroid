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
#include <list>
#include <unordered_set>
#include <view/view.h>
namespace cdroid{

class UIEventSource:public EventHandler{
private:
    typedef struct{
        nsecs_t  time;
        Runnable run;
    }RUNNER;
    std::list<RUNNER>mRunnables;
    Runnable mLayoutRunner;
    ViewGroup*mAttachedView;
    bool hasDelayedRunners()const;
    void handleCompose();
    int handleRunnables();
public:
    UIEventSource(View*,const Runnable& run);
    ~UIEventSource()override;
    bool processEvents();
    int checkEvents()override;
    int handleEvents()override;
    bool postDelayed(const Runnable& run,long delay=0);
    int removeCallbacks(const Runnable& what);
    bool hasCallbacks(const Runnable& what)const;
};

}//end namespace

#endif
