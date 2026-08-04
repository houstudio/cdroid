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
#include <lifecycle/viewmodelstore.h>
#include <vector>

namespace cdroid{
namespace lifecycle{

ViewModelStore::~ViewModelStore(){
    clear();
}

void ViewModelStore::put(const std::string& key, ViewModel* viewModel){
    auto it = mMap.find(key);
    ViewModel* old = (it != mMap.end()) ? it->second : nullptr;
    mMap[key] = viewModel;
    if(old){ old->clear(); delete old; }
}

ViewModel* ViewModelStore::get(const std::string& key){
    auto it = mMap.find(key);
    return it == mMap.end() ? nullptr : it->second;
}

void ViewModelStore::clear(){
    std::vector<ViewModel*> snapshot;
    snapshot.reserve(mMap.size());
    for(auto& kv : mMap) snapshot.push_back(kv.second);
    mMap.clear();
    for(auto* vm : snapshot){ vm->clear(); delete vm; }
}

}//namespace lifecycle
}//namespace cdroid
