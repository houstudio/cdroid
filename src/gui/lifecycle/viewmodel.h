#ifndef __VIEWMODEL_H__
#define __VIEWMODEL_H__
/*********************************************************************************
 * Port of androidx.lifecycle.ViewModel. Coroutine viewModelScope is omitted;
 * the addCloseable/getCloseable/clear/onCleared contract is preserved.
 *********************************************************************************/
#include <string>
#include <vector>
#include <unordered_map>
#include <lifecycle/closeable.h>
namespace cdroid{
namespace lifecycle{

class ViewModel{
public:
    ViewModel() = default;
    virtual ~ViewModel();

    // Adds a closeable bound to key; replaces (and closes) any previous one.
    // If already cleared, closes closeable immediately.
    void addCloseable(const std::string& key, Closeable* closeable);
    // Adds a closeable without a key.
    virtual void addCloseable(Closeable* closeable);
    // Returns the closeable bound to key, or null.
    Closeable* getCloseable(const std::string& key);

protected:
    // Called when this ViewModel is no longer used and will be destroyed.
    virtual void onCleared() {}

public: // internal
    // Closes all resources in order (keyed, then keyless), then invokes onCleared.
    void clear();

private:
    std::unordered_map<std::string, Closeable*> mKeyedCloseables;
    std::vector<Closeable*> mCloseables;
    bool mCleared = false;
};

}//namespace lifecycle
}//namespace cdroid
#endif
