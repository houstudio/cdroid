#include <core/activityfactory.h>
#include <porting/cdlog.h>
#include <unordered_map>
namespace cdroid{

namespace {
// Single UI thread in CDROID (see FragmentFactory); a function-local static map is sufficient.
std::unordered_map<std::string, std::function<Window*()>>& registry(){
    static std::unordered_map<std::string, std::function<Window*()>> r;
    return r;
}
} // namespace

void ActivityFactory::registerActivity(const std::string& className, std::function<Window*()> ctor){
    registry()[className] = std::move(ctor);
}

bool ActivityFactory::isActivityClass(const std::string& className){
    return registry().find(className) != registry().end();
}

Window* ActivityFactory::instantiate(const std::string& className){
    auto it = registry().find(className);
    fprintf(stderr, "[ActivityFactory] instantiate('%s') → %s\n", className.c_str(),
            it != registry().end() ? "FOUND" : "NOT FOUND");
    if(it == registry().end()){
        LOGE("ActivityFactory: no Window registered for '%s' (REGISTER_ACTIVITY it?)", className.c_str());
        return nullptr;
    }
    // The factory's `new ClassName()` runs the Window ctor, which self-registers with WindowManager
    // (WindowManager::getInstance().addWindow) — so the returned Window is already on screen.
    return it->second();
}

}//namespace
