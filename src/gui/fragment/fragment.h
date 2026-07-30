#ifndef __FRAGMENT_H__
#define __FRAGMENT_H__
/*********************************************************************************
 * Port of androidx.fragment.app.Fragment (skeleton). Fragment is a LifecycleOwner
 * (via SavedStateRegistryOwner), ViewModelStoreOwner, HasDefaultViewModelProviderFactory.
 *
 * C++ notes: mWho is public (same-package access in androidx); reflection-based
 * instantiate is replaced by FragmentFactory; mChildFragmentManager uses unique_ptr
 * (definition of ~Fragment() lives in fragment.cc to break the Fragment<->FragmentManager
 * cycle). Menu/permission/Loader/transition branches are stubbed for MVP.
 *********************************************************************************/
#include <string>
#include <memory>
#include <lifecycle/lifecycle.h>
#include <lifecycle/lifecycleowner.h>
#include <lifecycle/viewmodelstoreowner.h>
#include <lifecycle/hasdefaultviewmodelproviderfactory.h>
#include <lifecycle/viewmodelprovider.h>
#include <lifecycle/viewmodelstore.h>
#include <lifecycle/lifecycleregistry.h>
#include <savedstate/savedstateregistryowner.h>
#include <savedstate/savedstateregistrycontroller.h>

namespace cdroid{
class Context;
class Bundle;
class View;
class ViewGroup;
class LayoutInflater;
class AttributeSet;
class Handler;

namespace fragment{
class FragmentManager;
class FragmentHostCallback;
class FragmentViewLifecycleOwner;

class Fragment : public savedstate::SavedStateRegistryOwner,
                 public lifecycle::ViewModelStoreOwner,
                 public lifecycle::HasDefaultViewModelProviderFactory{
public:
    // Fragment state constants (verbatim from androidx).
    static const int INITIALIZING        = -1;
    static const int ATTACHED            = 0;
    static const int CREATED             = 1;
    static const int VIEW_CREATED        = 2;
    static const int AWAITING_EXIT_EFFECTS = 3;
    static const int ACTIVITY_CREATED    = 4;
    static const int STARTED             = 5;
    static const int AWAITING_ENTER_EFFECTS = 6;
    static const int RESUMED             = 7;

    Fragment();
    explicit Fragment(int contentLayoutId);
    ~Fragment() override;

    // --- identity / state (public for same-package access, as in androidx) ---
    int mState = INITIALIZING;
    std::string mWho;                 // unique id (assigned in ctor)
    cdroid::Bundle* mArguments = nullptr;
    std::string mTag;
    std::string mTargetWho;
    std::string mPreviousWho;
    int mFragmentId = 0;
    int mContainerId = 0;
    int mTargetRequestCode = 0;
    int mBackStackNesting = 0;
    bool mAdded = false;
    bool mRemoving = false;
    bool mFromLayout = false;
    bool mInLayout = false;
    bool mInDynamicContainer = false;
    bool mRestored = false;
    bool mHidden = false;
    bool mDetached = false;
    bool mHasMenu = false;
    bool mMenuVisible = true;
    bool mHiddenChanged = false;
    bool mRetainInstance = false;
    bool mIsCreated = false;
    bool mUserVisibleHint = true;
    lifecycle::Lifecycle::State mMaxState = lifecycle::Lifecycle::State::RESUMED;

    // links (set by FragmentManager)
    FragmentManager* mFragmentManager = nullptr;
    FragmentHostCallback* mHost = nullptr;
    Fragment* mParentFragment = nullptr;
    cdroid::ViewGroup* mContainer = nullptr;
    cdroid::View* mView = nullptr;

    // --- lifecycle callbacks (override in subclasses) ---
    virtual void onInflate(cdroid::Context* ctx, cdroid::AttributeSet* attrs, cdroid::Bundle* savedInstanceState){}
    virtual void onAttach(cdroid::Context* context){}
    virtual void onCreate(cdroid::Bundle* savedInstanceState){}
    virtual cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                                       cdroid::Bundle* savedInstanceState){ return nullptr; }
    virtual void onViewCreated(cdroid::View* view, cdroid::Bundle* savedInstanceState){}
    virtual void onActivityCreated(cdroid::Bundle* savedInstanceState){}
    virtual void onStart(){}
    virtual void onResume(){}
    virtual void onSaveInstanceState(cdroid::Bundle* outState){}
    virtual void onPause(){}
    virtual void onStop(){}
    virtual void onDestroyView(){}
    virtual void onDestroy(){}
    virtual void onDetach(){}
    virtual void onHiddenChanged(bool hidden){}
    virtual void onLowMemory(){}
    virtual void onConfigurationChanged(){}

    // --- owner interface implementations ---
    lifecycle::Lifecycle& getLifecycle() override; // returns mLifecycleRegistry
    lifecycle::ViewModelStore& getViewModelStore() override;
    savedstate::SavedStateRegistry& getSavedStateRegistry() override;
    lifecycle::ViewModelProvider::Factory& getDefaultViewModelProviderFactory() override;
    // View-scoped owner (null before onCreateView / after onDestroyView).
    FragmentViewLifecycleOwner* getViewLifecycleOwner() const { return mViewLifecycleOwner; }

    // --- accessors ---
    cdroid::Context* getContext();
    cdroid::Context* requireContext();
    cdroid::View* requireView();
    cdroid::View* getView() const { return mView; }
    cdroid::Bundle* getArguments() const { return mArguments; }
    void setArguments(cdroid::Bundle* args);
    FragmentManager* getParentFragmentManager();
    FragmentManager* getChildFragmentManager();
    Fragment* getParentFragment() const { return mParentFragment; }
    bool isAdded() const { return mHost != nullptr; }
    bool isDetached() const { return mDetached; }
    bool isRemoving() const { return mRemoving; }
    bool isResumed() const { return mState == RESUMED; }
    bool isVisible() const;
    bool isInLayout() const { return mInLayout; }
    int getId() const { return mFragmentId; }
    const std::string& getTag() const { return mTag; }

    // --- perform* (driven by FragmentStateManager; defined in fragment.cc) ---
    void performAttach();
    void performCreate(cdroid::Bundle* savedInstanceState);
    void performCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                           cdroid::Bundle* savedInstanceState);
    void performViewCreated(cdroid::Bundle* savedInstanceState);
    void performActivityCreated(cdroid::Bundle* savedInstanceState);
    void performStart();
    void performResume();
    void performPause();
    void performStop();
    void performDestroyView();
    void performDestroy();
    void performDetach();

protected:
    lifecycle::LifecycleRegistry* mLifecycleRegistry = nullptr; // owned
    lifecycle::ViewModelStore* mViewModelStore = nullptr;        // owned (MVP: per-Fragment)
    savedstate::SavedStateRegistryController* mSavedStateRegistryController = nullptr; // owned
    FragmentViewLifecycleOwner* mViewLifecycleOwner = nullptr;  // owned
    lifecycle::ViewModelProvider::Factory* mDefaultFactory = nullptr; // owned
    std::unique_ptr<FragmentManager> mChildFragmentManager;      // owned; breaks the FM cycle
    FragmentHostCallback* mChildHost = nullptr;                  // nested-fragment host (owned)      // owned; breaks the FM cycle
    int mContentLayoutId = 0;

private:
    // Generates a unique mWho.
    static std::string generateWho();
};

}//namespace fragment
}//namespace cdroid
#endif
