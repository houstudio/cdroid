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
#include <fragment/fragmentactivity.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenthostcallback.h>
#include <view/layoutinflater.h>
#include <view/viewgroup.h>
#include <core/context.h>
#include <core/handler.h>

namespace cdroid{
namespace fragment{

// FragmentHostCallback implementation bound to the FragmentActivity.
class FragmentActivity::HostCallbacks : public FragmentHostCallback{
    FragmentActivity* mActivity;
public:
    explicit HostCallbacks(FragmentActivity* a) : mActivity(a){}
    cdroid::Context* getContext() override { return mActivity->getContext(); }
    cdroid::Handler* getHandler() override {
        // FragmentManager dispatch is synchronous (no Handler.post scheduling),
        // so no real Handler is needed yet. Wired up with OnBackPressedDispatcher later.
        return nullptr;
    }
    cdroid::LayoutInflater* onGetLayoutInflater() override {
        return cdroid::LayoutInflater::from(getContext());
    }
    cdroid::Window* onGetHost() override { return mActivity; }
    cdroid::View* onFindViewById(int id) override {
        if(id == mActivity->getFragmentContainerId()) return mActivity;
        return mActivity->findViewById(id);
    }
    bool onHasView() override { return true; }
};

FragmentActivity::FragmentActivity(int x, int y, int w, int h)
    : Activity(x, y, w, h){
    // android.R.id.content analogue: the id fragments are added into.
    mContainerId = 0x01020002;
    setId(mContainerId);
    mLifecycleRegistry = new lifecycle::LifecycleRegistry(this);
    mViewModelStore = new lifecycle::ViewModelStore();
    mSavedStateRegistryController = new savedstate::SavedStateRegistryController(this);
    mFragmentManager = new FragmentManager();
    mHost = new HostCallbacks(this);
    mFragmentManager->attachController(mHost, mHost, nullptr);
}

FragmentActivity::~FragmentActivity(){
    delete mFragmentManager;
    delete mHost;
    delete mSavedStateRegistryController;
    delete mViewModelStore;
    delete mLifecycleRegistry;
}

lifecycle::Lifecycle& FragmentActivity::getLifecycle(){
    return *mLifecycleRegistry;
}

lifecycle::ViewModelStore& FragmentActivity::getViewModelStore(){
    return *mViewModelStore;
}
savedstate::SavedStateRegistry& FragmentActivity::getSavedStateRegistry(){
    return mSavedStateRegistryController->getSavedStateRegistry();
}

void FragmentActivity::onCreate(Bundle* savedInstanceState){
    Activity::onCreate(savedInstanceState);
    mFragmentManager->dispatchAttach();
    mFragmentManager->dispatchCreate();
    mFragmentManager->dispatchViewCreated();
    mFragmentManager->dispatchActivityCreated();
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_CREATE);
}

void FragmentActivity::onStart(){
    Activity::onStart();
    mFragmentManager->dispatchStart();
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_START);
}

void FragmentActivity::onResume(){
    Activity::onResume();
    mFragmentManager->dispatchResume();
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_RESUME);
}

void FragmentActivity::onPause(){
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_PAUSE);
    mFragmentManager->dispatchPause();
    Activity::onPause();
}

void FragmentActivity::onStop(){
    mFragmentManager->dispatchStop();
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_STOP);
    Activity::onStop();
}

void FragmentActivity::onDestroy(){
    mFragmentManager->dispatchDestroyView();
    mFragmentManager->dispatchDestroy();
    Activity::onDestroy();
}

void FragmentActivity::onBackPressed(){
    if(mFragmentManager->popBackStackImmediate()) return;
    Activity::onBackPressed();
}

FragmentManager* FragmentActivity::getSupportFragmentManager(){
    return mFragmentManager;
}

}//namespace fragment
}//namespace cdroid
