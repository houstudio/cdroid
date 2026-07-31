/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
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
