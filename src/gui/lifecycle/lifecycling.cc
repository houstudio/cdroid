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
// Wraps a DefaultLifecycleObserver, dispatching each event to the matching
// callback. Mirrors androidx DefaultLifecycleObserverAdapter (reflective branch
// omitted).
class DefaultLifecycleObserverAdapter : public LifecycleEventObserver{
    DefaultLifecycleObserver* mObserver;
public:
    explicit DefaultLifecycleObserverAdapter(DefaultLifecycleObserver* o) : mObserver(o) {}
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
    }
};
}//anonymous

LifecycleEventObserver* Lifecycling::lifecycleEventObserver(LifecycleObserver* object){
    if(auto* e = dynamic_cast<LifecycleEventObserver*>(object)) return e;
    if(auto* d = dynamic_cast<DefaultLifecycleObserver*>(object))
        return new DefaultLifecycleObserverAdapter(d);
    return nullptr; // unsupported observer kind (no reflective @OnLifecycleEvent)
}

std::string Lifecycling::getAdapterName(const std::string& className){
    return className + "_LifecycleAdapter";
}

}//namespace lifecycle
}//namespace cdroid
