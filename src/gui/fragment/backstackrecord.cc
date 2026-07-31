#include <fragment/backstackrecord.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragment.h>
#include <algorithm>

namespace cdroid{
namespace fragment{

BackStackRecord::BackStackRecord(FragmentManager* manager) : mManager(manager){
}

int BackStackRecord::commit(){
    return commitInternal();
}

int BackStackRecord::commitAllowingStateLoss(){
    return commitInternal();
}

void BackStackRecord::commitNow(){
    commitInternal();
}

void BackStackRecord::commitNowAllowingStateLoss(){
    commitInternal();
}

int BackStackRecord::commitInternal(){
    executeOps();
    if(mAddToBackStack && mManager){
        mManager->addBackStackState(this);
    }
    return mIndex;
}

void BackStackRecord::executeOps(){
    if(!mManager) return;
    // Declare shared element names so FragmentManager can resolve target views in the
    // entering fragment (by transitionName) when it builds the enter transition.
    if(!mSharedElements.empty()){
        std::vector<std::string> names;
        for(const SharedElement& se : mSharedElements) names.push_back(se.name);
        mManager->setPendingSharedElementNames(names);
    }
    for(Op& op : mOps){
        Fragment* f = op.mFragment;
        if(!f) continue;
        switch(op.mCmd){
            case OP_ADD:
                mManager->addFragment(f, false);
                break;
            case OP_REPLACE: {
                int cid = f->mContainerId;
                std::vector<Fragment*> existing = mManager->getFragments();
                Fragment* oldFrag = nullptr;
                for(Fragment* e : existing){
                    if(e && e != f && e->mContainerId == cid){
                        oldFrag = e;
                        // A reversible transaction must keep the displaced fragment alive
                        // (retained at CREATED) so popBackStack can restore it; otherwise
                        // removeFragment would fully destroy it and pop would throw.
                        if(mAddToBackStack) mManager->retainFragment(e);
                        else                 mManager->removeFragment(e);
                    }
                }
                op.mOldFragment = oldFrag; // remembered so pop can restore it
                mManager->addFragment(f, false);
                break;
            }
            case OP_REMOVE: mManager->removeFragment(f); break;
            case OP_HIDE:   mManager->hideFragment(f); break;
            case OP_SHOW:   mManager->showFragment(f); break;
            case OP_DETACH: mManager->detachFragment(f); break;
            case OP_ATTACH: mManager->attachFragment(f); break;
            case OP_SET_MAX_LIFECYCLE:
                op.mOldMaxState = f->mMaxState;              // save for pop restore (androidx)
                mManager->setMaxLifecycle(f, op.mCurrentMaxState);
                break;
            default: break;
        }
    }
}

void BackStackRecord::executePopOps(){
    if(!mManager) return;
    for(auto it = mOps.rbegin(); it != mOps.rend(); ++it){
        Fragment* f = it->mFragment;
        if(!f) continue;
        switch(it->mCmd){
            case OP_ADD:    mManager->removeFragment(f); break;
            case OP_REMOVE: mManager->addFragment(f, false); break;
            case OP_REPLACE: {
                // Reverse of executeOps REPLACE: destroy the new fragment, restore the old one.
                if(f) mManager->removeFragment(f);
                if(it->mOldFragment) mManager->unretainFragment(it->mOldFragment);
                break;
            }
            case OP_HIDE:   mManager->showFragment(f); break;
            case OP_SHOW:   mManager->hideFragment(f); break;
            case OP_DETACH: mManager->attachFragment(f); break;
            case OP_ATTACH: mManager->detachFragment(f); break;
            case OP_SET_MAX_LIFECYCLE:
                mManager->setMaxLifecycle(f, it->mOldMaxState); // restore prior ceiling on pop
                break;
            default: break;
        }
    }
}

}//namespace fragment
}//namespace cdroid
