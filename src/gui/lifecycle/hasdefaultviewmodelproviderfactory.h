#ifndef __HASDEFAULTVIEWMODELPROVIDERFACTORY_H__
#define __HASDEFAULTVIEWMODELPROVIDERFACTORY_H__
/*********************************************************************************
 * Port of androidx.lifecycle.HasDefaultViewModelProviderFactory. The default
 * factory cannot reflectively instantiate ViewModels in C++, so owners are
 * expected to override getDefaultViewModelProviderFactory (Fragment does this
 * with SavedStateViewModelFactory).
 *********************************************************************************/
#include <lifecycle/viewmodelprovider.h>
#include <lifecycle/creationextras.h>
namespace cdroid{
namespace lifecycle{

class HasDefaultViewModelProviderFactory{
public:
    virtual ~HasDefaultViewModelProviderFactory() = default;
    virtual ViewModelProvider::Factory& getDefaultViewModelProviderFactory();
    virtual CreationExtras& getDefaultViewModelCreationExtras();
};

}//namespace lifecycle
}//namespace cdroid
#endif
