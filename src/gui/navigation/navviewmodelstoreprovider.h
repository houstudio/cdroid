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
#ifndef __NAVVIEWMODELSTOREPROVIDER_H__
#define __NAVVIEWMODELSTOREPROVIDER_H__
/*********************************************************************************
 * Port of androidx.navigation.NavViewModelStoreProvider. Provides a ViewModelStore
 * per NavBackStackEntry id (used by NavController to scope VMs to back-stack entries).
 *********************************************************************************/
#include <string>
#include <lifecycle/viewmodelstore.h>
namespace cdroid{

class NavViewModelStoreProvider{
public:
    virtual ~NavViewModelStoreProvider() = default;
    virtual lifecycle::ViewModelStore* get(const std::string& key) = 0;
    virtual void clear(const std::string& key) = 0;
};

}//namespace cdroid
#endif
