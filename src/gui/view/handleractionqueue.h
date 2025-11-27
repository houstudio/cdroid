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
#ifndef __HANDLER_ACTION_QUEUE_H__
#define __HANDLER_ACTION_QUEUE_H__
#include <view/view.h>
namespace  cdroid{

class HandlerActionQueue {
private:
    class HandlerAction {
    public:
        Runnable action;
        long delay;
    public:
        HandlerAction(const Runnable& action, long delay);
        bool matches(const Runnable& otherAction)const;
    };
private:
    std::vector<HandlerAction*> mActions;
public:
    HandlerActionQueue();
    virtual ~HandlerActionQueue();
    void post(const Runnable& action);
    void postDelayed(const Runnable& action, long delayMillis);
    void removeCallbacks(const Runnable& action);
    void executeActions(UIEventSource& handler);
    int size()const;
    Runnable& getRunnable(int index);
    long getDelay(int index)const;
};
}/*endof namespace*/
#endif /*__HANDLER_ACTION_QUEUE_H__*/
