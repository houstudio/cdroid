#include <lifecycle/viewmodel.h>

namespace cdroid{
namespace lifecycle{

ViewModel::~ViewModel(){
    // Deletion timing is owned by ViewModelStore; resources are closed via clear().
}

void ViewModel::addCloseable(const std::string& key, Closeable* closeable){
    if(mCleared){
        if(closeable) closeable->close();
        return;
    }
    Closeable* previous = nullptr;
    auto it = mKeyedCloseables.find(key);
    if(it != mKeyedCloseables.end()) previous = it->second;
    mKeyedCloseables[key] = closeable;
    if(previous) previous->close();
}

void ViewModel::addCloseable(Closeable* closeable){
    if(mCleared){
        if(closeable) closeable->close();
        return;
    }
    mCloseables.push_back(closeable);
}

Closeable* ViewModel::getCloseable(const std::string& key){
    auto it = mKeyedCloseables.find(key);
    return it == mKeyedCloseables.end() ? nullptr : it->second;
}

void ViewModel::clear(){
    if(mCleared) return;
    mCleared = true;
    for(auto& kv : mKeyedCloseables){ if(kv.second) kv.second->close(); }
    mKeyedCloseables.clear();
    for(auto* c : mCloseables){ if(c) c->close(); }
    mCloseables.clear();
    onCleared();
}

}//namespace lifecycle
}//namespace cdroid
