#ifndef __LIFECYCLEOBSERVER_H__
#define __LIFECYCLEOBSERVER_H__
/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * Port of androidx.lifecycle.LifecycleObserver. Marker interface: do not
 * implement directly, implement DefaultLifecycleObserver or
 * LifecycleEventObserver instead.
 *********************************************************************************/
namespace cdroid{
namespace lifecycle{

class LifecycleObserver{
public:
    virtual ~LifecycleObserver() = default;
};

}//namespace lifecycle
}//namespace cdroid
#endif
