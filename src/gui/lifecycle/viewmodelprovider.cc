#include <lifecycle/viewmodelprovider.h>
#include <lifecycle/hasdefaultviewmodelproviderfactory.h>
#include <porting/cdlog.h>

namespace cdroid{
namespace lifecycle{

const CreationExtras::Key ViewModelProvider::VIEW_MODEL_KEY;

ViewModelProvider::ViewModelProvider(ViewModelStore* store, Factory* factory, CreationExtras* extras)
    : mStore(store), mFactory(factory), mExtras(extras){}

std::string ViewModelProvider::defaultKey(const std::string& modelClass){
    // androidx uses the canonical class name; C++ uses the supplied class name string.
    return modelClass;
}

ViewModel* ViewModelProvider::getImpl(const std::string& key, const std::string& modelClass){
    ViewModel* viewModel = mStore ? mStore->get(key) : nullptr;
    if(viewModel != nullptr) return viewModel;
    if(!mFactory){
        LOGE("ViewModelProvider has no Factory to create %s", modelClass.c_str());
        return nullptr;
    }
    MutableCreationExtras extras = mExtras ? MutableCreationExtras(*mExtras) : MutableCreationExtras();
    extras.put<std::string>(&VIEW_MODEL_KEY, std::make_shared<std::string>(key));
    viewModel = mFactory->create(modelClass, extras);
    if(viewModel && mStore) mStore->put(key, viewModel);
    return viewModel;
}

ViewModelProvider* ViewModelProvider::create(ViewModelStoreOwner* owner, Factory* factory, CreationExtras* extras){
    if(!factory || !extras){
        auto* h = dynamic_cast<HasDefaultViewModelProviderFactory*>(owner);
        if(!factory && h) factory = &h->getDefaultViewModelProviderFactory();
        if(!extras && h) extras = &h->getDefaultViewModelCreationExtras();
    }
    return new ViewModelProvider(&owner->getViewModelStore(), factory, extras);
}

}//namespace lifecycle
}//namespace cdroid
