#ifndef __VIEWMODELSTOREOWNER_H__
#define __VIEWMODELSTOREOWNER_H__
/*********************************************************************************
 * Port of androidx.lifecycle.ViewModelStoreOwner.
 *********************************************************************************/
#include <lifecycle/viewmodelstore.h>
namespace cdroid{
namespace lifecycle{

class ViewModelStoreOwner{
public:
    virtual ~ViewModelStoreOwner() = default;
    virtual ViewModelStore& getViewModelStore() = 0;
};

}//namespace lifecycle
}//namespace cdroid
#endif
