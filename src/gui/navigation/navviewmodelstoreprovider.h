#ifndef __NAVVIEWMODELSTOREPROVIDER_H__
#define __NAVVIEWMODELSTOREPROVIDER_H__
/*********************************************************************************
 * Port of androidx.navigation.NavViewModelStoreProvider. Provides a ViewModelStore
 * per NavBackStackEntry id (used by NavController to scope VMs to back-stack entries).
 *********************************************************************************/
#include <string>
#include <lifecycle/viewmodelstore.h>
namespace cdroid{

class NavViewModelStoreProvider{
public:
    virtual ~NavViewModelStoreProvider() = default;
    virtual lifecycle::ViewModelStore* get(const std::string& key) = 0;
    virtual void clear(const std::string& key) = 0;
};

}//namespace cdroid
#endif
