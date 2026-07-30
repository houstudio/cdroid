#ifndef __DEFAULTLIFECYCLEOBSERVER_H__
#define __DEFAULTLIFECYCLEOBSERVER_H__
/*********************************************************************************
 * Port of androidx.lifecycle.DefaultLifecycleObserver. Default no-op callbacks;
 * override only the ones you need.
 *********************************************************************************/
#include <lifecycle/lifecycleobserver.h>
#include <lifecycle/lifecycleowner.h>
namespace cdroid{
namespace lifecycle{

class DefaultLifecycleObserver : public LifecycleObserver{
public:
    virtual void onCreate(LifecycleOwner*) {}
    virtual void onStart(LifecycleOwner*) {}
    virtual void onResume(LifecycleOwner*) {}
    virtual void onPause(LifecycleOwner*) {}
    virtual void onStop(LifecycleOwner*) {}
    virtual void onDestroy(LifecycleOwner*) {}
};

}//namespace lifecycle
}//namespace cdroid
#endif
