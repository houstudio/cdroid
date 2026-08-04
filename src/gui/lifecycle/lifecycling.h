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
