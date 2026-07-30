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
