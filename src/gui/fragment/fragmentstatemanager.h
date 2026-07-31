#ifndef __FRAGMENTSTATEMANAGER_H__
#define __FRAGMENTSTATEMANAGER_H__
/*********************************************************************************
 * Port of androidx.fragment.app.FragmentStateManager (simplified). Owns one
 * Fragment's state machine: computeExpectedState (clamps the manager state by
 * mMaxState/mAdded) + moveToExpectedState (re-entrancy guard via mMovingToState +
 * a while-recompute engine, exactly like androidx FragmentStateManager). Step
 * bodies (perform* + Fade transition) mirror the former FragmentManager::moveToState
 * loop. Replaces the inlined-in-FM MVP loop.
 *
 * Deferred (not ported yet): SpecialEffectsController, FragmentStore, mFromLayout,
 * mInDynamicContainer, mDeferStart, mTransitioning, saved-state plumbing.
 *********************************************************************************/
namespace cdroid{
namespace fragment{

class Fragment;
class FragmentManager;

class FragmentStateManager{
public:
    explicit FragmentStateManager(FragmentManager* fm, Fragment* f);
    // The owning FragmentManager feeds its current host state (mCurState) here so that
    // computeExpectedState can clamp against it.
    void setFragmentManagerState(int s){ mFragmentManagerState = s; }
    int  getFragmentManagerState() const { return mFragmentManagerState; }
    Fragment* getFragment() const { return mFragment; }

    // androidx FragmentStateManager.computeExpectedState (CDROID subset).
    int  computeExpectedState();
    // Re-entrancy-guarded driver: while(computeExpectedState() != mState) step. Use for the
    // normal lifecycle path (add/dispatch/setMaxLifecycle). setMaxLifecycle called from inside
    // a lifecycle callback (e.g. onViewCreated) hits the guard and returns; the outer loop's
    // next computeExpectedState() picks up the new mMaxState — no infinite recursion.
    void moveToExpectedState();
    // Drive to an explicit target state (used by remove/retain which want INITIALIZING/CREATED
    // regardless of computeExpectedState). Also re-entrancy-guarded.
    void moveToState(int explicitTarget);
private:
    FragmentManager* mFragmentManager;
    Fragment* mFragment;
    int mFragmentManagerState = -1; // Fragment::INITIALIZING; mirrors FragmentManager::mCurState
    bool mMovingToState = false;    // re-entrancy guard (androidx FragmentStateManager.mMovingToState)
    void stepUp();
    void stepDown();
};

}}//namespace fragment::cdroid
#endif/*__FRAGMENTSTATEMANAGER_H__*/
