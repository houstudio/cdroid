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
#include <fragment/fragment.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenthostcallback.h>
#include <fragment/fragmentviewlifecycleowner.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <view/layoutinflater.h>
#include <widget/cdwindow.h>
#include <core/bundle.h>
#include <core/context.h>
#include <atomic>
#include <stdexcept>

namespace cdroid{
namespace fragment{

std::string Fragment::generateWho(){
    static std::atomic<long> sCounter{0};
    return "android:fragment:" + std::to_string(++sCounter);
}

namespace{
// Nested-fragment host: a Fragment's childFragmentManager sees this Fragment as host.
// Child fragments live inside this Fragment's own view (whose id == getId()).
class FragmentChildHost : public FragmentHostCallback{
    Fragment* mFragment;
public:
    explicit FragmentChildHost(Fragment* f) : mFragment(f){}
    cdroid::Context* getContext() override { return mFragment->getContext(); }
    // Reuse the host chain's Handler (the activity's) rather than minting one. A nested child
    // FragmentManager resolves the same host Handler up to the activity, matching androidx.
    // Needed so a deferred child-FM commit (NavHostFragment.onCreate -> setGraph -> navigate)
    // can reach the looper via mHost->getHandler().
    cdroid::Handler* getHandler() override {
        return mFragment->mHost ? mFragment->mHost->getHandler() : nullptr;
    }
    cdroid::LayoutInflater* onGetLayoutInflater() override { return cdroid::LayoutInflater::from(getContext()); }
    cdroid::Window* onGetHost() override { return mFragment->mHost ? mFragment->mHost->onGetHost() : nullptr; }
    cdroid::View* onFindViewById(int id) override {
        if(mFragment->mView && id == mFragment->getId()) return mFragment->mView;
        return mFragment->mView ? mFragment->mView->findViewById(id) : nullptr;
    }
    bool onHasView() override { return mFragment->mView != nullptr; }
};
}//anonymous

Fragment::Fragment(){
    mWho = generateWho();
    mLifecycleRegistry = new lifecycle::LifecycleRegistry(this);
    mSavedStateRegistryController = new savedstate::SavedStateRegistryController(this);
    mViewModelStore = new lifecycle::ViewModelStore();
    mChildFragmentManager.reset(new FragmentManager());
}

Fragment::Fragment(int contentLayoutId) : Fragment(){
    mContentLayoutId = contentLayoutId;
}

Fragment::~Fragment(){
    // The 6 Fragment Transitions are owned (androidx holds them in fields; GC reclaims). CDROID has
    // no GC, so ~Fragment must delete them. collectEffects clones them (mEnterTransition->clone())
    // into a TransitionEffect; the clone self-deletes (setDeleteWhenEnded). This original/template
    // transition is owned by the Fragment.
    delete mEnterTransition;
    delete mExitTransition;
    delete mReenterTransition;
    delete mReturnTransition;
    delete mSharedElementEnterTransition;
    delete mSharedElementReturnTransition;
    delete mLifecycleRegistry;
    delete mSavedStateRegistryController;
    delete mViewModelStore;
    delete mDefaultFactory;
    delete mChildHost;
    delete mViewLifecycleOwner;
    // mChildFragmentManager (unique_ptr) releases automatically.
}

// Fragment owns its 6 Transition* (androidx fields; GC reclaims there). On replace, delete the
// previous; ~Fragment deletes whatever remains. collectEffects clones these (->clone() into a
// TransitionEffect); the clone self-deletes via setDeleteWhenEnded.
void Fragment::setEnterTransition(Transition* t){
    if(mEnterTransition != t){
        delete mEnterTransition;
        mEnterTransition = t;
    }
}
void Fragment::setExitTransition(Transition* t){
    if(mExitTransition != t){
        delete mExitTransition;
        mExitTransition = t;
    }
}
void Fragment::setReenterTransition(Transition* t){
    if(mReenterTransition != t){
        delete mReenterTransition;
        mReenterTransition = t;
    }
}
void Fragment::setReturnTransition(Transition* t){
    if(mReturnTransition != t){
        delete mReturnTransition;
        mReturnTransition = t;
    }
}

// --- owner interface ---
lifecycle::Lifecycle& Fragment::getLifecycle(){
    return *mLifecycleRegistry;
}

lifecycle::ViewModelStore& Fragment::getViewModelStore(){
    return *mViewModelStore;
}

savedstate::SavedStateRegistry& Fragment::getSavedStateRegistry(){
    return mSavedStateRegistryController->getSavedStateRegistry();
}

lifecycle::ViewModelProvider::Factory& Fragment::getDefaultViewModelProviderFactory(){
    if(mDefaultFactory) return *mDefaultFactory;
    return lifecycle::HasDefaultViewModelProviderFactory::getDefaultViewModelProviderFactory();
}

// --- accessors ---
cdroid::Context* Fragment::getContext(){
    if(mHost) return mHost->getContext();
    if(mParentFragment) return mParentFragment->getContext();
    return nullptr;
}
cdroid::Context* Fragment::requireContext(){
    cdroid::Context* c = getContext();
    if(!c) throw std::runtime_error("Fragment " + mWho + " not attached to a context");
    return c;
}
cdroid::View* Fragment::requireView(){
    if(!mView) throw std::runtime_error("Fragment " + mWho + " did not return a View");
    return mView;
}
void Fragment::setArguments(cdroid::Bundle* args){
    if(mFragmentManager != nullptr) throw std::runtime_error("Fragment already active");
    delete mArguments;
    mArguments = args;
}
FragmentManager* Fragment::getParentFragmentManager(){
    if(mFragmentManager == nullptr){
        throw std::runtime_error("Fragment " + mWho + " not associated with a FragmentManager");
    }
    return mFragmentManager;
}

FragmentManager* Fragment::getChildFragmentManager(){
    return mChildFragmentManager.get();
}

bool Fragment::isVisible() const{
    return mHost != nullptr && !mHidden && mView != nullptr && mUserVisibleHint;
}

// --- perform* (lifecycle dispatch + nested childFragmentManager) ---
void Fragment::performAttach(){
    onAttach(getContext());
    // Attach the child FragmentManager here (androidx attaches it during onAttach) so it has a
    // live host — and thus a resolvable Handler — before onCreate. NavHostFragment::onCreate
    // commits on the child FM (setGraph -> navigate); with a deferred commit that commit must be
    // able to reach the host Handler. Attaching in performCreate (the old site, after onCreate)
    // left the child FM's mHost null during onCreate.
    if(mChildFragmentManager && !mChildHost){
        mChildHost = new FragmentChildHost(this);
        mChildFragmentManager->attachController(mChildHost, mChildHost, this);
    }
}

void Fragment::performCreate(cdroid::Bundle* savedInstanceState){
    onCreate(savedInstanceState);
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_CREATE);
    mIsCreated = true;
    if(mChildFragmentManager){
        mChildFragmentManager->dispatchCreate();
    }
}

void Fragment::performCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                                 cdroid::Bundle* savedInstanceState){
    mView = onCreateView(inflater, container, savedInstanceState);
    // The view-scoped lifecycle owner is created with the view.
    if(!mViewLifecycleOwner) mViewLifecycleOwner = new FragmentViewLifecycleOwner(this);
    mViewLifecycleOwner->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_CREATE);
}

void Fragment::performViewCreated(cdroid::Bundle* savedInstanceState){
    onViewCreated(mView, savedInstanceState);
}

void Fragment::performActivityCreated(cdroid::Bundle* savedInstanceState){
    onActivityCreated(savedInstanceState);
    if(mChildFragmentManager) mChildFragmentManager->dispatchActivityCreated();
}

void Fragment::performStart(){
    onStart();
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_START);
    if(mViewLifecycleOwner) mViewLifecycleOwner->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_START);
    if(mChildFragmentManager) mChildFragmentManager->dispatchStart();
}

void Fragment::performResume(){
    onResume();
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_RESUME);
    if(mViewLifecycleOwner) mViewLifecycleOwner->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_RESUME);
    if(mChildFragmentManager) mChildFragmentManager->dispatchResume();
}

void Fragment::performPause(){
    if(mChildFragmentManager) mChildFragmentManager->dispatchPause();
    if(mViewLifecycleOwner) mViewLifecycleOwner->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_PAUSE);
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_PAUSE);
    onPause();
}

void Fragment::performStop(){
    if(mChildFragmentManager) mChildFragmentManager->dispatchStop();
    if(mViewLifecycleOwner) mViewLifecycleOwner->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_STOP);
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_STOP);
    onStop();
}

void Fragment::performDestroyView(){
    if(mChildFragmentManager) mChildFragmentManager->dispatchDestroyView();
    onDestroyView();
    if(mViewLifecycleOwner){
        mViewLifecycleOwner->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_DESTROY);
        delete mViewLifecycleOwner;
        mViewLifecycleOwner = nullptr;
    }
}

void Fragment::performDestroy(){
    if(mChildFragmentManager) mChildFragmentManager->dispatchDestroy();
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_DESTROY);
    onDestroy();
    mIsCreated = false;
}

void Fragment::performDetach(){
    onDetach();
}

// --- options-menu participation (androidx Fragment) ---
bool Fragment::isMenuVisible() const {
    return mMenuVisible && (mFragmentManager == nullptr
           || mFragmentManager->isParentMenuVisible(mParentFragment));
}

void Fragment::setHasOptionsMenu(bool hasMenu){
    if(mHasMenu != hasMenu){
        mHasMenu = hasMenu;
        // androidx: if(isAdded() && !isHidden()) mHost.onSupportInvalidateOptionsMenu();
        if(mHost && !mHidden) mHost->onSupportInvalidateOptionsMenu();
    }
}

void Fragment::setMenuVisibility(bool menuVisible){
    if(mMenuVisible != menuVisible){
        mMenuVisible = menuVisible;
        if(mHasMenu && mHost && !mHidden) mHost->onSupportInvalidateOptionsMenu();
    }
}

bool Fragment::performCreateOptionsMenu(Menu& menu, MenuInflater& inflater){
    bool show = false;
    if(!mHidden){
        if(mHasMenu && mMenuVisible){
            show = true;
            onCreateOptionsMenu(menu, inflater);
        }
        if(mChildFragmentManager) show |= mChildFragmentManager->dispatchCreateOptionsMenu(menu, inflater);
    }
    return show;
}

bool Fragment::performPrepareOptionsMenu(Menu& menu){
    bool show = false;
    if(!mHidden){
        if(mHasMenu && mMenuVisible){
            show = true;
            onPrepareOptionsMenu(menu);
        }
        if(mChildFragmentManager) show |= mChildFragmentManager->dispatchPrepareOptionsMenu(menu);
    }
    return show;
}

bool Fragment::performOptionsItemSelected(MenuItem& item){
    if(!mHidden){
        if(mHasMenu && mMenuVisible){
            if(onOptionsItemSelected(item)) return true;
        }
        if(mChildFragmentManager) return mChildFragmentManager->dispatchOptionsItemSelected(item);
    }
    return false;
}

bool Fragment::performContextItemSelected(MenuItem& item){
    if(!mHidden){
        if(onContextItemSelected(item)) return true;
        if(mChildFragmentManager) return mChildFragmentManager->dispatchContextItemSelected(item);
    }
    return false;
}

void Fragment::performOptionsMenuClosed(Menu& menu){
    if(!mHidden){
        if(mHasMenu && mMenuVisible){
            onOptionsMenuClosed(menu);
        }
        if(mChildFragmentManager) mChildFragmentManager->dispatchOptionsMenuClosed(menu);
    }
}

}//namespace fragment
}//namespace cdroid
