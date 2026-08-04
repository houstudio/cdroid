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
#ifndef __FRAGMENTMANAGER_H__
#define __FRAGMENTMANAGER_H__
/*********************************************************************************
 * Port of androidx.fragment.app.FragmentManager (MVP). Drives Fragment lifecycle
 * via dispatchXxx -> dispatchStateChange -> moveToState -> ensureFragmentState.
 * FragmentStore/FragmentStateManager are inlined into a simplified per-fragment
 * state loop for MVP (no special-effects/animation intermediate states); they can
 * be split out later. BackStack/Transaction execution is wired in the next batch.
 *********************************************************************************/
#include <string>
#include <vector>
#include <unordered_map>
#include <lifecycle/lifecycle.h>
#include <core/handler.h>
#include <core/callbackbase.h>
namespace cdroid{
class LayoutInflater;
class View;
namespace fragment{

class Fragment;
class FragmentHostCallback;
class FragmentContainer;
class FragmentFactory;
class FragmentTransaction;
class BackStackRecord;
class FragmentStateManager;

class FragmentManager{
public:
    static const int POP_BACK_STACK_INCLUSIVE = 1;

    FragmentManager();
    virtual ~FragmentManager();

    // --- host attachment + lifecycle dispatch ---
    void attachController(FragmentHostCallback* host, FragmentContainer* container, Fragment* parent);
    void dispatchStateChange(int state); // sets mCurState + drives all added fragments
    void dispatchAttach();
    void dispatchCreate();
    void dispatchViewCreated();
    void dispatchActivityCreated();
    void dispatchStart();
    void dispatchResume();
    void dispatchPause();
    void dispatchStop();
    void dispatchDestroyView();
    void dispatchDestroy();
    int getCurState() const { return mCurState; }

    // --- transactions (BackStackRecord-backed) ---
    // A deferred commit, mirroring androidx FragmentManager: commit() enqueues a record and
    // scheduleCommit() posts mExecCommit to the host Handler; the real executeOps()+state
    // sweep runs on the next main-loop iteration (execPendingActions).
    class OpGenerator{
    public:
        virtual ~OpGenerator() = default;
        // Append this generator's BackStackRecord(s) to records and whether each is a pop.
        virtual bool generateOps(std::vector<BackStackRecord*>& records,
                                 std::vector<bool>& isRecordPop) = 0;
    };

    FragmentTransaction* beginTransaction();
    bool executePendingTransactions();
    bool popBackStackImmediate();
    bool popBackStackImmediate(const std::string& name, int flags);
    // Enqueue a record for deferred execution (androidx enqueueAction). Transfers ownership of
    // `action` to this FragmentManager (freed after execution, or held in mBackStack if added
    // to the back stack).
    void enqueueAction(BackStackRecord* action, bool allowStateLoss);
    // Run all pending commits synchronously now (androidx execPendingActions). Returns whether
    // anything ran. Also used as the safety valve at call sites that need a commit's effect
    // immediately (executePendingTransactions / commitNow drains).
    bool execPendingActions(bool allowStateLoss);
    // Execute a single record synchronously (androidx execSingleAction, the commitNow path):
    // drains pending first, then runs this record's ops + state sweep without enqueueing.
    void execSingleAction(BackStackRecord* action, bool allowStateLoss);
    void addBackStackState(BackStackRecord* state){ mBackStack.push_back(state); }

    // --- lookups ---
    Fragment* findFragmentById(int id);
    Fragment* findFragmentByTag(const std::string& tag);
    std::vector<Fragment*> getFragments() const;
    Fragment* getPrimaryNavigationFragment() const;

    void setFragmentFactory(FragmentFactory* factory);
    FragmentFactory* getFragmentFactory() const;

    // --- internal fragment ops (used by BackStackRecord.executeOps) ---
    void addFragment(Fragment* fragment, bool hidden);
    void removeFragment(Fragment* fragment);
    // Retain/restore a fragment across a reversible (back-stack) transaction — see .cc.
    void retainFragment(Fragment* fragment);
    void unretainFragment(Fragment* fragment);
    void showFragment(Fragment* fragment);
    void hideFragment(Fragment* fragment);
    void attachFragment(Fragment* fragment);
    void detachFragment(Fragment* fragment);
    // Drives a single fragment to newState (simplified state machine for MVP).
    void moveToState(Fragment* f, int newState);
    // Cap a fragment's lifecycle at `state` (androidx FragmentManager.setMaxLifecycle): sets
    // mMaxState and re-drives so moveToState's clamp takes effect.
    void setMaxLifecycle(Fragment* f, lifecycle::Lifecycle::State state);

    FragmentHostCallback* getHost() const { return mHost; }
    Fragment* getParent() const { return mParent; }

    // Shared-element transition: the active BackStackRecord declares shared element names;
    // FragmentManager resolves them to target views (by transitionName) in the entering fragment.
    void setPendingSharedElementNames(const std::vector<std::string>& names){ mPendingSharedNames = names; }

private:
    friend class FragmentStateManager; // FSM drives the per-fragment state machine
    friend class BackStackRecord;      // records call enqueueAction/execSingleAction/generateOps
    std::unordered_map<Fragment*, FragmentStateManager*> mStateManagers;
    FragmentStateManager* getOrCreateStateManager(Fragment* f);
    FragmentHostCallback* mHost = nullptr;
    FragmentContainer* mContainer = nullptr;
    Fragment* mParent = nullptr;
    int mCurState = -1; // Fragment::INITIALIZING
    FragmentFactory* mFragmentFactory = nullptr;
    std::vector<Fragment*> mAdded;
    std::unordered_map<std::string, Fragment*> mActive; // who -> Fragment
    std::vector<BackStackRecord*> mBackStack;
    std::vector<std::string> mPendingSharedNames;
    bool mDestroyed = false;
    bool mStateSaved = false;
    bool mStopped = true;
    // --- deferred-commit machinery (androidx FragmentManager) ---
    std::vector<BackStackRecord*> mPendingActions; // records enqueued by commit(), awaiting exec
    bool mExecutingActions = false;                // re-entrancy guard for execPendingActions
    cdroid::Runnable mExecCommit;                  // posted to host Handler -> execPendingActions(true)
    std::vector<BackStackRecord*> mTmpRecords;     // scratch buffers for batched execution
    std::vector<bool> mTmpIsPop;
    void scheduleCommit();
    bool generateOpsForPendingActions(std::vector<BackStackRecord*>& records,
                                      std::vector<bool>& isRecordPop);
    void removeRedundantOperationsAndExecute(std::vector<BackStackRecord*>& records,
                                             std::vector<bool>& isRecordPop);
    void executeOpsTogether(std::vector<BackStackRecord*>& records,
                            std::vector<bool>& isRecordPop, int startIndex, int endIndex);
    void ensureExecReady(bool allowStateLoss);
    void cleanupExec();
    static cdroid::View* findViewByTransitionName(cdroid::View* root, const std::string& name);
};

}//namespace fragment
}//namespace cdroid
#endif
