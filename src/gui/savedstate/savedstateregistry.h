#ifndef __SAVEDSTATEREGISTRY_H__
#define __SAVEDSTATEREGISTRY_H__
/*********************************************************************************
 * Port of androidx.savedstate.SavedStateRegistry (+ internal SavedStateRegistryImpl,
 * merged into one class for C++). Providers save SavedState by key; owners drive
 * attach/restore/save through SavedStateRegistryController.
 *
 * consumeRestoredStateForKey returns a heap-owned copy (caller deletes) or nullptr.
 * SavedStateProvider::saveState likewise returns a heap-owned SavedState (caller
 * deletes).
 *********************************************************************************/
#include <string>
#include <unordered_map>
#include <savedstate/savedstate.h>
namespace cdroid{
namespace savedstate{

class SavedStateRegistryOwner;

class SavedStateRegistry{
public:
    class SavedStateProvider{
    public:
        virtual ~SavedStateProvider() = default;
        // Returns a heap-owned SavedState (caller owns).
        virtual SavedState* saveState() = 0;
    };

    bool isRestored() const { return mIsRestored; }

    // Returns and consumes the restored state for key (heap-owned copy or null).
    SavedState* consumeRestoredStateForKey(const std::string& key);
    void registerSavedStateProvider(const std::string& key, SavedStateProvider* provider);
    SavedStateProvider* getSavedStateProvider(const std::string& key) const;
    void unregisterSavedStateProvider(const std::string& key);

    // Owner-driven lifecycle (called by SavedStateRegistryController).
    void performAttach(SavedStateRegistryOwner* owner);
    void performRestore(SavedState* savedState);
    void performSave(SavedState& outBundle);

private:
    static const char* SAVED_COMPONENTS_KEY() {
        return "androidx.lifecycle.BundlableSavedStateRegistry.key";
    }
    std::unordered_map<std::string, SavedStateProvider*> mKeyToProviders;
    SavedState mRestoredState; // restored + unconsumed entries
    bool mIsRestored = false;
    bool mAttached = false;
};

}//namespace savedstate
}//namespace cdroid
#endif
