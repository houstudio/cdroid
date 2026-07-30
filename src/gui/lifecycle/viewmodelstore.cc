#include <lifecycle/viewmodelstore.h>
#include <vector>

namespace cdroid{
namespace lifecycle{

ViewModelStore::~ViewModelStore(){
    clear();
}

void ViewModelStore::put(const std::string& key, ViewModel* viewModel){
    auto it = mMap.find(key);
    ViewModel* old = (it != mMap.end()) ? it->second : nullptr;
    mMap[key] = viewModel;
    if(old){ old->clear(); delete old; }
}

ViewModel* ViewModelStore::get(const std::string& key){
    auto it = mMap.find(key);
    return it == mMap.end() ? nullptr : it->second;
}

void ViewModelStore::clear(){
    std::vector<ViewModel*> snapshot;
    snapshot.reserve(mMap.size());
    for(auto& kv : mMap) snapshot.push_back(kv.second);
    mMap.clear();
    for(auto* vm : snapshot){ vm->clear(); delete vm; }
}

}//namespace lifecycle
}//namespace cdroid
