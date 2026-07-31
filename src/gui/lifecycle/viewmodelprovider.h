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
#ifndef __VIEWMODELPROVIDER_H__
#define __VIEWMODELPROVIDER_H__
/*********************************************************************************
 * Port of androidx.lifecycle.ViewModelProvider. modelClass is a string (class
 * name) instead of KClass; reflective instantiation is replaced by an explicit
 * Factory the caller supplies.
 *********************************************************************************/
#include <string>
#include <lifecycle/viewmodel.h>
#include <lifecycle/viewmodelstore.h>
#include <lifecycle/viewmodelstoreowner.h>
#include <lifecycle/creationextras.h>
namespace cdroid{
namespace lifecycle{

class ViewModelProvider{
public:
    class Factory{
    public:
        virtual ~Factory() = default;
        virtual ViewModel* create(const std::string& modelClass, CreationExtras& extras) = 0;
    };

    ViewModelProvider(ViewModelStore* store, Factory* factory, CreationExtras* extras);

    /** Returns (creating+caching if needed) the ViewModel of modelClass. */
    template<typename T> T* get(const std::string& modelClass){
        return static_cast<T*>(getImpl(defaultKey(modelClass), modelClass));
    }
    template<typename T> T* get(const std::string& key, const std::string& modelClass){
        return static_cast<T*>(getImpl(key, modelClass));
    }

    static ViewModelProvider* create(ViewModelStoreOwner* owner, Factory* factory = nullptr, CreationExtras* extras = nullptr);
    static const CreationExtras::Key VIEW_MODEL_KEY;

private:
    ViewModel* getImpl(const std::string& key, const std::string& modelClass);
    static std::string defaultKey(const std::string& modelClass);
    ViewModelStore* mStore;
    Factory* mFactory;
    CreationExtras* mExtras;
};

}//namespace lifecycle
}//namespace cdroid
#endif
