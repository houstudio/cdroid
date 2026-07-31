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
#include <lifecycle/lifecycleregistry.h>
#include <core/looper.h>
#include <algorithm>
#include <stdexcept>
#include <porting/cdlog.h>

namespace cdroid{
namespace lifecycle{
namespace{
bool isMainThread(){
    return cdroid::Looper::getMainLooper() == cdroid::Looper::myLooper();
}
}//anonymous

LifecycleRegistry::ObserverWithState::ObserverWithState(LifecycleObserver* o, State initial)
    : observer(o), state(initial), lifecycleObserver(nullptr), ownsLifecycleObserver(false){
    lifecycleObserver = Lifecycling::lifecycleEventObserver(o);
    // A DefaultLifecycleObserver is wrapped in a fresh adapter we own; a plain
    // LifecycleEventObserver is returned as-is (not owned).
    ownsLifecycleObserver = (dynamic_cast<DefaultLifecycleObserver*>(o) != nullptr);
}

LifecycleRegistry::ObserverWithState::~ObserverWithState(){
    if(ownsLifecycleObserver) delete lifecycleObserver;
}

void LifecycleRegistry::ObserverWithState::dispatchEvent(LifecycleOwner* owner, Event event){
    State newState = Lifecycle::getTargetState(event);
    state = std::min(state, newState);
    if(lifecycleObserver) lifecycleObserver->onStateChanged(owner, event);
    state = newState;
}

LifecycleRegistry::LifecycleRegistry(LifecycleOwner* provider)
    : LifecycleRegistry(provider, true){}

LifecycleRegistry::LifecycleRegistry(LifecycleOwner* provider, bool enforceMainThread)
    : mLifecycleOwner(provider), mEnforceMainThread(enforceMainThread){}

LifecycleRegistry* LifecycleRegistry::createUnsafe(LifecycleOwner* owner){
    return new LifecycleRegistry(owner, false);
}

void LifecycleRegistry::enforceMainThreadIfNeeded(const char* method){
    if(mEnforceMainThread && !isMainThread()){
        LOGE("Method %s must be called on the main thread", method);
    }
}

void LifecycleRegistry::checkLifecycleStateTransition(State current, State next){
    if(current == State::INITIALIZED && next == State::DESTROYED){
        throw std::runtime_error("State must be at least CREATED to be moved to DESTROYED");
    }
    if(current == State::DESTROYED && current != next){
        throw std::runtime_error("State is DESTROYED and cannot be moved to a new state");
    }
}

void LifecycleRegistry::markState(State state){
    enforceMainThreadIfNeeded("markState");
    setCurrentState(state);
}

void LifecycleRegistry::setCurrentState(State state){
    enforceMainThreadIfNeeded("setCurrentState");
    moveToState(state);
}

void LifecycleRegistry::handleLifecycleEvent(Event event){
    enforceMainThreadIfNeeded("handleLifecycleEvent");
    moveToState(Lifecycle::getTargetState(event));
}

Lifecycle::State LifecycleRegistry::getCurrentState() const{
    return mInternalState;
}

int LifecycleRegistry::getObserverCount(){
    enforceMainThreadIfNeeded("getObserverCount");
    return mObserverMap.size();
}

bool LifecycleRegistry::isSynced() const{
    if(mObserverMap.size() == 0) return true;
    State eldest = mObserverMap.first()->value->state;
    State newest = mObserverMap.last()->value->state;
    return eldest == newest && mInternalState == newest;
}

Lifecycle::State LifecycleRegistry::calculateTargetState(LifecycleObserver* observer) const{
    ObserverMap::Entry* sibling = mObserverMap.ceil(observer); // predecessor of observer
    State siblingState = sibling ? sibling->value->state : State::RESUMED;
    State parentState = mParentStates.empty() ? State::RESUMED : mParentStates.back();
    return std::min({mInternalState, siblingState, parentState});
}

void LifecycleRegistry::addObserver(LifecycleObserver* observer){
    enforceMainThreadIfNeeded("addObserver");
    State initialState = (mInternalState == State::DESTROYED) ? State::DESTROYED : State::INITIALIZED;
    auto stateful = std::make_shared<ObserverWithState>(observer, initialState);
    if(mObserverMap.putIfAbsent(observer, stateful) != nullptr){
        return; // already registered
    }
    if(!mLifecycleOwner) return; // owner gone (defensive; does not happen in C++)
    bool isReentrance = (mAddingObserverCounter != 0) || mHandlingEvent;
    State targetState = calculateTargetState(observer);
    mAddingObserverCounter++;
    auto it = mObserverMap.ceil(observer); // re-resolve the stored handle after put
    // Drive the new observer up to targetState.
    while(stateful->state < targetState && mObserverMap.contains(observer)){
        mParentStates.push_back(stateful->state);
        Event event;
        if(!Lifecycle::upFrom(stateful->state, &event)){
            LOGE("no event up from state %d", (int)stateful->state);
            mParentStates.pop_back();
            break;
        }
        stateful->dispatchEvent(mLifecycleOwner, event);
        mParentStates.pop_back();
        targetState = calculateTargetState(observer);
    }
    if(!isReentrance){
        sync();
    }
    mAddingObserverCounter--;
    (void)it;
}

void LifecycleRegistry::removeObserver(LifecycleObserver* observer){
    enforceMainThreadIfNeeded("removeObserver");
    // No destruction events dispatched (matches upstream). The shared_ptr held by
    // the entry keeps the ObserverWithState alive for any in-flight dispatch.
    mObserverMap.remove(observer);
}

void LifecycleRegistry::moveToState(State next){
    if(mInternalState == next){
        return;
    }
    checkLifecycleStateTransition(mInternalState, next);
    mInternalState = next;
    if(mHandlingEvent || mAddingObserverCounter != 0){
        mNewEventOccurred = true;
        return; // active top-level loop will restart sync
    }
    mHandlingEvent = true;
    sync();
    mHandlingEvent = false;
    if(mInternalState == State::DESTROYED){
        mObserverMap.clearAll(); // drop all observers (equivalent to observerMap = new())
    }
}

void LifecycleRegistry::forwardPass(LifecycleOwner* owner){
    mObserverMap.forEachWithAdditions([this, owner](ObserverMap::Entry& entry){
        std::shared_ptr<ObserverWithState> ows = entry.value;
        while(ows->state < mInternalState && !mNewEventOccurred && mObserverMap.contains(entry.key)){
            mParentStates.push_back(ows->state);
            Event event;
            if(!Lifecycle::upFrom(ows->state, &event)){
                LOGE("no event up from state %d", (int)ows->state);
                mParentStates.pop_back();
                break;
            }
            ows->dispatchEvent(owner, event);
            mParentStates.pop_back();
        }
    });
}

void LifecycleRegistry::backwardPass(LifecycleOwner* owner){
    mObserverMap.forEachReversed([this, owner](ObserverMap::Entry& entry){
        std::shared_ptr<ObserverWithState> ows = entry.value;
        while(ows->state > mInternalState && !mNewEventOccurred && mObserverMap.contains(entry.key)){
            Event event;
            if(!Lifecycle::downFrom(ows->state, &event)){
                LOGE("no event down from state %d", (int)ows->state);
                break;
            }
            mParentStates.push_back(Lifecycle::getTargetState(event));
            ows->dispatchEvent(owner, event);
            mParentStates.pop_back();
        }
    });
}

void LifecycleRegistry::sync(){
    if(!mLifecycleOwner){
        throw std::runtime_error("LifecycleOwner of this LifecycleRegistry is gone; "
                                 "too late to change lifecycle state.");
    }
    while(!isSynced()){
        mNewEventOccurred = false;
        // If registry state is lower than the oldest observer, bring observers down.
        if(mInternalState < mObserverMap.first()->value->state){
            backwardPass(mLifecycleOwner);
        }
        ObserverMap::Entry* lastEntry = mObserverMap.last();
        // If registry state is higher than the newest observer, bring observers up.
        if(!mNewEventOccurred && lastEntry && mInternalState > lastEntry->value->state){
            forwardPass(mLifecycleOwner);
        }
    }
    mNewEventOccurred = false;
}

}//namespace lifecycle
}//namespace cdroid
