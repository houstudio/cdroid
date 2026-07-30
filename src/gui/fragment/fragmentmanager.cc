#include <fragment/fragmentmanager.h>
#include <fragment/fragment.h>
#include <fragment/fragmenthostcallback.h>
#include <fragment/fragmentcontainer.h>
#include <fragment/fragmentfactory.h>
#include <fragment/fragmenttransaction.h>
#include <fragment/backstackrecord.h>
#include <fragment/fragmentanim.h>
#include <animation/animation.h>
#include <view/view.h>
#include <view/viewgroup.h>
#include <view/layoutinflater.h>
#include <algorithm>
#include <porting/cdlog.h>

namespace cdroid{
namespace fragment{

FragmentManager::FragmentManager() = default;

FragmentManager::~FragmentManager(){
    for(BackStackRecord* r : mBackStack) delete r;
    mBackStack.clear();
    mAdded.clear();
    mActive.clear();
}

void FragmentManager::attachController(FragmentHostCallback* host, FragmentContainer* container, Fragment* parent){
    mHost = host;
    mContainer = container;
    mParent = parent;
    // androidx instanceof probes for ViewModelStoreOwner / SavedStateRegistryOwner /
    // OnBackPressedDispatcherOwner / FragmentOnAttachListener are deferred until the
    // corresponding host interfaces are wired on FragmentWindow (stage 2b-5).
}

// --- lifecycle dispatch (host -> FM -> each added fragment) ---
void FragmentManager::dispatchStateChange(int state){
    mCurState = state;
    for(Fragment* f : mAdded){
        if(f) moveToState(f, state);
    }
}

void FragmentManager::dispatchAttach(){ dispatchStateChange(Fragment::ATTACHED); }
void FragmentManager::dispatchCreate(){ dispatchStateChange(Fragment::CREATED); }
void FragmentManager::dispatchViewCreated(){ dispatchStateChange(Fragment::VIEW_CREATED); }
void FragmentManager::dispatchActivityCreated(){ dispatchStateChange(Fragment::ACTIVITY_CREATED); }
void FragmentManager::dispatchStart(){ mStopped = false; dispatchStateChange(Fragment::STARTED); }
void FragmentManager::dispatchResume(){ dispatchStateChange(Fragment::RESUMED); }
void FragmentManager::dispatchPause(){ dispatchStateChange(Fragment::STARTED); }
void FragmentManager::dispatchStop(){ mStopped = true; dispatchStateChange(Fragment::ACTIVITY_CREATED); }
void FragmentManager::dispatchDestroyView(){ dispatchStateChange(Fragment::CREATED); }
void FragmentManager::dispatchDestroy(){
    mDestroyed = true;
    dispatchStateChange(Fragment::INITIALIZING);
}

// --- internal fragment ops ---
void FragmentManager::addFragment(Fragment* f, bool hidden){
    if(!f) return;
    if(mActive.count(f->mWho)) return;
    f->mFragmentManager = this;
    f->mHost = mHost;
    f->mAdded = true;
    f->mHidden = hidden;
    // Resolve the container ViewGroup fragments inflate into / are added to.
    f->mContainer = mContainer ? dynamic_cast<cdroid::ViewGroup*>(mContainer->onFindViewById(f->mContainerId)) : nullptr;
    mAdded.push_back(f);
    mActive[f->mWho] = f;
    moveToState(f, mCurState);
}

void FragmentManager::removeFragment(Fragment* f){
    if(!f) return;
    f->mRemoving = true;
    f->mAdded = false;
    moveToState(f, Fragment::INITIALIZING);
    mAdded.erase(std::remove(mAdded.begin(), mAdded.end(), f), mAdded.end());
    mActive.erase(f->mWho);
    f->mFragmentManager = nullptr;
    f->mHost = nullptr;
}

void FragmentManager::showFragment(Fragment* f){
    if(!f) return;
    f->mHidden = false;
    f->mHiddenChanged = true;
    if(f->mView) f->mView->setVisibility(cdroid::View::VISIBLE);
}

void FragmentManager::hideFragment(Fragment* f){
    if(!f) return;
    f->mHidden = true;
    f->mHiddenChanged = true;
    if(f->mView) f->mView->setVisibility(cdroid::View::GONE);
}

void FragmentManager::attachFragment(Fragment* f){
    if(!f) return;
    f->mDetached = false;
}

void FragmentManager::detachFragment(Fragment* f){
    if(!f) return;
    f->mDetached = true;
    if(f->mView && f->mContainer) f->mContainer->removeView(f->mView);
}

// --- simplified per-fragment state machine (no special-effects intermediate states) ---
void FragmentManager::moveToState(Fragment* f, int newState){
    if(!f) return;
    // step up
    while(f->mState < newState){
        switch(f->mState){
            case Fragment::INITIALIZING:
                f->performAttach(); f->mState = Fragment::ATTACHED; break;
            case Fragment::ATTACHED:
                f->performCreate(nullptr); f->mState = Fragment::CREATED; break;
            case Fragment::CREATED: {
                cdroid::LayoutInflater* inflater = mHost ? mHost->onGetLayoutInflater() : nullptr;
                f->performCreateView(inflater, f->mContainer, nullptr);
                if(f->mView && f->mContainer) f->mContainer->addView(f->mView);
                // Enter transition: fade the view in.
                if(f->mView){
                    Animation* enter = FragmentAnim::loadEnterAnimation(f);
                    enter->setDuration(300);
                    f->mView->startAnimation(enter);
                }
                f->performViewCreated(nullptr);
                f->mState = Fragment::VIEW_CREATED;
                break;
            }
            case Fragment::VIEW_CREATED:
                f->performActivityCreated(nullptr); f->mState = Fragment::ACTIVITY_CREATED; break;
            case Fragment::ACTIVITY_CREATED:
                f->performStart(); f->mState = Fragment::STARTED; break;
            case Fragment::AWAITING_EXIT_EFFECTS:
            case Fragment::STARTED:
                f->performResume(); f->mState = Fragment::RESUMED; break;
            case Fragment::AWAITING_ENTER_EFFECTS:
                f->mState = Fragment::RESUMED; break;
            default: break;
        }
    }
    // step down
    while(f->mState > newState){
        switch(f->mState){
            case Fragment::RESUMED:
            case Fragment::AWAITING_ENTER_EFFECTS:
                f->performPause(); f->mState = Fragment::STARTED; break;
            case Fragment::STARTED:
            case Fragment::AWAITING_EXIT_EFFECTS:
                f->performStop(); f->mState = Fragment::ACTIVITY_CREATED; break;
            case Fragment::ACTIVITY_CREATED:
                f->mState = Fragment::VIEW_CREATED; break; // no callback on the way down here
            case Fragment::VIEW_CREATED:
                if(f->mView && f->mContainer){
                    // Exit transition: fade out, then remove the view on animation end.
                    Animation* exit = FragmentAnim::loadExitAnimation(f);
                    exit->setDuration(200);
                    cdroid::View* view = f->mView;
                    cdroid::ViewGroup* container = f->mContainer;
                    Animation::AnimationListener listener;
                    listener.onAnimationEnd = [view, container](Animation&){ container->removeView(view); };
                    exit->setAnimationListener(listener);
                    f->mView->startAnimation(exit);
                }
                f->performDestroyView();
                f->mView = nullptr;
                f->mState = Fragment::CREATED;
                break;
            case Fragment::CREATED:
                f->performDestroy(); f->mState = Fragment::ATTACHED; break;
            case Fragment::ATTACHED:
                f->performDetach(); f->mState = Fragment::INITIALIZING; break;
            default: break;
        }
    }
}

// --- lookups ---
Fragment* FragmentManager::findFragmentById(int id){
    for(Fragment* f : mAdded){ if(f && f->mFragmentId == id) return f; }
    return nullptr;
}

Fragment* FragmentManager::findFragmentByTag(const std::string& tag){
    for(Fragment* f : mAdded){ if(f && f->mTag == tag) return f; }
    return nullptr;
}

std::vector<Fragment*> FragmentManager::getFragments() const{ return mAdded; }

Fragment* FragmentManager::getPrimaryNavigationFragment() const{
    // MVP: not tracked yet.
    return nullptr;
}

void FragmentManager::setFragmentFactory(FragmentFactory* factory){ mFragmentFactory = factory; }
FragmentFactory* FragmentManager::getFragmentFactory() const{ return mFragmentFactory; }

// --- transactions (BackStackRecord-backed; stage 2b-4) ---
FragmentTransaction* FragmentManager::beginTransaction(){
    return new BackStackRecord(this);
}

bool FragmentManager::executePendingTransactions(){
    return false; // stage 2b-4
}

bool FragmentManager::popBackStackImmediate(){
    if(mBackStack.empty()) return false;
    BackStackRecord* record = mBackStack.back();
    mBackStack.pop_back();
    record->executePopOps();
    delete record;
    return true;
}

bool FragmentManager::popBackStackImmediate(const std::string& /*name*/, int /*flags*/){
    return false; // stage 2b-4
}

void FragmentManager::enqueueAction(BackStackRecord* action){
    if(action) mBackStack.push_back(action); // stage 2b-4 wires execution
}

}//namespace fragment
}//namespace cdroid
