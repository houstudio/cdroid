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
namespace cdroid{
class LayoutInflater;
namespace fragment{

class Fragment;
class FragmentHostCallback;
class FragmentContainer;
class FragmentFactory;
class FragmentTransaction;
class BackStackRecord;

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

    // --- transactions (BackStackRecord-backed; next batch) ---
    FragmentTransaction* beginTransaction();
    bool executePendingTransactions();
    bool popBackStackImmediate();
    bool popBackStackImmediate(const std::string& name, int flags);
    void enqueueAction(BackStackRecord* action);
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
    void showFragment(Fragment* fragment);
    void hideFragment(Fragment* fragment);
    void attachFragment(Fragment* fragment);
    void detachFragment(Fragment* fragment);
    // Drives a single fragment to newState (simplified state machine for MVP).
    void moveToState(Fragment* f, int newState);

    FragmentHostCallback* getHost() const { return mHost; }
    Fragment* getParent() const { return mParent; }

private:
    FragmentHostCallback* mHost = nullptr;
    FragmentContainer* mContainer = nullptr;
    Fragment* mParent = nullptr;
    int mCurState = -1; // Fragment::INITIALIZING
    FragmentFactory* mFragmentFactory = nullptr;
    std::vector<Fragment*> mAdded;
    std::unordered_map<std::string, Fragment*> mActive; // who -> Fragment
    std::vector<BackStackRecord*> mBackStack;
    bool mDestroyed = false;
    bool mStateSaved = false;
    bool mStopped = true;
};

}//namespace fragment
}//namespace cdroid
#endif
