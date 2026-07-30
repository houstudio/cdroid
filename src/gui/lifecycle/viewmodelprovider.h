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
