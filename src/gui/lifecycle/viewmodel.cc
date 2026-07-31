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
#include <lifecycle/viewmodel.h>

namespace cdroid{
namespace lifecycle{

ViewModel::~ViewModel(){
    // Deletion timing is owned by ViewModelStore; resources are closed via clear().
}

void ViewModel::addCloseable(const std::string& key, Closeable* closeable){
    if(mCleared){
        if(closeable) closeable->close();
        return;
    }
    Closeable* previous = nullptr;
    auto it = mKeyedCloseables.find(key);
    if(it != mKeyedCloseables.end()) previous = it->second;
    mKeyedCloseables[key] = closeable;
    if(previous) previous->close();
}

void ViewModel::addCloseable(Closeable* closeable){
    if(mCleared){
        if(closeable) closeable->close();
        return;
    }
    mCloseables.push_back(closeable);
}

Closeable* ViewModel::getCloseable(const std::string& key){
    auto it = mKeyedCloseables.find(key);
    return it == mKeyedCloseables.end() ? nullptr : it->second;
}

void ViewModel::clear(){
    if(mCleared) return;
    mCleared = true;
    for(auto& kv : mKeyedCloseables){ if(kv.second) kv.second->close(); }
    mKeyedCloseables.clear();
    for(auto* c : mCloseables){ if(c) c->close(); }
    mCloseables.clear();
    onCleared();
}

}//namespace lifecycle
}//namespace cdroid
