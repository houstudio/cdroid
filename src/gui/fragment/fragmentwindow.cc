#include <fragment/fragmentwindow.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenthostcallback.h>
#include <view/layoutinflater.h>
#include <view/viewgroup.h>
#include <core/context.h>
#include <core/handler.h>

namespace cdroid{
namespace fragment{

// FragmentHostCallback implementation bound to the FragmentWindow.
class FragmentWindow::HostCallbacks : public FragmentHostCallback{
    FragmentWindow* mWindow;
public:
    explicit HostCallbacks(FragmentWindow* w) : mWindow(w){}
    cdroid::Context* getContext() override { return mWindow->getContext(); }
    cdroid::Handler* getHandler() override {
        // MVP: FragmentManager dispatch is synchronous (no Handler.post scheduling),
        // so no real Handler is needed yet. Wired up with OnBackPressedDispatcher later.
        return nullptr;
    }
    cdroid::LayoutInflater* onGetLayoutInflater() override {
        return cdroid::LayoutInflater::from(getContext());
    }
    cdroid::Window* onGetHost() override { return mWindow; }
    cdroid::View* onFindViewById(int id) override {
        if(id == mWindow->getFragmentContainerId()) return mWindow;
        return mWindow->findViewById(id);
    }
    bool onHasView() override { return true; }
};

FragmentWindow::FragmentWindow(int x, int y, int w, int h)
    : Window(x, y, w, h){
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

FragmentWindow::~FragmentWindow(){
    mFragmentManager->dispatchDestroyView();
    mFragmentManager->dispatchDestroy();
    delete mFragmentManager;
    delete mHost;
    delete mSavedStateRegistryController;
    delete mViewModelStore;
    delete mLifecycleRegistry;
}

lifecycle::Lifecycle& FragmentWindow::getLifecycle(){ return *mLifecycleRegistry; }
lifecycle::ViewModelStore& FragmentWindow::getViewModelStore(){ return *mViewModelStore; }
savedstate::SavedStateRegistry& FragmentWindow::getSavedStateRegistry(){
    return mSavedStateRegistryController->getSavedStateRegistry();
}

void FragmentWindow::onCreate(){
    Window::onCreate();
    mFragmentManager->dispatchAttach();
    mFragmentManager->dispatchCreate();
    mFragmentManager->dispatchViewCreated();
    mFragmentManager->dispatchActivityCreated();
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_CREATE);
}

void FragmentWindow::onActive(){
    Window::onActive();
    mFragmentManager->dispatchStart();
    mFragmentManager->dispatchResume();
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_RESUME);
}

void FragmentWindow::onDeactive(){
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_PAUSE);
    mFragmentManager->dispatchPause();
    mFragmentManager->dispatchStop();
    mLifecycleRegistry->handleLifecycleEvent(lifecycle::Lifecycle::Event::ON_STOP);
    Window::onDeactive();
}

void FragmentWindow::onBackPressed(){
    if(mFragmentManager->popBackStackImmediate()) return;
    Window::onBackPressed();
}

FragmentManager* FragmentWindow::getSupportFragmentManager(){ return mFragmentManager; }

}//namespace fragment
}//namespace cdroid
