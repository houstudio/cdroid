#ifndef __VIEWMODELSTORE_H__
#define __VIEWMODELSTORE_H__
/*********************************************************************************
 * Port of androidx.lifecycle.ViewModelStore. Owns the ViewModel instances put
 * into it and clears them on destruction/clear().
 *********************************************************************************/
#include <string>
#include <unordered_map>
#include <lifecycle/viewmodel.h>
namespace cdroid{
namespace lifecycle{

class ViewModelStore{
public:
    ViewModelStore() = default;
    ~ViewModelStore();

    // Stores viewModel under key (ownership transferred); clears any previous.
    void put(const std::string& key, ViewModel* viewModel);
    // Returns the ViewModel under key (borrowed), or null.
    ViewModel* get(const std::string& key);
    // Clears and deletes all stored ViewModels.
    void clear();

private:
    std::unordered_map<std::string, ViewModel*> mMap;
};

}//namespace lifecycle
}//namespace cdroid
#endif
