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
#include <view/handleractionqueue.h>
#include <core/uieventsource.h>
namespace cdroid{

HandlerActionQueue::HandlerActionQueue(){
}

HandlerActionQueue::~HandlerActionQueue(){
    for(auto action:mActions){
        delete action;
    }
}

void HandlerActionQueue::post(const Runnable& action) {
    postDelayed(action, 0);
}

void HandlerActionQueue::postDelayed(const Runnable& action, long delayMillis) {
    HandlerAction* handlerAction = new HandlerAction(action, delayMillis);
    mActions.push_back(handlerAction);
}

void HandlerActionQueue::removeCallbacks(const Runnable& action) {
    const size_t count = mActions.size();
    size_t j = 0;
    for (size_t i = 0; i < count; i++) {
        if (mActions[i]->matches(action)) {
            // Remove this action by overwriting it within
            // this loop or nulling it out later.
            continue;
        }
        if (j != i) {
            // At least one previous entry was removed, so
            // this one needs to move to the "new" list.
            mActions[j] = mActions[i];
        }
        j++;
    }

    // The "new" list only has j entries.
    // Null out any remaining entries.
    const int finalCount = j;
    for (; j < count; j++) {
        delete mActions[j];
        mActions[j] = nullptr;
    }
    mActions.resize(finalCount);
}

void HandlerActionQueue::executeActions(UIEventSource& handler) {
    for (size_t i = 0, count = mActions.size(); i < count; i++) {
        HandlerAction* handlerAction = mActions[i];
        handler.postDelayed(handlerAction->action, handlerAction->delay);
    }
    mActions.clear();
}

int HandlerActionQueue::size() const{
    return mActions.size();
}

Runnable& HandlerActionQueue::getRunnable(int index) {
    FATAL_IF(index >= mActions.size(),"IndexOutOfBoundsException");
    return mActions[index]->action;
}

long HandlerActionQueue::getDelay(int index) const{
    FATAL_IF(index >= mActions.size(),"IndexOutOfBoundsException");
    return mActions[index]->delay;
}

HandlerActionQueue::HandlerAction::HandlerAction(const Runnable& action, long delay) {
    this->action = action;
    this->delay = delay;
}

bool HandlerActionQueue::HandlerAction::matches(const Runnable& otherAction)const {
    return ((otherAction == nullptr) && (action == nullptr))
            || ((action != nullptr) && (action==otherAction));
}

}/*endof namespace*/
