#include <fragment/fragmentmanagerviewmodel.h>
#include <fragment/fragment.h>
#include <lifecycle/viewmodelstore.h>
#include <lifecycle/viewmodelprovider.h>
#include <lifecycle/creationextras.h>

namespace cdroid{
namespace fragment{

namespace{
class FMVMFactory : public lifecycle::ViewModelProvider::Factory{
public:
    lifecycle::ViewModel* create(const std::string& /*modelClass*/, lifecycle::CreationExtras&) override{
        return new FragmentManagerViewModel(true);
    }
};
}//anonymous

FragmentManagerViewModel::FragmentManagerViewModel(bool stateAutomaticallySaved)
    : mStateAutomaticallySaved(stateAutomaticallySaved){}

FragmentManagerViewModel::~FragmentManagerViewModel(){
    for(auto& kv : mChildNonConfigs){ kv.second->onCleared(); delete kv.second; }
    for(auto& kv : mViewModelStores){ kv.second->clear(); delete kv.second; }
    mChildNonConfigs.clear();
    mViewModelStores.clear();
}

FragmentManagerViewModel* FragmentManagerViewModel::getInstance(lifecycle::ViewModelStore* store){
    static FMVMFactory sFactory;
    lifecycle::ViewModelProvider provider(store, &sFactory, nullptr);
    return provider.get<FragmentManagerViewModel>("androidx.fragment.app.FragmentManagerViewModel");
}

void FragmentManagerViewModel::onCleared(){
    mHasBeenCleared = true;
}

void FragmentManagerViewModel::addRetainedFragment(Fragment* fragment){
    if(mIsStateSaved || !fragment) return;
    if(mRetainedFragments.count(fragment->mWho)) return;
    mRetainedFragments[fragment->mWho] = fragment;
}

Fragment* FragmentManagerViewModel::findRetainedFragmentByWho(const std::string& who) const{
    auto it = mRetainedFragments.find(who);
    return it == mRetainedFragments.end() ? nullptr : it->second;
}

std::vector<Fragment*> FragmentManagerViewModel::getRetainedFragments() const{
    std::vector<Fragment*> out;
    out.reserve(mRetainedFragments.size());
    for(const auto& kv : mRetainedFragments) out.push_back(kv.second);
    return out;
}

bool FragmentManagerViewModel::shouldDestroy(Fragment* fragment) const{
    if(!fragment || !mRetainedFragments.count(fragment->mWho)) return true;
    return mStateAutomaticallySaved ? mHasBeenCleared : !mHasSavedSnapshot;
}

void FragmentManagerViewModel::removeRetainedFragment(Fragment* fragment){
    if(mIsStateSaved || !fragment) return;
    mRetainedFragments.erase(fragment->mWho);
}

FragmentManagerViewModel* FragmentManagerViewModel::getChildNonConfig(Fragment* f){
    if(!f) return nullptr;
    FragmentManagerViewModel*& child = mChildNonConfigs[f->mWho];
    if(!child) child = new FragmentManagerViewModel(mStateAutomaticallySaved);
    return child;
}

lifecycle::ViewModelStore* FragmentManagerViewModel::getViewModelStore(Fragment* f){
    if(!f) return nullptr;
    lifecycle::ViewModelStore*& store = mViewModelStores[f->mWho];
    if(!store) store = new lifecycle::ViewModelStore();
    return store;
}

void FragmentManagerViewModel::clearNonConfigStateInternal(const std::string& who, bool destroyChildNonConfig){
    auto childIt = mChildNonConfigs.find(who);
    if(childIt != mChildNonConfigs.end()){
        if(destroyChildNonConfig){
            std::vector<std::string> grandchildren;
            for(const auto& kv : childIt->second->mChildNonConfigs) grandchildren.push_back(kv.first);
            for(const auto& gw : grandchildren) childIt->second->clearNonConfigState(gw, true);
        }
        childIt->second->onCleared();
        delete childIt->second;
        mChildNonConfigs.erase(childIt);
    }
    auto storeIt = mViewModelStores.find(who);
    if(storeIt != mViewModelStores.end()){
        storeIt->second->clear();
        delete storeIt->second;
        mViewModelStores.erase(storeIt);
    }
}

void FragmentManagerViewModel::clearNonConfigState(Fragment* f, bool destroyChildNonConfig){
    if(f) clearNonConfigStateInternal(f->mWho, destroyChildNonConfig);
}

void FragmentManagerViewModel::clearNonConfigState(const std::string& who, bool destroyChildNonConfig){
    clearNonConfigStateInternal(who, destroyChildNonConfig);
}

}//namespace fragment
}//namespace cdroid
