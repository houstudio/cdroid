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
#include <fragment/fragmentviewlifecycleowner.h>
namespace cdroid{
namespace fragment{

FragmentViewLifecycleOwner::FragmentViewLifecycleOwner(Fragment* /*fragment*/){
    mLifecycleRegistry = new lifecycle::LifecycleRegistry(this);
    mViewModelStore = new lifecycle::ViewModelStore();
    mSavedStateRegistryController = new savedstate::SavedStateRegistryController(this);
}

FragmentViewLifecycleOwner::~FragmentViewLifecycleOwner(){
    delete mLifecycleRegistry;
    if(mViewModelStore) mViewModelStore->clear();
    delete mViewModelStore;
    delete mSavedStateRegistryController;
}

lifecycle::Lifecycle& FragmentViewLifecycleOwner::getLifecycle(){
    return *mLifecycleRegistry;
}

lifecycle::ViewModelStore& FragmentViewLifecycleOwner::getViewModelStore(){
    return *mViewModelStore;
}

savedstate::SavedStateRegistry& FragmentViewLifecycleOwner::getSavedStateRegistry(){
    return mSavedStateRegistryController->getSavedStateRegistry();
}

lifecycle::ViewModelProvider::Factory& FragmentViewLifecycleOwner::getDefaultViewModelProviderFactory(){
    return lifecycle::HasDefaultViewModelProviderFactory::getDefaultViewModelProviderFactory();
}

void FragmentViewLifecycleOwner::handleLifecycleEvent(lifecycle::Lifecycle::Event event){
    mLifecycleRegistry->handleLifecycleEvent(event);
}

}//namespace fragment
}//namespace cdroid
