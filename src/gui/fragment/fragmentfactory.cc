#include <fragment/fragmentfactory.h>
#include <porting/cdlog.h>
#include <unordered_map>

namespace cdroid{
namespace fragment{

namespace{
// Lazily-initialized global registry (single UI thread in CDROID).
std::unordered_map<std::string, std::function<Fragment*()>>& registry(){
    static std::unordered_map<std::string, std::function<Fragment*()>> sRegistry;
    return sRegistry;
}
}//anonymous

void FragmentFactory::registerFragment(const std::string& className, std::function<Fragment*()> ctor){
    registry()[className] = std::move(ctor);
}

bool FragmentFactory::isFragmentClass(const std::string& className){
    return registry().count(className) > 0;
}

Fragment* FragmentFactory::instantiate(const std::string& className){
    auto it = registry().find(className);
    if(it == registry().end()){
        LOGE("FragmentFactory: no Fragment registered for class %s "
             "(use REGISTER_FRAGMENT or FragmentFactory::registerFragment)", className.c_str());
        return nullptr;
    }
    return it->second();
}

}//namespace fragment
}//namespace cdroid
