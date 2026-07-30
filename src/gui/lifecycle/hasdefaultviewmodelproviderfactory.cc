#include <lifecycle/hasdefaultviewmodelproviderfactory.h>
#include <porting/cdlog.h>

namespace cdroid{
namespace lifecycle{
namespace{
// No reflective instantiation in C++; this stub logs and returns null. Override
// HasDefaultViewModelProviderFactory::getDefaultViewModelProviderFactory to supply
// a real factory.
class DefaultFactory : public ViewModelProvider::Factory{
public:
    ViewModel* create(const std::string& modelClass, CreationExtras&) override{
        LOGE("DefaultViewModelProviderFactory cannot create %s (no reflection); "
             "override getDefaultViewModelProviderFactory", modelClass.c_str());
        return nullptr;
    }
};
}//anonymous

ViewModelProvider::Factory& HasDefaultViewModelProviderFactory::getDefaultViewModelProviderFactory(){
    static DefaultFactory sDefault;
    return sDefault;
}

CreationExtras& HasDefaultViewModelProviderFactory::getDefaultViewModelCreationExtras(){
    return const_cast<CreationExtras&>(EmptyCreationExtras());
}

}//namespace lifecycle
}//namespace cdroid
