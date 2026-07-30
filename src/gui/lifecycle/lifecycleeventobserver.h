#ifndef __LIFECYCLEEVENTOBSERVER_H__
#define __LIFECYCLEEVENTOBSERVER_H__
/*********************************************************************************
 * Port of androidx.lifecycle.LifecycleEventObserver.
 *********************************************************************************/
#include <lifecycle/lifecycle.h>
#include <lifecycle/lifecycleobserver.h>
namespace cdroid{
namespace lifecycle{

class LifecycleEventObserver : public LifecycleObserver{
public:
    virtual void onStateChanged(LifecycleOwner* source, Lifecycle::Event event) = 0;
};

}//namespace lifecycle
}//namespace cdroid
#endif
