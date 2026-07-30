#include <fragment/fragment.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenthostcallback.h>
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
    cdroid::Handler* getHandler() override { return nullptr; }
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
    delete mLifecycleRegistry;
    delete mSavedStateRegistryController;
    delete mViewModelStore;
    delete mDefaultFactory;
    delete mChildHost;
    // mViewLifecycleOwner created in 2b-5; nullptr until then.
    // mChildFragmentManager (unique_ptr) releases automatically.
}

// --- owner interface ---
lifecycle::Lifecycle& Fragment::getLifecycle(){ return *mLifecycleRegistry; }
lifecycle::ViewModelStore& Fragment::getViewModelStore(){ return *mViewModelStore; }
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
FragmentManager* Fragment::getChildFragmentManager(){ return mChildFragmentManager.get(); }

bool Fragment::isVisible() const{
    return mHost != nullptr && !mHidden && mView != nullptr && mUserVisibleHint;
}

// --- perform* (lifecycle dispatch + nested childFragmentManager) ---
void Fragment::performAttach(){
    onAttach(getContext());
}
void Fragment::performCreate(cdroid::Bundle* savedInstanceState){
    onCreate(savedInstanceState);
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_CREATE);
    mIsCreated = true;
    if(mChildFragmentManager){
        if(!mChildHost){
            mChildHost = new FragmentChildHost(this);
            mChildFragmentManager->attachController(mChildHost, mChildHost, this);
        }
        mChildFragmentManager->dispatchCreate();
    }
}
void Fragment::performCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                                 cdroid::Bundle* savedInstanceState){
    mView = onCreateView(inflater, container, savedInstanceState);
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
    if(mChildFragmentManager) mChildFragmentManager->dispatchStart();
}
void Fragment::performResume(){
    onResume();
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_RESUME);
    if(mChildFragmentManager) mChildFragmentManager->dispatchResume();
}
void Fragment::performPause(){
    if(mChildFragmentManager) mChildFragmentManager->dispatchPause();
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_PAUSE);
    onPause();
}
void Fragment::performStop(){
    if(mChildFragmentManager) mChildFragmentManager->dispatchStop();
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_STOP);
    onStop();
}
void Fragment::performDestroyView(){
    if(mChildFragmentManager) mChildFragmentManager->dispatchDestroyView();
    onDestroyView();
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

}//namespace fragment
}//namespace cdroid
