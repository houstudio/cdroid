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
#include <lifecycle/lifecycling.h>
#include <lifecycle/defaultlifecycleobserver.h>
#include <lifecycle/lifecycleowner.h>

namespace cdroid{
namespace lifecycle{
namespace{
// Wraps a DefaultLifecycleObserver, dispatching each event to the matching callback. Mirrors
// androidx DefaultLifecycleObserverAdapter: when the wrapped object ALSO implements
// LifecycleEventObserver, onStateChanged is dispatched to BOTH the per-callback AND the
// LifecycleEventObserver.onStateChanged (reflective branch omitted).
class DefaultLifecycleObserverAdapter : public LifecycleEventObserver{
    DefaultLifecycleObserver* mObserver;
    LifecycleEventObserver* mEventObserver; // non-null when the object also is-a LifecycleEventObserver
public:
    DefaultLifecycleObserverAdapter(DefaultLifecycleObserver* o, LifecycleEventObserver* e)
        : mObserver(o), mEventObserver(e) {}
    void onStateChanged(LifecycleOwner* owner, Lifecycle::Event event) override{
        switch(event){
            case Lifecycle::Event::ON_CREATE:  mObserver->onCreate(owner); break;
            case Lifecycle::Event::ON_START:   mObserver->onStart(owner);  break;
            case Lifecycle::Event::ON_RESUME:  mObserver->onResume(owner); break;
            case Lifecycle::Event::ON_PAUSE:   mObserver->onPause(owner);  break;
            case Lifecycle::Event::ON_STOP:    mObserver->onStop(owner);   break;
            case Lifecycle::Event::ON_DESTROY: mObserver->onDestroy(owner);break;
            case Lifecycle::Event::ON_ANY: break;
        }
        // androidx: after the DefaultLifecycleObserver per-callback, also forward to the
        // LifecycleEventObserver side if the object implements it (both-interfaces observer).
        if(mEventObserver) mEventObserver->onStateChanged(owner, event);
    }
};
}//anonymous

LifecycleEventObserver* Lifecycling::lifecycleEventObserver(LifecycleObserver* object){
    // Mirrors androidx Lifecycling.jvm.kt:
    //  - both DefaultLifecycleObserver AND LifecycleEventObserver -> adapter(dlo, leo) dispatches BOTH;
    //  - DefaultLifecycleObserver only -> adapter(dlo, null) -> per-callback;
    //  - LifecycleEventObserver only -> as-is -> onStateChanged.
    // (The old code short-circuited on LifecycleEventObserver, so a both-interfaces observer only
    // received onStateChanged and its DefaultLifecycleObserver per-callbacks were never invoked.)
    // dynamic_cast (not static_cast) is required: LifecycleObserver is a virtual base of both
    // DefaultLifecycleObserver and LifecycleEventObserver, so base->derived needs runtime offset.
    DefaultLifecycleObserver* dlo = dynamic_cast<DefaultLifecycleObserver*>(object);
    LifecycleEventObserver* leo = dynamic_cast<LifecycleEventObserver*>(object);
    if(dlo && leo)
        return new DefaultLifecycleObserverAdapter(dlo, leo);
    if(dlo)
        return new DefaultLifecycleObserverAdapter(dlo, nullptr);
    if(leo)
        return leo;
    return nullptr; // unsupported observer kind (no reflective @OnLifecycleEvent)
}

std::string Lifecycling::getAdapterName(const std::string& className){
    return className + "_LifecycleAdapter";
}

}//namespace lifecycle
}//namespace cdroid
