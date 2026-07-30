#ifndef __LIFECYCLEOWNER_H__
#define __LIFECYCLEOWNER_H__
/*********************************************************************************
 * Port of androidx.lifecycle.LifecycleOwner.
 *********************************************************************************/
#include <lifecycle/lifecycle.h>
namespace cdroid{
namespace lifecycle{

class LifecycleOwner{
public:
    virtual ~LifecycleOwner() = default;
    virtual Lifecycle& getLifecycle() = 0;
};

}//namespace lifecycle
}//namespace cdroid
#endif
