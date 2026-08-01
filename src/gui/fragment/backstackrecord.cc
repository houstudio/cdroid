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
#include <fragment/backstackrecord.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragment.h>
#include <algorithm>

namespace cdroid{
namespace fragment{

BackStackRecord::BackStackRecord(FragmentManager* manager) : mManager(manager){
}

// Deferred: hand ownership of this record to the FragmentManager's pending queue; execution
// (executeOps + state sweep) happens on the next main-loop iteration via execPendingActions, or
// immediately if a caller drains with executePendingTransactions.
int BackStackRecord::commit(){
    if(mManager) mManager->enqueueAction(this, false);
    return mIndex;
}

int BackStackRecord::commitAllowingStateLoss(){
    if(mManager) mManager->enqueueAction(this, true);
    return mIndex;
}

// Synchronous: drain pending first, then execute this record inline without enqueueing
// (androidx FragmentManager.commitNow -> execPendingActions + execSingleAction).
void BackStackRecord::commitNow(){
    if(mManager) mManager->execSingleAction(this, false);
}

void BackStackRecord::commitNowAllowingStateLoss(){
    if(mManager) mManager->execSingleAction(this, true);
}

// OpGenerator: this record contributes itself as a single forward op batch.
bool BackStackRecord::generateOps(std::vector<BackStackRecord*>& records,
                                  std::vector<bool>& isRecordPop){
    records.push_back(this);
    isRecordPop.push_back(false);
    return true;
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
        // Push this op's custom animations onto the fragment before executing it (androidx
        // executeOps: f.setAnimations(op.mEnterAnim,...)). Structural ops only (ADD..ATTACH);
        // setMaxLifecycle/setPrimaryNav carry no animations.
        if(op.mCmd >= OP_ADD && op.mCmd <= OP_ATTACH){
            f->setAnimations(op.mEnterAnim, op.mExitAnim, op.mPopEnterAnim, op.mPopExitAnim);
        }
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
                        // Push the op's animations onto the displaced fragment too so its exit
                        // uses the custom anim; otherwise it falls back to the default Fade
                        // transition, whose beginDelayedTransition would also fade the entering
                        // fragment and mask its custom enter animation.
                        e->setAnimations(op.mEnterAnim, op.mExitAnim, op.mPopEnterAnim, op.mPopExitAnim);
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
        // Pop direction: SEC FragmentAnim.getNextAnim(isPop=true) reads mPopEnterAnim/mPopExitAnim
        // directly (set by executeOps above), so no anim swap needed here anymore.
        switch(it->mCmd){
            case OP_ADD:    mManager->removeFragment(f); break;
            case OP_REMOVE: mManager->addFragment(f, false); break;
            case OP_REPLACE: {
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

BackStackRecordState BackStackRecord::captureState() const{
    // androidx BackStackRecordState(BackStackRecord): snapshot each op (cmd + fragment mWho + anims
    // + lifecycle) and the record name. The fragment is captured by mWho (not pointer) so the record
    // can be rebuilt after the fragment is destroyed and re-instantiated.
    BackStackRecordState s;
    s.name = mName;
    for(const Op& op : mOps){
        BackStackRecordState::OpState os;
        os.cmd = op.mCmd;
        os.fragmentWho = (op.mFragment && !op.mFragment->mWho.empty()) ? op.mFragment->mWho : "";
        os.enterAnim = op.mEnterAnim;
        os.exitAnim = op.mExitAnim;
        os.popEnterAnim = op.mPopEnterAnim;
        os.popExitAnim = op.mPopExitAnim;
        os.currentMaxState = op.mCurrentMaxState;
        os.oldMaxState = op.mOldMaxState;
        s.ops.push_back(os);
    }
    return s;
}

void BackStackRecord::restoreFromState(const BackStackRecordState& state,
                                       const std::unordered_map<std::string, Fragment*>& fragments){
    // androidx BackStackRecordState.fillInBackStackRecord: rebuild the ops from the snapshot,
    // resolving each fragment by mWho against the re-created fragments.
    mOps.clear();
    mName = state.name;
    for(const BackStackRecordState::OpState& os : state.ops){
        Op op;
        op.mCmd = os.cmd;
        if(!os.fragmentWho.empty()){
            auto it = fragments.find(os.fragmentWho);
            op.mFragment = (it != fragments.end()) ? it->second : nullptr;
        }
        op.mEnterAnim = os.enterAnim;
        op.mExitAnim = os.exitAnim;
        op.mPopEnterAnim = os.popEnterAnim;
        op.mPopExitAnim = os.popExitAnim;
        op.mCurrentMaxState = os.currentMaxState;
        op.mOldMaxState = os.oldMaxState;
        addOp(op);
    }
    mAddToBackStack = true;
}

}//namespace fragment
}//namespace cdroid
