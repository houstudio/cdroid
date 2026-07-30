#ifndef __LIFECYCLING_H__
#define __LIFECYCLING_H__
/*********************************************************************************
 * Port of androidx.lifecycle.Lifecycling (internal). Resolves an arbitrary
 * LifecycleObserver into a unified LifecycleEventObserver. Reflective
 * @OnLifecycleEvent observers are intentionally not supported.
 *********************************************************************************/
#include <string>
#include <lifecycle/lifecycleeventobserver.h>
#include <lifecycle/lifecycleobserver.h>
namespace cdroid{
namespace lifecycle{

class Lifecycling{
public:
    /** Adapts DefaultLifecycleObserver / LifecycleEventObserver into a single
     *  LifecycleEventObserver. Returns the same pointer for a LifecycleEventObserver
     *  (not owned) or a new adapter for a DefaultLifecycleObserver (caller-owned). */
    static LifecycleEventObserver* lifecycleEventObserver(LifecycleObserver* object);

    static std::string getAdapterName(const std::string& className);
};

}//namespace lifecycle
}//namespace cdroid
#endif
