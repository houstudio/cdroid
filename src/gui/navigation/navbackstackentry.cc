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
#include <navigation/navbackstackentry.h>
#include <atomic>

namespace cdroid{

std::string NavBackStackEntry::generateId(){
    static std::atomic<long> sCounter{0};
    return "nav-back-stack-entry:" + std::to_string(++sCounter);
}

NavBackStackEntry::NavBackStackEntry(NavDestination* destination, Bundle* arguments)
    : mDestination(destination)
    , mArguments(arguments){
    mId = generateId();
    mLifecycleRegistry = new lifecycle::LifecycleRegistry(this);
    mViewModelStore = new lifecycle::ViewModelStore();
    mSavedStateRegistryController = new savedstate::SavedStateRegistryController(this);
}

NavBackStackEntry::~NavBackStackEntry(){
    delete mLifecycleRegistry;
    if(mViewModelStore) mViewModelStore->clear();
    delete mViewModelStore;
    delete mSavedStateRegistryController;
}

lifecycle::Lifecycle& NavBackStackEntry::getLifecycle(){
    return *mLifecycleRegistry;
}

lifecycle::ViewModelStore& NavBackStackEntry::getViewModelStore(){
    return *mViewModelStore;
}

savedstate::SavedStateRegistry& NavBackStackEntry::getSavedStateRegistry(){
    return mSavedStateRegistryController->getSavedStateRegistry();
}

lifecycle::ViewModelProvider::Factory& NavBackStackEntry::getDefaultViewModelProviderFactory(){
    return lifecycle::HasDefaultViewModelProviderFactory::getDefaultViewModelProviderFactory();
}

void NavBackStackEntry::handleLifecycleEvent(lifecycle::Lifecycle::Event event){
    mLifecycleRegistry->handleLifecycleEvent(event);
}

void NavBackStackEntry::setCurrentState(lifecycle::Lifecycle::State s){
    mLifecycleRegistry->setCurrentState(s);
}

}//namespace cdroid
