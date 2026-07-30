#ifndef __SAVEDSTATEHANDLE_H__
#define __SAVEDSTATEHANDLE_H__
/*********************************************************************************
 * Port of androidx.lifecycle.SavedStateHandle. A key-value store passed to a ViewModel
 * so its state persists across process death. Backed by the stage-1 SavedState (map<string,
 * any>). Flow/getLiveData are omitted (no coroutines/LiveData); the typed get/put + save/restore
 * surface is preserved.
 *********************************************************************************/
#include <string>
#include <savedstate/savedstate.h>
namespace cdroid{
namespace savedstate{

class SavedStateHandle{
public:
    SavedStateHandle() = default;
    explicit SavedStateHandle(SavedState* state){ if(state) mData.putAll(*state); }

    template<typename T> void put(const std::string& key, const T& value){ mData.put(key, value); }
    template<typename T> bool get(const std::string& key, T* out) const { return mData.get(key, out); }
    bool contains(const std::string& key) const { return mData.contains(key); }
    void remove(const std::string& key){ mData.remove(key); }
    const SavedState& getSavedState() const { return mData; }

    // androidx SavedStateHandle.createHandle(restoredState, defaultState).
    static SavedStateHandle* createHandle(SavedState* restoredState, SavedState* defaultState);

private:
    SavedState mData;
};

}//namespace savedstate
}//namespace cdroid
#endif
