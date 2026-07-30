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
