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
#include <lifecycle/lifecycle.h>
#include <porting/cdlog.h>

namespace cdroid{
namespace lifecycle{

Lifecycle::State Lifecycle::getTargetState(Event event){
    switch(event){
        case Event::ON_CREATE:
        case Event::ON_STOP:    return State::CREATED;
        case Event::ON_START:
        case Event::ON_PAUSE:   return State::STARTED;
        case Event::ON_RESUME:  return State::RESUMED;
        case Event::ON_DESTROY: return State::DESTROYED;
        case Event::ON_ANY:     break;
    }
    throw std::invalid_argument("ON_ANY has no target state");
}

bool Lifecycle::downFrom(State state, Event* out){
    switch(state){
        case State::CREATED:  *out = Event::ON_DESTROY; return true;
        case State::STARTED:  *out = Event::ON_STOP;    return true;
        case State::RESUMED:  *out = Event::ON_PAUSE;   return true;
        case State::DESTROYED:
        case State::INITIALIZED: return false;
    }
    return false;
}

bool Lifecycle::downTo(State state, Event* out){
    switch(state){
        case State::DESTROYED: *out = Event::ON_DESTROY; return true;
        case State::CREATED:   *out = Event::ON_STOP;    return true;
        case State::STARTED:   *out = Event::ON_PAUSE;   return true;
        case State::INITIALIZED:
        case State::RESUMED:   return false;
    }
    return false;
}

bool Lifecycle::upFrom(State state, Event* out){
    switch(state){
        case State::INITIALIZED: *out = Event::ON_CREATE; return true;
        case State::CREATED:     *out = Event::ON_START;  return true;
        case State::STARTED:     *out = Event::ON_RESUME; return true;
        case State::DESTROYED:
        case State::RESUMED:     return false;
    }
    return false;
}

bool Lifecycle::upTo(State state, Event* out){
    switch(state){
        case State::CREATED: *out = Event::ON_CREATE; return true;
        case State::STARTED: *out = Event::ON_START;  return true;
        case State::RESUMED: *out = Event::ON_RESUME; return true;
        case State::DESTROYED:
        case State::INITIALIZED: return false;
    }
    return false;
}

}//namespace lifecycle
}//namespace cdroid
